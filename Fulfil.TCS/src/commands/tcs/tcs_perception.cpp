//
// Created by priyanka on 9/6/24.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <numeric>
#include <cmath>
#include <Fulfil.CPPUtils/logging.h>
#include "tcs_perception.h"
#include <tuple>
#include "tcs_error_codes.h"
#include <json.hpp>
#include "aruco/aruco_utils.h"
#include <chrono>

using fulfil::dispense::commands::get_error_name_from_code;
using fulfil::dispense::commands::TCSErrorCodes;
using fulfil::dispense::commands::TCSPerception;
using fulfil::utils::aruco::ArucoTransforms;
using fulfil::utils::Logger;

/**
 * Any array of ideal rectangles that select the region of interest in
 * LFR image.  Values are absolute since they are so particular to the hardcoded
 * baseline image.
 */
std::map<std::string, std::vector<cv::Point2f>> baselineLfrCavities = {
    {
        "LFB-3.2", {
            cv::Point(370, 450), // LFB 3.2 top-left: (370/2592 px, 450/1944 px)
            cv::Point(370, 1546), // LFB 3.2 top-right: (2210/2592 px, 1546/1944 px)
            cv::Point(2210, 450), // LFB 3.2 bottom-left: (370/2592 px, 450/1944 px)
            cv::Point(2210, 1546), // LFB 3.2 bottom-right: (2210/2592 px, 1546/1944 px)
        }
    }
};

static std::shared_ptr<fulfil::utils::aruco::ArucoTransforms> arucoInstance;

std::shared_ptr<fulfil::utils::aruco::ArucoTransforms> TCSPerception::getTCSArucoTransforms() {
    if (arucoInstance != nullptr) {
        return arucoInstance;
    }

    auto parameters = std::make_shared<cv::aruco::DetectorParameters>();
    // Width of border boundry is same as 1 bit column
    parameters->markerBorderBits = 1; 
    // Ignore boxes that are too tiny
    parameters->maxMarkerPerimeterRate = 4 * 0.0463; // = 4 edges * 120px / 2592px eyeballing in img editor
    // Ignore boxes that are too big
    parameters->minMarkerPerimeterRate = 4 * 0.0135; // = 4 edges * 35px / 2592px eyeballing in img editor
    // Use newest fastest algo
    // TODO: Restore for openCV 4.7+
    //parameters.useAruco3Detection = true;

    // TODO: Switch to this for OpenCV 4.7+
    //auto dict = cv::aruco:: Dictionary::extendDictionary(8, 4);
    auto dict = cv::aruco::generateCustomDictionary(8,4);
    arucoInstance = std::make_shared<fulfil::utils::aruco::ArucoTransforms>(dict, parameters, 0.75, 5, 8);
    arucoInstance->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFB-3.2.jpeg", "LFB-3.2", 8);

    return arucoInstance;
}

TCSPerception::TCSPerception(std::shared_ptr<std::string> lfb_generation)
{

}

void TCSPerception::SaveImages(cv::Mat image, std::string image_path)
{
    try
    {
        if (image.size().empty())
        {
            Logger::Instance()->Error("Cannot save empty image to {}", image_path);
        }
        cv::imwrite(image_path, image);
        Logger::Instance()->Info("{} saved successfully!!!", image_path);
    }
    catch (const std::exception &ex)
    {
        Logger::Instance()->Error("TCSPerception::SaveImages caught error: {}", ex.what());
    }
    catch (...)
    {
        Logger::Instance()->Error("TCSPerception::SaveImages hit error in catch(...)");
    }
}

void TCSPerception::write_result_file(std::string labelFilename, int markerCount) {
    try {
        std::ofstream file(labelFilename);
        if (file.is_open())
        {
            // Build JSON object and stringify it to file
            std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
            (*result_json)["isEmpty"] = this->is_bag_empty;
            (*result_json)["markers"] = markerCount;
            (*result_json)["lfbGeneration"] = *bot_generation;
            file << result_json->dump();
            file.close();
            Logger::Instance()->Info("Wrote result file: {}", labelFilename);
        }
        else {
            Logger::Instance()->Info("Could not open file: {}", labelFilename);
        }
    }
    catch (...)
    {
        Logger::Instance()->Error("Could not write file: " + labelFilename);
    }
}

bool TCSPerception::is_bag_clips_closed(std::shared_ptr<cv::Mat> bag_image, std::string directory_path)
{
    // Prep save results to local file
    std::string labelFilename = directory_path + "result.json";
    
    cv::Mat image = *bag_image;
    int clips_closed_count = 0;
    // TODO exit cleanly between sequence if any point in the sequence fails instead of continuing on

    // Apply an Aruco-anchored homography (rotate/scale/translate) to make this image's marker pixel positions
    // match as closely as possible to a clean baseline and transform the entire image accordingly to coerce
    // the LFR into a perfectly horizontally aligned and rotated rectangle.  Also scale a baseline region of
    // interest rectangle based on the discovered homography between Aruco corners.
    auto homog = TCSPerception::getTCSArucoTransforms()->findImgHomographyFromMarkers(image, directory_path);
    auto startTime = std::chrono::steady_clock::now();

    // default if any errors are encountered in the algorithm is true, to assume the bag is empty
    try
    { 
        // Set default cynical value in case of marker error
        this->is_bag_empty = false;
        if (homog->transformMatrix.empty()) {
            this->success_code = homog->maxMatchesSeen > 0 ? (TCSErrorCodes::NotEnoughMarkersDetected) : (TCSErrorCodes::NoMarkersDetected);
            this->error_description = get_error_name_from_code((TCSErrorCodes)this->success_code) + " -> " + std::string("Could not find Aruco marker baseline, found: " + std::to_string(homog->maxMatchesSeen));
            Logger::Instance()->Error(this->error_description);
            write_result_file(labelFilename, homog->maxMatchesSeen);
            return false;
        }

        // Apply and (estimated) best linear tranform to input image to reposition pixel points more like baseline's
        auto imgRepinnedToBaseline = TCSPerception::getTCSArucoTransforms()->applyHomographyToImg(image, homog);
        // The baseline rect should now match the repinned image very well or well enough
        auto baselineLFBCavityRect = baselineLfrCavities[homog->bestCandidateName];

        // TODO: Do the algo here based on imgRepinnedToBasline, which ensures all relative
        // positioning in the image matches a known baseline as much as possible by Aruco
        // tag location.  Starting from this ensures your algo is largely rotation / 
        // scale / translation resistant

        // ex.:
        //cv::Mat region_of_interest = calculate_roi(imgRepinnedToBaseline, baselineLFBCavityRect, shrink_factor_X, shrink_factor_Y, directory_path);
        

        auto stopTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
        Logger::Instance()->Info("is_bag_clips_closed took {}ms", duration);

        write_result_file(labelFilename, homog->maxMatchesSeen);

        Logger::Instance()->Info("Returning : {}", std::to_string(clips_closed_count));
        write_result_file(labelFilename, homog->maxMatchesSeen);
        return clips_closed_count;
    }
    catch (const std::exception &e)
    {
        Logger::Instance()->Trace("TCSPerception in catch exception");
        if (this->success_code == TCSErrorCodes::Success)
        {
            this->success_code = TCSErrorCodes::UnspecifiedError;
            this->error_description = get_error_name_from_code((TCSErrorCodes)this->success_code) + " -> " + std::string("Caught exception in is_bag_clips_closed: ") + e.what();
            Logger::Instance()->Error(this->error_description);
        }
        else
        {
            Logger::Instance()->Debug("TCSPerception in catch exception block already has success_code {} so no error fields will be updated", this->success_code);
        }
        write_result_file(labelFilename, homog->maxMatchesSeen);
        return this->is_bag_empty;
    }
    catch (...)
    {
        Logger::Instance()->Trace("TCSPerception in catch (...) block");
        if (this->success_code == TCSErrorCodes::Success)
        {
            this->success_code = TCSErrorCodes::UnspecifiedError;
            this->error_description = get_error_name_from_code((TCSErrorCodes)this->success_code) + " -> " + std::string("Caught error in is_bag_clips_closed in catch(...)!");
            Logger::Instance()->Error(this->error_description);
        }
        else
        {
            Logger::Instance()->Debug("TCSPerception in catch (...) block already has success_code {} so no error fields will be updated", this->success_code);
        }
    }
    write_result_file(labelFilename, homog->maxMatchesSeen);
    return this->is_bag_empty;
}
