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
#include <thread>
#include <future>
#include <fstream>
#include <stdexcept>
#include <numeric>
#include <cmath>
#include <Fulfil.CPPUtils/logging.h>
#include "commands/tcs/tcs_perception.h"
#include "commands/tcs/tcs_error_codes.h"
#include <tuple>
#include <json.hpp>
//#include "orbbec/orbbec_camera.h"
#include <Fulfil.CPPUtils/aruco/aruco_utils.h>
#include <chrono>
#include <assert.h>
#include <string_view>

using fulfil::dispense::commands::tcs::get_error_name_from_code;
using fulfil::dispense::commands::tcs::TCSErrorCodes;
using fulfil::dispense::commands::tcs::TCSPerception;
using fulfil::dispense::commands::tcs::SiftEncoding;
using fulfil::dispense::commands::tcs::BagClipBaseline;
using fulfil::dispense::commands::tcs::SiftMatch;
using fulfil::dispense::commands::tcs::SiftEncoding;
using fulfil::dispense::commands::tcs::BagClipInference;
using fulfil::dispense::commands::tcs::BagClipsInference;
using fulfil::dispense::commands::tcs::BagLoadInference;
using fulfil::dispense::commands::tcs::BagOrientationInference;
using fulfil::dispense::commands::tcs::PBLBaguetteOrientation;
using fulfil::dispense::commands::tcs::BagType;
using fulfil::utils::aruco::ArucoTransforms;
using fulfil::utils::aruco::HomographyResult;
using fulfil::utils::Logger;
using cv::aruco::generateCustomDictionary;

// An unfortunate wrapper needed for std::async to resolve stuff
cv::Ptr<cv::aruco::Dictionary> generateCustomDictionary2Args(int a, int b) {
    return generateCustomDictionary(a, b);
}

TCSPerception::TCSPerception() {

    Logger::Instance()->Info("Generating Aruco dicts in BG...");
    // Slightly important to do biggest compute first
    std::future<cv::Ptr<cv::aruco::Dictionary>> facility_id_future_dict = std::async(std::launch::async, generateCustomDictionary2Args, 999, 6); 
    std::future<cv::Ptr<cv::aruco::Dictionary>> tote_id_future_dict = std::async(std::launch::async, generateCustomDictionary2Args, 250, 5); 
    std::future<cv::Ptr<cv::aruco::Dictionary>> bag_cavity_future_dict = std::async(std::launch::async, generateCustomDictionary2Args, 8, 4); 
    std::future<cv::Ptr<cv::aruco::Dictionary>> bag_type_future_dict = std::async(std::launch::async, generateCustomDictionary2Args, 2, 3); 

    Logger::Instance()->Info("Generating SIFT keypoints from baseline images");
    this->sift = cv::SIFT::create();
    // Pre-compute all SIFT keypoint/descriptors for all baseline images
    this->bag_orientation_baselines = new std::map<std::string, std::shared_ptr<SiftEncoding>>();
    this->bag_clip_baselines = new std::map<std::string, std::shared_ptr<BagClipBaseline>>();
    this->bag_clip_bounding_boxes = new std::map<std::string, std::tuple<cv::Point2f*, cv::Point2f*>*>();

    // Bag orientation SIFT pre-computes
    auto bag_front_right_img = cv::imread("Fulfil.TCS/assets/baselines/Bag-front-right.jpeg", cv::IMREAD_COLOR);
    std::vector<cv::KeyPoint> front_right_kps;
    cv::Mat front_right_descriptors;
    sift->detectAndCompute(bag_front_right_img, cv::noArray(), front_right_kps, front_right_descriptors);
    std::shared_ptr<std::vector<std::shared_ptr<cv::KeyPoint>>> front_right_kp_ptrs = std::make_shared<std::vector<std::shared_ptr<cv::KeyPoint>>>();
    for (auto const& kp : front_right_kps) {
        front_right_kp_ptrs->push_back(std::make_shared<cv::KeyPoint>(kp));
    }
    (*bag_orientation_baselines)["Bag-front-right"] = std::make_shared<SiftEncoding>(front_right_kp_ptrs, std::make_shared<cv::Mat>(front_right_descriptors));

    auto bag_back_left_img = cv::imread("Fulfil.TCS/assets/baselines/Bag-back-left.jpeg", cv::IMREAD_COLOR);
    std::vector<cv::KeyPoint> back_left_kps;
    cv::Mat back_left_descriptors;
    sift->detectAndCompute(bag_back_left_img, cv::noArray(), back_left_kps, back_left_descriptors);
    auto back_left_kp_ptrs = std::make_shared<std::vector<std::shared_ptr<cv::KeyPoint>>>();
    for (auto const& kp : back_left_kps) {
        back_left_kp_ptrs->push_back(std::make_shared<cv::KeyPoint>(kp));
    }
    (*bag_orientation_baselines)["Bag-back-left"] = std::make_shared<SiftEncoding>(back_left_kp_ptrs, std::make_shared<cv::Mat>(back_left_descriptors));

    // Bag clip bounding boxes and SIFT pre-computes

    (*bag_clip_bounding_boxes)["LFP-B_open_white-liner_tl"] = new std::tuple(new cv::Point2f(432, 165), new cv::Point2f(477, 217));
    (*bag_clip_bounding_boxes)["LFP-B_open_white-liner_tr"] = new std::tuple(new cv::Point2f(850, 170), new cv::Point2f(896, 220));
    (*bag_clip_bounding_boxes)["LFP-B_open_white-liner_bl"] = new std::tuple(new cv::Point2f(432, 526), new cv::Point2f(475, 571));
    (*bag_clip_bounding_boxes)["LFP-B_open_white-liner_br"] = new std::tuple(new cv::Point2f(842, 526), new cv::Point2f(890, 577));
    register_clip_baselines("LFP-B", false, "white-liner");

    (*bag_clip_bounding_boxes)["LFP-B_closed_white-liner_tl"] = new std::tuple(new cv::Point2f(449, 187), new cv::Point2f(493, 249));
    (*bag_clip_bounding_boxes)["LFP-B_closed_white-liner_tr"] = new std::tuple(new cv::Point2f(841, 193), new cv::Point2f(877, 250));
    (*bag_clip_bounding_boxes)["LFP-B_closed_white-liner_bl"] = new std::tuple(new cv::Point2f(444, 486), new cv::Point2f(486, 541));
    (*bag_clip_bounding_boxes)["LFP-B_closed_white-liner_br"] = new std::tuple(new cv::Point2f(822, 492), new cv::Point2f(871, 551));
    register_clip_baselines("LFP-B", true, "white-liner");


    (*bag_clip_bounding_boxes)["LFP-A_closed_brown-bag_tl"] = new std::tuple(new cv::Point2f(433, 190), new cv::Point2f(476, 254));
    (*bag_clip_bounding_boxes)["LFP-A_closed_brown-bag_tr"] = new std::tuple(new cv::Point2f(808, 206), new cv::Point2f(852, 258));
    (*bag_clip_bounding_boxes)["LFP-A_closed_brown-bag_bl"] = new std::tuple(new cv::Point2f(429, 488), new cv::Point2f(473, 552));
    (*bag_clip_bounding_boxes)["LFP-A_closed_brown-bag_br"] = new std::tuple(new cv::Point2f(799, 494), new cv::Point2f(842, 565));
    register_clip_baselines("LFP-A", true, "brown-bag");

    (*bag_clip_bounding_boxes)["LFP-A_open_brown-bag_tl"] = new std::tuple(new cv::Point2f(415, 166), new cv::Point2f(464, 250));
    (*bag_clip_bounding_boxes)["LFP-A_open_brown-bag_tr"] = new std::tuple(new cv::Point2f(821, 171), new cv::Point2f(870, 250));
    (*bag_clip_bounding_boxes)["LFP-A_open_brown-bag_bl"] = new std::tuple(new cv::Point2f(421, 504), new cv::Point2f(461, 576));
    (*bag_clip_bounding_boxes)["LFP-A_open_brown-bag_br"] = new std::tuple(new cv::Point2f(816, 520), new cv::Point2f(864, 578));
    register_clip_baselines("LFP-A", false, "brown-bag");

    (*bag_clip_bounding_boxes)["LFP-A_open_white-liner_tl"] = new std::tuple(new cv::Point2f(416, 171), new cv::Point2f(469, 256));
    (*bag_clip_bounding_boxes)["LFP-A_open_white-liner_tr"] = new std::tuple(new cv::Point2f(817, 181), new cv::Point2f(847, 258));
    (*bag_clip_bounding_boxes)["LFP-A_open_white-liner_bl"] = new std::tuple(new cv::Point2f(409, 507), new cv::Point2f(460, 581));
    (*bag_clip_bounding_boxes)["LFP-A_open_white-liner_br"] = new std::tuple(new cv::Point2f(808, 526), new cv::Point2f(858, 588));
    register_clip_baselines("LFP-A", false, "white-liner");

    (*bag_clip_bounding_boxes)["LFP-A_closed_white-liner_tl"] = new std::tuple(new cv::Point2f(433, 173), new cv::Point2f(476, 254));
    (*bag_clip_bounding_boxes)["LFP-A_closed_white-liner_tr"] = new std::tuple(new cv::Point2f(812, 187), new cv::Point2f(849, 261));
    (*bag_clip_bounding_boxes)["LFP-A_closed_white-liner_bl"] = new std::tuple(new cv::Point2f(427, 499), new cv::Point2f(474, 568));
    (*bag_clip_bounding_boxes)["LFP-A_closed_white-liner_br"] = new std::tuple(new cv::Point2f(803, 502), new cv::Point2f(847, 580));
    register_clip_baselines("LFP-A", true, "white-liner");

    Logger::Instance()->Info("Waiting for Aruco dicts to wrap up...");
    bag_type_id_aruco_dict = bag_type_future_dict.get();
    tote_id_aruco_dict = tote_id_future_dict.get();
    facility_id_aruco_dict = facility_id_future_dict.get();
    bag_cavity_markers_aruco_dict = bag_cavity_future_dict.get();

    Logger::Instance()->Info("TCS Perception inference loaded");
}

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
    //parameters->minMarkerPerimeterRate = 4 * 0.0135; // = 4 edges * 35px / 2592px eyeballing in img editor
    // Use newest fastest algo

    topViewAruco = std::make_shared<fulfil::utils::aruco::ArucoTransforms>(bag_cavity_markers_aruco_dict, parameters, 0.75, 2, 6);


    // !!!!!!!!!!!!  If you add to this be sure to update TCSPerception constructor too  !!!!!!!!!!!!!
    topViewAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFP-B_closed_white-liner.jpeg", "LFP-B_closed_white-liner", 6);
    topViewAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFP-B_open_white-liner.jpeg", "LFP-B_open_white-liner", 6);
    topViewAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFP-A_closed_white-liner.jpeg", "LFP-A_closed_white-liner", 4);
    topViewAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFP-A_open_white-liner.jpeg", "LFP-A_open_white-liner", 4);
    topViewAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFP-A_closed_brown-bag.jpeg", "LFP-A_closed_brown-bag", 4);
    topViewAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFP-A_open_brown-bag.jpeg", "LFP-A_open_brown-bag", 3);
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
    //To-Do - Improve Dictionary Config based on accurate test data
    // Ignore boxes that are too tiny
    //parameters->maxMarkerPerimeterRate = 4 * 0.0500; // = 4 edges * 120px / 2592px eyeballing in img editor
    // Ignore boxes that are too big
   // parameters->minMarkerPerimeterRate = 4 * 0.0300; // = 4 edges * 35px / 2592px eyeballing in img editor
    auto dict = bag_type_id_aruco_dict;
    bagTypeAruco = std::make_shared<fulfil::utils::aruco::ArucoTransforms>(dict, parameters, 0.75, 1, 2);
    //Baseline 1 - Front facing bag with opening towards right
    bagTypeAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/Bag-front-right.jpeg", "Front", 1);
    //Baseline 2 - Back facing bag with opening towards left
    bagTypeAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/Bag-back-left.jpeg", "Back", 1);
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
    auto dict = tote_id_aruco_dict;
    toteIDAruco = std::make_shared<fulfil::utils::aruco::ArucoTransforms>(dict, parameters, 0.75, 5, 8);
    toteIDAruco->loadBaselineImageAsCandidate("Fulfil.TCS/assets/baselines/LFB-3.2.jpeg", "LFB-3.2", 8);

    return toteIDAruco;
}

std::shared_ptr<cv::Mat> TCSPerception::crop_clip_arm(std::shared_ptr<cv::Mat> image, std::string baseline_name) {
    auto bounding_box = (*bag_clip_bounding_boxes)[baseline_name];
    auto p1 = std::get<0>(*bounding_box);
    auto p2 = std::get<1>(*bounding_box);
    cv::Mat roi(*image, cv::Rect(std::min(p1->x, p2->x), std::min(p1->y, p2->y), std::abs(p2->x - p1->x), std::abs(p2->y - p1->y)));
    auto croppedImage = std::make_shared<cv::Mat>();
    roi.copyTo(*croppedImage);
    return croppedImage;
}

std::shared_ptr<SiftEncoding> TCSPerception::sift_encode_arm(std::shared_ptr<cv::Mat> image, std::string baseline_name) {
    auto cropped_img = crop_clip_arm(image, baseline_name);
    std::vector<cv::KeyPoint> kps;
    cv::Mat descriptors;
    sift->detectAndCompute(*cropped_img, cv::noArray(), kps, descriptors);
    auto kp_ptrs = std::make_shared<std::vector<std::shared_ptr<cv::KeyPoint>>>();
    // Copy ref keypoints to proper shared ptrs
    for (auto const& kp : kps) {
        kp_ptrs->push_back(std::make_shared<cv::KeyPoint>(kp));
    }
    return std::make_shared<SiftEncoding>(kp_ptrs, std::make_shared<cv::Mat>(descriptors));
}

// Register a baseline image's corner clips, the crop corners must first be defined in bag_clip_bounding_boxes
void TCSPerception::register_clip_baselines(std::string lfr_and_cavity, bool is_closed, std::string name) {
    std::string img_key = lfr_and_cavity + "_" + (is_closed ? "closed" : "open") + "_" + name;
    std::shared_ptr<cv::Mat> img = std::make_shared<cv::Mat>(cv::imread("Fulfil.TCS/assets/baselines/" + img_key + ".jpeg", cv::IMREAD_COLOR));
    for (int i = 0; i < 4; i++) {
        auto corner_name = clip_index_to_hash(i);
        auto clip_key = img_key + "_" + corner_name;
        (*bag_clip_baselines)[clip_key] =
            std::make_shared<BagClipBaseline>(img, lfr_and_cavity, is_closed, i, sift_encode_arm(img, clip_key));
    }
}

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

void TCSPerception::write_clip_closed_result_file(std::shared_ptr<BagClipsInference> result, std::string request_id, std::string labelFilename) {
    try {
        std::ofstream file(labelFilename);
        if (file.is_open())
        {
            // Build JSON object and stringify it to file
            std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
            (*result_json)["status"] = result->status;
            (*result_json)["requestId"] = request_id;
            (*result_json)["allClosed"] = result->all_clips_closed;
            (*result_json)["lfbCavityType"] = result->lfb_cavity_type;
            (*result_json)["topLeftIsClosed"] = result->top_left_inference->is_closed;
            (*result_json)["topLeftConfidence"] = result->top_left_inference->confidence;
            (*result_json)["topLeftStatus"] = result->top_left_inference->status;
            (*result_json)["topRightIsClosed"] = result->top_right_inference->is_closed;
            (*result_json)["topRightConfidence"] = result->top_right_inference->confidence;
            (*result_json)["topRightStatus"] = result->top_right_inference->status;
            (*result_json)["bottomLeftIsClosed"] = result->bottom_left_inference->is_closed;
            (*result_json)["bottomLeftConfidence"] = result->bottom_left_inference->confidence;
            (*result_json)["bottomLeftStatus"] = result->bottom_left_inference->status;
            (*result_json)["bottomRightIsClosed"] = result->bottom_right_inference->is_closed;
            (*result_json)["bottomRightConfidence"] = result->bottom_right_inference->confidence;
            (*result_json)["bottomRightStatus"] = result->bottom_right_inference->status;
            file << result_json->dump();
            file.close();
            Logger::Instance()->Info("Wrote result file: {}", labelFilename);
        }
        else {
            Logger::Instance()->Info("Could not write file: {}", labelFilename);
        }
    }
    catch (...)
    {
        Logger::Instance()->Error("Could not write file: " + labelFilename);
    }
}

void TCSPerception::drawMatchingKeypoints(std::shared_ptr<cv::Mat> baseline,  std::shared_ptr<std::vector<std::shared_ptr<cv::KeyPoint>>> keypoints1,
                         std::shared_ptr<cv::Mat> testImage, std::shared_ptr<std::vector<std::shared_ptr<cv::KeyPoint>>> keypoints2,
                             std::vector<cv::DMatch> matches){
    int rows = std::max(baseline->rows, testImage->rows);
    int columns = baseline->cols + testImage->cols;

    cv::Mat siftImage = cv::Mat::zeros(rows, columns, baseline->type());
    
    double maxSpatialDiff = 10.0;

    baseline->copyTo(siftImage(cv::Rect(0, 0, baseline->cols, baseline->rows)));
    testImage->copyTo(siftImage(cv::Rect(baseline->cols, 0, testImage->cols, testImage->rows)));

    for (int i = 0; i < matches.size(); ++i) {
        cv::Point2f point1 = (*keypoints1)[matches[i].queryIdx]->pt;
        cv::Point2f point2 = (*keypoints2)[matches[i].trainIdx]->pt;

        if (std::abs(point1.x - point2.x) <= maxSpatialDiff && std::abs(point1.y - point2.y) <= maxSpatialDiff) {

            cv::Point2f newPoint2 = point2;
            newPoint2.x += baseline->cols;

            cv::Scalar color(rand() % 256, rand() % 256, rand() % 256);

            cv::circle(siftImage, point1, 4, color, 1, cv::LINE_AA);
            cv::circle(siftImage, newPoint2, 4, color, 1, cv::LINE_AA);
            cv::line(siftImage, point1, newPoint2, color, 1, cv::LINE_AA);
        }
    }
    cv::imwrite("/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/data/test/sift_match.jpeg", siftImage);
}

double TCSPerception::computeDescriptorSimilarity(std::shared_ptr<cv::Mat> baselineDescriptors, std::shared_ptr<cv::Mat> descriptors, std::shared_ptr<std::vector<std::shared_ptr<cv::KeyPoint>>> keyPointsBaseline, std::shared_ptr<std::vector<std::shared_ptr<cv::KeyPoint>>> keyPoints, std::shared_ptr<cv::Mat> image, cv::Mat baseline) {
    double similarity = 0.0f;
    
    if (baselineDescriptors->empty() || descriptors->empty()) {
        Logger::Instance()->Info("No descriptors to compare with baseline!");
        return 0.0;
    }

    cv::BFMatcher matcher(cv::NORM_L2);
    std::vector<std::vector<cv::DMatch>> matches;
    matcher.knnMatch(*baselineDescriptors, *descriptors, matches, 2);

    double ratioThreshold = 0.68f;
    std::vector<cv::DMatch> baselineMatches;

    for (auto match : matches) {
        if (match.size() >= 2 && match[0].distance < ratioThreshold * match[1].distance) {
            baselineMatches.push_back(match[0]);
        }
    }
   
    if (!baseline.empty()) {
        drawMatchingKeypoints(std::make_shared<cv::Mat>(baseline), keyPointsBaseline, image, keyPoints, baselineMatches);
    }
    similarity = (float)baselineMatches.size() / (float)std::min(keyPointsBaseline->size(), keyPoints->size());
    return similarity;
}

//To-Do Priyanka - Fix/Improve the logic to detect structure accurately
//Ideas - Crop out only the rough bag area
bool TCSPerception::bagStructureSimilarity(std::shared_ptr<cv::Mat> image, PBLBaguetteOrientation orient, std::shared_ptr<cv::Mat> transformedImage) {
    bool bagproblem = false; //default false, doesn't have high accuracy yet
    double threshold = 0.8f; // TODO: tune this after collecting enough prod samples and maybe hypertune best case?
    auto baseline = (orient == PBLBaguetteOrientation::FRONT_RIGHT_OPENING || orient == PBLBaguetteOrientation::FRONT_LEFT_OPENING) ?
        (*bag_orientation_baselines)["Bag-front-right"] :
        (*bag_orientation_baselines)["Bag-back-left"];
    
    std::vector<cv::KeyPoint> keyPoints;
    cv::Mat descriptors;
   
    sift->detectAndCompute(*transformedImage, cv::noArray(), keyPoints, descriptors);
    auto shared_kps = std::make_shared<std::vector<std::shared_ptr<cv::KeyPoint>>>();
    for (auto const& kp : keyPoints) {
        shared_kps->push_back(std::make_shared<cv::KeyPoint>(kp));
    }
    cv::Mat empty_mat;
    double similarity_ratio = computeDescriptorSimilarity(baseline->descriptors, std::make_shared<cv::Mat>(descriptors), baseline->keypoints, shared_kps, transformedImage, empty_mat);

    if (similarity_ratio > threshold) {
        bagproblem = false;
        Logger::Instance()->Info("Bag structure matches baseline!!");
    }
   
    return bagproblem;
}

std::shared_ptr<cv::Point2f> TCSPerception::transformRotation(std::shared_ptr<cv::Point2f> point, cv::Mat homography_matrix) {
    cv::Mat pointMatrix = (cv::Mat_<double>(3, 1) << point->x, point->y, 1.0);
    cv::Mat distanceMatrix = homography_matrix * pointMatrix;

    double w = distanceMatrix.at<double>(2, 0);
    double x = distanceMatrix.at<double>(0, 0) / w;
    double y = distanceMatrix.at<double>(1, 0) / w;

    return std::make_shared<cv::Point2f>(static_cast<float>(x), static_cast<float>(y));
}

//Maybe include this later if orientation result isn't accurate
double TCSPerception::calculateSlope(std::shared_ptr<cv::Point2f> p1, std::shared_ptr<cv::Point2f> p2) {
    if (p2->x == p1->x) {
        return std::numeric_limits<double>::infinity();
        Logger::Instance()->Info("Both points have the same x coordinate value");
    }
    return (p2->y - p1->y) / (p2->x - p1->x);
}

PBLBaguetteOrientation TCSPerception::calculateBagOrientation(std::shared_ptr<HomographyResult> homography, cv::Rect quadrant1) {
    PBLBaguetteOrientation orient;
    int idx = 0;
    auto corners = homography->imgMarkers->matches->markers;
    cv::Mat homography_matrix = homography->transformMatrix;
    if (!corners || corners->empty()) {
        return PBLBaguetteOrientation::UNEXPECTED;
    }
    auto markerCorners = (*corners)[idx];
    if (!markerCorners || markerCorners->size() < 2) {
        return PBLBaguetteOrientation::UNEXPECTED;
    }
    std::shared_ptr<cv::Point2f> point1 = markerCorners->at(1);
    std::shared_ptr<cv::Point2f> point2 = markerCorners->at(0);

    std::shared_ptr<cv::Point2f> point1Transform = transformRotation(point1, homography_matrix);

    double length = std::abs(point1->x - point1Transform->x);
    double breadth = std::abs(point1->y - point1Transform->y);
    double pixelThreshold = 200.0f; // TODO: tune this after collecting enough prod samples and maybe hypertune best case?
    if (quadrant1.contains(*point1Transform)) {
        orient = (length < pixelThreshold && breadth < pixelThreshold) ? PBLBaguetteOrientation::FRONT_RIGHT_OPENING : PBLBaguetteOrientation::FRONT_LEFT_OPENING;       
    }
    else {
        orient = (length < pixelThreshold && breadth < pixelThreshold) ? PBLBaguetteOrientation::BACK_LEFT_OPENING : PBLBaguetteOrientation::BACK_RIGHT_OPENING;
    }

    return orient;
}

//To-Do: Add aditional checks for INSULATED bags when we have the final tag/id info
// for now default to ambient
BagType TCSPerception::computeBagType(std::shared_ptr<HomographyResult> homography) {
    BagType bagType;
    std::shared_ptr<std::vector<int>> markerIds = homography->imgMarkers->matches->ids;
    if (markerIds->size() == 0) return BagType::UNKNOWN;
    bagType = (markerIds->size()==1 && ((*markerIds)[0]==5 || (*markerIds)[0] == 0)) ? BagType::AMBIENT : BagType::INSULATED;
    return bagType;
}

//To-Do - Add logging/detailed function descriptions
std::shared_ptr<BagOrientationInference> TCSPerception::getBagOrientation(std::shared_ptr<cv::Mat> bag_image, std::string directoryPath) {
    // Prep save results to local file
    std::string labelFilename = directoryPath + "bag_orientation.json";

    auto image = std::make_shared<cv::Mat>(*bag_image);
    auto startTime = std::chrono::steady_clock::now();
     
    auto homog = (*TCSPerception::getTCSBagTypeAruco()->findImgHomographyFromMarkers(*image, directoryPath))[0];
    
    if (homog->transformMatrix.empty()) {
        auto errCode = homog->max_markers_seen > 0 ? (TCSErrorCodes::NotEnoughMarkersDetected) : (TCSErrorCodes::NoMarkersDetected);
        Logger::Instance()->Info("Aruco homograhy returned empty results");
        return std::make_shared<BagOrientationInference>(BagOrientationInference{ PBLBaguetteOrientation::UNEXPECTED, BagType::UNKNOWN, false});
    }
    //write_result_file(labelFilename, homog->max_markers_seen);
    // Aruco transform the image to a more baseline-like positioning
    if (homog->max_markers_seen > 1) {
        auto errCode = TCSErrorCodes::NotEnoughMarkersDetected;
        Logger::Instance()->Info("Detected more than 1 marker on the bag");
    }
   
    auto repinnedImg = TCSPerception::getTCSBagTypeAruco()->applyHomographyToImg(*image, homog);
    BagType bagType = computeBagType(homog);
    
    int width = image->cols;
    int height = image->rows;
    cv::Rect quadrant1(width / 2, 0, width / 2, height / 2);
    PBLBaguetteOrientation orientation = calculateBagOrientation(homog, quadrant1);
    Logger::Instance()->Info("Detected Bag Orientation: {}", static_cast<int>(orientation));
    Logger::Instance()->Info("Detected Bag type: {}", static_cast<int>(bagType));
    
    bool bagProblem = bagStructureSimilarity(image, orientation, std::make_shared<cv::Mat>(repinnedImg));
    return std::make_shared<BagOrientationInference>(BagOrientationInference{orientation, bagType, bagProblem});
}

std::shared_ptr<BagLoadInference> TCSPerception::getBagLoadedState(std::shared_ptr<cv::Mat> top_image, std::shared_ptr<cv::Mat> side1_image, std::shared_ptr<cv::Mat> side2_image, std::string directoryPath) {
    return std::make_shared<BagLoadInference>(BagLoadInference{1});
}

std::string TCSPerception::clip_index_to_hash(int clip_index) {
    std::string upper_lower = clip_index < 2 ? "t" : "b";
    std::string left_right = clip_index % 2 == 0 ? "l" : "r";
    return upper_lower + left_right;
}

static bool ends_with(std::string_view str, std::string_view suffix)
{
    return str.size() >= suffix.size() && str.compare(str.size()-suffix.size(), suffix.size(), suffix) == 0;
}

std::shared_ptr<BagClipInference> TCSPerception::inferBagClipState(std::map<std::string, std::shared_ptr<cv::Mat>> baseline_to_repinned_img, int clip_index, std::string lfr_version_and_cavity, std::string directoryPath) {
    double threshold = 0.095d; // TODO: tune this after collecting enough prod samples and maybe hypertune best case?
    // Now that we know cavity type from Aruco match, we can iter through all matching cavity baselines to determine
    // both the clip crop and resulting clip inference inference state

    // Iterate through all bag_clip_baselines for best baseline matches excluding mismatching cavity types
    // and collect the highest confidence clip matching states
    double best_similarity = -1.0;
    std::string best_key = "";
    for (auto clip_base = bag_clip_baselines->begin(); clip_base != bag_clip_baselines->end(); ++clip_base) {
        auto clip_name = clip_base->first;
        std::string baseline_name = clip_name.substr(0, clip_name.find_last_of("_"));
        // Skip if not the right cavity
        if (lfr_version_and_cavity != clip_base->second->lfb_type_cavity) {
            continue;
        }
        // Skip if not the correct clip
        if (clip_index != clip_base->second->clip_index) {
            continue;
        }
        // Skip if the parent Aruco transformed image never made it this far
        if (baseline_to_repinned_img.count(baseline_name) == 0) {
            continue;
        }

        auto baseline = (*bag_clip_baselines)[clip_name];
        assert(baseline != nullptr);
        auto match = std::make_shared<SiftMatch>(sift_encode_arm(baseline_to_repinned_img[baseline_name], clip_name), baseline->encoded);

        auto baseline_img = cv::imread("Fulfil.TCS/assets/baselines/" + baseline_name + ".jpeg", cv::IMREAD_COLOR);
        double similarity = computeDescriptorSimilarity(
            match->baseline->descriptors, 
            match->candidate->descriptors, 
            match->baseline->keypoints, 
            match->candidate->keypoints, baseline_to_repinned_img[baseline_name], baseline_img);

        // take the max of the baseline with highest similarity and must also be above some confidence threshold
        if (similarity > best_similarity) {
            best_similarity = similarity;
            best_key = clip_name;
        }
    }
    assert(best_similarity != -1.0);
    if (best_similarity < threshold) {
        Logger::Instance()->Info("Too low of confidence for clip! best similarity: {}", best_similarity);
        return std::make_shared<BagClipInference>(false, best_similarity, TCSErrorCodes::LowConfidenceError, best_key);
    }
    bool is_closed = best_key.find("_closed_") != std::string::npos;
    Logger::Instance()->Info("Picked {} similarity: {} for {} (closed: {})", best_key, best_similarity, clip_index, is_closed);
    return std::make_shared<BagClipInference>(is_closed, best_similarity, TCSErrorCodes::Success, best_key);
}

std::shared_ptr<BagClipsInference> TCSPerception::getBagClipStates(cv::Mat bag_image, std::string lfr_version, std::string request_id, std::string directoryPath) {
    // Prep save results to local file
    std::string labelFilename = directoryPath + "bag_clips.json";
    std::cerr << "label file: " + labelFilename;
    
    int clips_closed_count = 0;
    auto startTime = std::chrono::steady_clock::now();

    // Apply an Aruco-anchored homography (rotate/scale/translate) to make this image's marker pixel positions
    // match as closely as possible to a clean baseline and transform the entire image accordingly to coerce
    // the LFR into a perfectly horizontally aligned and rotated rectangle.  Also scale a baseline region of
    // interest rectangle based on the discovered homography between Aruco corners.
    auto homogs = TCSPerception::getTCSLFRTopViewAruco()->findImgHomographyFromMarkers(bag_image, directoryPath);

    if (homogs->size() == 0) {
        auto errCode = TCSErrorCodes::NotEnoughMarkersDetected;
        //write_result_file(labelFilename, homog->max_markers_seen);
        auto topLeft = std::make_shared<BagClipInference>(false, 0.0, errCode, "");
        auto topRight = std::make_shared<BagClipInference>(false, 0.0, errCode, "");
        auto bottomLeft = std::make_shared<BagClipInference>(false, 0.0, errCode, "");
        auto bottomRight = std::make_shared<BagClipInference>(false, 0.0, errCode, "");
        return std::make_shared<BagClipsInference>("", false, false, errCode, topLeft, topRight, bottomLeft, bottomRight);
    }

    // Aruco transform the image to a more baseline-like positioning
    std::string best_homog_name = (*homogs)[0]->bestCandidateName;
    int first_split = best_homog_name.find("_");
    std::string lfr_version_and_cavity = best_homog_name.substr(0, first_split);
    std::cerr << "\nAruco deduced img to " + lfr_version_and_cavity;

    // Extract the winning cavity, it's the only decision point we fully trust from Aruco homog to cut baseline candidates in half
    // and may the best clip match win per clip among the halved baseline candidates remaining.

    // Pre-compute stretching the input image to fit all valid Aruco baselines for use in per-clip inference
    std::map<std::string, std::shared_ptr<cv::Mat>> baseline_to_repinned_img;
    for (auto const& homog : *homogs) {
        // Only consider Aruco baselines matching the best's LFR and cavity type
        if (homog->bestCandidateName.find(lfr_version_and_cavity) == 0) {
            auto repinned_img = std::make_shared<cv::Mat>(TCSPerception::getTCSLFRTopViewAruco()->applyHomographyToImg(bag_image, homog));
            baseline_to_repinned_img[homog->bestCandidateName] = repinned_img;
        }
    }

    // Run 4 mini-inferences against all 4 cropped bag clips
    auto topLeft = inferBagClipState(baseline_to_repinned_img, 0, lfr_version_and_cavity, directoryPath);
    auto topRight = inferBagClipState(baseline_to_repinned_img, 1, lfr_version_and_cavity, directoryPath);
    auto bottomLeft = inferBagClipState(baseline_to_repinned_img, 2, lfr_version_and_cavity, directoryPath);
    auto bottomRight = inferBagClipState(baseline_to_repinned_img, 3, lfr_version_and_cavity, directoryPath);

    bool allSuccess = topLeft->status == TCSErrorCodes::Success && topRight->status == TCSErrorCodes::Success && bottomLeft->status == TCSErrorCodes::Success && bottomRight->status == TCSErrorCodes::Success;
    bool allOpen = allSuccess && !topLeft->is_closed && !topRight->is_closed && !bottomLeft->is_closed && !bottomRight->is_closed;
    bool allClosed = allSuccess && topLeft->is_closed && topRight->is_closed && bottomLeft->is_closed && bottomRight->is_closed;
    TCSErrorCodes status = allSuccess ? TCSErrorCodes::Success : (TCSErrorCodes)(topLeft->status | topRight->status | bottomLeft->status | bottomRight->status);

    auto stopTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - startTime).count();
    Logger::Instance()->Info("getBagClipStates took {}ms", duration);
    auto res = std::make_shared<BagClipsInference>(lfr_version_and_cavity, allClosed, allOpen, status, topLeft, topRight, bottomLeft, bottomRight);
    write_clip_closed_result_file(res, request_id, labelFilename);
    return res;
}
