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
#include "commands/tcs/tcs_perception.h"
#include <tuple>
#include "commands/tcs/tcs_error_codes.h"
#include <json.hpp>
#include "aruco/aruco_utils.h"
#include <chrono>

using fulfil::dispense::commands::tcs::get_error_name_from_code;
using fulfil::dispense::commands::tcs::TCSErrorCodes;
using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::dispense::commands::tcs::BagClipInference;
using fulfil::dispense::commands::tcs::BagClipsInference;
using fulfil::dispense::commands::tcs::BagLoadInference;
using fulfil::dispense::commands::tcs::BagOrientationInference;
using fulfil::utils::aruco::ArucoTransforms;
using fulfil::utils::Logger;

/**
 * Any array of ideal rectangles that bound all 4 LFR bag clip locations.
 * Points are based on assets/baselines image(s).
 */
std::map<std::string, std::vector<std::vector<cv::Point2f>>> bagClipBBoxes = {
    {
        "LFB-3.2", {}
            // TODO! These are not true crop boxes yet!!!
            // Upper-left bag clip bbox
            /*[
                cv::Point(370, 450), //top-left
                cv::Point(370, 1546), //top-right
                cv::Point(2210, 450), //bottom-left
                cv::Point(2210, 1546), //bottom-right
            ],
            // Upper-right bag clip bbox
            [
                cv::Point(370, 450), //top-left
                cv::Point(370, 1546), //top-right
                cv::Point(2210, 450), //bottom-left
                cv::Point(2210, 1546), //bottom-right
            ],
            // Lower-left bag clip bbox
            [
                cv::Point(370, 450), //top-left
                cv::Point(370, 1546), //top-right
                cv::Point(2210, 450), //bottom-left
                cv::Point(2210, 1546), //bottom-right
            ].
            // Lower-right bag clip bbox
            [
                cv::Point(370, 450), //top-left
                cv::Point(370, 1546), //top-right
                cv::Point(2210, 450), //bottom-left
                cv::Point(2210, 1546), //bottom-right
            ]
        },*/
        // TODO: Side dispense LFR model should be defined here or LFB-3.2 renamed
    }
};

/**
 * Any array of ideal rectangles that select only the inner cavity of an
 * LFR image.  Values are absolute since they are so particular to the hardcoded
 * baseline image.  Points are based on assets/baselines image(s).
 */
std::map<std::string, std::vector<cv::Point2f>> baselineLfrCavities = {
    {
        "LFB-3.2", {
            cv::Point(370, 450), // LFB 3.2 top-left: (370/2592 px, 450/1944 px)
            cv::Point(370, 1546), // LFB 3.2 top-right: (2210/2592 px, 1546/1944 px)
            cv::Point(2210, 450), // LFB 3.2 bottom-left: (370/2592 px, 450/1944 px)
            cv::Point(2210, 1546), // LFB 3.2 bottom-right: (2210/2592 px, 1546/1944 px)
        },
        // TODO: Side dispense LFR model should be defined here or LFB-3.2 renamed
    }
};

static std::shared_ptr<fulfil::utils::aruco::ArucoTransforms> topViewAruco;

std::shared_ptr<fulfil::utils::aruco::ArucoTransforms> TCSPerception::getTCSLFRTopViewAruco() {
    if (topViewAruco != nullptr) {
        return topViewAruco;
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
    topViewAruco = std::make_shared<fulfil::utils::aruco::ArucoTransforms>(dict, parameters, 0.75, 5, 8);
    topViewAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFB-3.2.jpeg", "LFB-3.2", 8);
    // TODO: Side dispense LFR model should be loaded here

    return topViewAruco;
}

static std::shared_ptr<fulfil::utils::aruco::ArucoTransforms> bagTypeAruco;

/**
 * Get an Aruco class instance tuned for finding the boolean bag type
 */
std::shared_ptr<fulfil::utils::aruco::ArucoTransforms> TCSPerception::getTCSBagTypeAruco() {
    if (bagTypeAruco != nullptr) {
        return bagTypeAruco;
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
    auto dict = cv::aruco::generateCustomDictionary(2,3);
    bagTypeAruco = std::make_shared<fulfil::utils::aruco::ArucoTransforms>(dict, parameters, 0.75, 5, 8);
    bagTypeAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFB-3.2.jpeg", "LFB-3.2", 8);

    return bagTypeAruco;
}

static std::shared_ptr<fulfil::utils::aruco::ArucoTransforms> toteIDAruco;

/**
 * Get an Aruco class instance tuned for finding the tote ID
 */
std::shared_ptr<fulfil::utils::aruco::ArucoTransforms> TCSPerception::getTCSToteIDAruco() {
    if (toteIDAruco != nullptr) {
        return toteIDAruco;
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
    auto dict = cv::aruco::generateCustomDictionary(1000, 4);
    toteIDAruco = std::make_shared<fulfil::utils::aruco::ArucoTransforms>(dict, parameters, 0.75, 5, 8);
    toteIDAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFB-3.2.jpeg", "LFB-3.2", 8);

    return toteIDAruco;
}

TCSPerception::TCSPerception() {}

void SaveImages(cv::Mat image, std::string image_path)
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

/*void write_result_file(std::string labelFilename, int markerCount) {
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
}*/

// TODO: Probably not a bool, an object or enum maybe?
std::shared_ptr<BagOrientationInference> TCSPerception::getBagOrientation(std::shared_ptr<cv::Mat> bag_image, std::string directoryPath) {
    return std::make_shared<BagOrientationInference>(BagOrientationInference{1});
}

// TODO: probly not a bool
std::shared_ptr<BagLoadInference> TCSPerception::getBagLoadedState(std::shared_ptr<cv::Mat> top_image, std::shared_ptr<cv::Mat> side1_image, std::shared_ptr<cv::Mat> side2_image, std::string directoryPath) {
    return std::make_shared<BagLoadInference>(BagLoadInference{1});
}

std::shared_ptr<BagClipInference> inferBagClipState(cv::Mat image, int clipIndex, std::string lfrVersion, std::string directoryPath) {
    // TODO: Implement the actual algo here!  SIFT would probably work fine...
    return std::make_shared<BagClipInference>(BagClipInference{true, 1, TCSErrorCodes::Success});
}

std::shared_ptr<BagClipsInference> TCSPerception::getBagClipStates(std::shared_ptr<cv::Mat> bag_image, std::string lfrVersion, std::string directoryPath)
{
    // Prep save results to local file
    std::string labelFilename = directoryPath + "result.json";
    
    cv::Mat image = *bag_image;
    int clips_closed_count = 0;
    auto startTime = std::chrono::steady_clock::now();

    // Apply an Aruco-anchored homography (rotate/scale/translate) to make this image's marker pixel positions
    // match as closely as possible to a clean baseline and transform the entire image accordingly to coerce
    // the LFR into a perfectly horizontally aligned and rotated rectangle.  Also scale a baseline region of
    // interest rectangle based on the discovered homography between Aruco corners.
    auto homog = TCSPerception::getTCSLFRTopViewAruco()->findImgHomographyFromMarkers(image, directoryPath);

    if (homog->transformMatrix.empty()) {
        auto errCode = homog->maxMatchesSeen > 0 ? (TCSErrorCodes::NotEnoughMarkersDetected) : (TCSErrorCodes::NoMarkersDetected);
        //write_result_file(labelFilename, homog->maxMatchesSeen);
        auto topLeft = std::make_shared<BagClipInference>(BagClipInference{false, 0.0, errCode});
        auto topRight = std::make_shared<BagClipInference>(BagClipInference{false, 0.0, errCode});
        auto bottomLeft = std::make_shared<BagClipInference>(BagClipInference{false, 0.0, errCode});
        auto bottomRight = std::make_shared<BagClipInference>(BagClipInference{false, 0.0, errCode});
        return std::make_shared<BagClipsInference>(BagClipsInference{false, false, topLeft, topRight, bottomLeft, bottomRight});
    }

    // Aruco transform the image to a more baseline-like positioning
    auto repinnedImg = TCSPerception::getTCSLFRTopViewAruco()->applyHomographyToImg(image, homog);

    // Run 4 mini-inferences against all 4 cropped bag clips
    auto topLeft = inferBagClipState(repinnedImg, 0, lfrVersion, directoryPath);
    auto topRight = inferBagClipState(repinnedImg, 1, lfrVersion, directoryPath);
    auto bottomLeft = inferBagClipState(repinnedImg, 2, lfrVersion, directoryPath);
    auto bottomRight = inferBagClipState(repinnedImg, 3, lfrVersion, directoryPath);

    bool allSuccess = topLeft->status == TCSErrorCodes::Success && topRight->status == TCSErrorCodes::Success && bottomLeft->status == TCSErrorCodes::Success && bottomRight->status == TCSErrorCodes::Success;
    bool allOpen = allSuccess && !topLeft->isClosed && !topRight->isClosed && !bottomLeft->isClosed && !bottomRight->isClosed;
    bool allClosed = allSuccess && topLeft->isClosed && topRight->isClosed && bottomLeft->isClosed && bottomRight->isClosed;

    auto stopTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
    Logger::Instance()->Info("getBagClipStates took {}ms", duration);
    // TODO: write result file
    return std::make_shared<BagClipsInference>(BagClipsInference{allClosed, allOpen, topLeft, topRight, bottomLeft, bottomRight});
}
