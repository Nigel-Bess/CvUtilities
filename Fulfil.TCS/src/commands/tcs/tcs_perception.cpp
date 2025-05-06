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
using fulfil::dispense::commands::tcs::PBLBaguetteOrientation;
using fulfil::dispense::commands::tcs::BagType;
using fulfil::utils::aruco::ArucoTransforms;
using fulfil::utils::aruco::HomographyResult;
//using fulfil::utils::aruco::SiftBaseline;
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
    //To-Do - Improve Dictionary Config based on accurate test data
    // Ignore boxes that are too tiny
    //parameters->maxMarkerPerimeterRate = 4 * 0.0500; // = 4 edges * 120px / 2592px eyeballing in img editor
    // Ignore boxes that are too big
   // parameters->minMarkerPerimeterRate = 4 * 0.0300; // = 4 edges * 35px / 2592px eyeballing in img editor
    auto dict = cv::aruco::generateCustomDictionary(8, 4);
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

//To-Do - Check if Eric needs any of this logic, if not - remove all the sift madness

//SiftBaseline calculateSiftBaseline(cv::Mat baseline_image, std::shared_ptr<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>> markerCorners, std::shared_ptr<std::vector<int>> markerIds) {
//    SiftBaseline siftBaseline;
//
//    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
//    std::vector<std::vector<cv::Point2f>> corners;
//    sift->detectAndCompute(baseline_image, cv::noArray(), siftBaseline.keypoints, siftBaseline.descriptors);
//    for (auto& ptr : *markerCorners) {
//        if (ptr) {
//            corners.push_back(*ptr);
//        }
//    }
//    siftBaseline.markerCorners = corners;
//    siftBaseline.markerIds = *markerIds;
//
//    return siftBaseline;
//}
//
//
//
//double computeMarkerSimilarity(std::vector<int> baselineIds, std::vector<int> ids) {
//    if (baselineIds.empty()) {
//        Logger::Instance()->Info("No marker ids to compare with baseline!");
//        return 0.0;
//    }
//    int count = 0;
//    for (int id : baselineIds) {
//        if (std::find(ids.begin(), ids.end(), id) != ids.end()) {
//            count++;
//        }
//    }
//    return static_cast<double>(count) / baselineIds.size();
//}
//
//
//bool TCSPerception::compareOrientationWithBaseline(cv::Mat image,
//    std::shared_ptr<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>> markerCorners, std::shared_ptr<std::vector<int>> markerIds) {
//    SiftBaseline baseline = calculateSiftBaseline(image, markerCorners, markerIds);
//    double keypointSimilarityThreshold = 0.8;
//    double markerSimilarityThreshold = 0.9;
//    bool baslineMatch = false;
//    std::vector<cv::KeyPoint> keypoints;
//    cv::Mat descriptors;
//    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
//    sift->detectAndCompute(image, cv::noArray(), keypoints, descriptors);
//    
//    double keypointSimilarity = computeKeypointSimilarity(baseline.descriptors, descriptors);
//    bool keypointMatch = keypointSimilarity >= keypointSimilarityThreshold;
// 
//    double markerSimilarity = computeMarkerSimilarity(baseline.markerIds, *markerIds);
//    bool markersMatch = markerSimilarity >= markerSimilarityThreshold;
//
//    if (keypointMatch && markersMatch) {
//        baslineMatch = true;
//    }
//    return baslineMatch;
//}
//
////baseline calculation, move the baseline results to class variables, use later fpr comparison
//cv::Mat TCSPerception::siftDetection(std::shared_ptr<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>> sharedCorners, cv::Mat img) {
//    std::vector<cv::KeyPoint> keyPoints;
//    cv::Mat descriptors;
//    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
//    sift->detectAndCompute(img, cv::noArray(), keyPoints, descriptors);
//    cv::Mat sift_image;
//    std::vector<cv::KeyPoint> filteredKeyPoints;
//    std::vector<float> keypointAngles;
//
//    for (const auto& kp : keyPoints) {
//        for (const auto& markerPtr : *sharedCorners) {
//            if (markerPtr == nullptr) continue;
//            const std::vector<cv::Point2f>& marker = *markerPtr;
//            if (cv::pointPolygonTest(marker, kp.pt, false) >= 0) {
//                filteredKeyPoints.push_back(kp);
//                keypointAngles.push_back(kp.angle);
//                break;
//            }
//        }
//    }
//    Logger::Instance()->Info("Display Filtered Key Angles!");
//
//    for (int i = 0; i < keypointAngles.size(); i++) {
//        Logger::Instance()->Info("Keypoint {} : {} degrees", i, keypointAngles[i]);
//    }
//
//    cv::drawKeypoints(img, filteredKeyPoints, sift_image, cv::Scalar(0, 255, 0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
//    
//    cv::imwrite("/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/data/test/sift_output.jpeg", sift_image);
//    cv::Mat test_image = cv::imread("/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/test/assets/Bag-front-right.jpeg", cv::IMREAD_COLOR);
//    compareSift(test_image, img, descriptors, filteredKeyPoints, sharedCorners);
//    return descriptors;
//}
//
//
//void TCSPerception::compareSift(cv::Mat baseline_image, cv::Mat image, cv::Mat descriptors_baseline, std::vector<cv::KeyPoint> keypoints_baseline, std::shared_ptr<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>> sharedCorners) {
//    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
//
//    std::vector<cv::KeyPoint> keyPoints;
//    cv::Mat descriptors;
//    //cv::Mat test_image = cv::imread("/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/data/test/test_image.jpeg");
//    
//    sift->detectAndCompute(image, cv::noArray(), keyPoints, descriptors);
//    //cv::Mat sift_image;
//    std::vector<cv::KeyPoint> filteredKeyPoints;
//    std::vector<float> keypointAngles;
//
//    if (descriptors.empty()) {
//        Logger::Instance()->Info("No descriptors detected in the image!");
//        //return 0;
//    }
//
//    for (const auto& kp : keyPoints) {
//        for (const auto& markerPtr : *sharedCorners) {
//            if (markerPtr == nullptr) continue;
//            const std::vector<cv::Point2f>& marker = *markerPtr;
//            if (cv::pointPolygonTest(marker, kp.pt, false) >= 0) {
//                filteredKeyPoints.push_back(kp);
//                keypointAngles.push_back(kp.angle);
//                break;
//            }
//        }
//    }
//    //Logger::Instance()->Info("Display Filtered Key Angles!");
//
//    /*for (int i = 0; i < keypointAngles.size(); i++) {
//        Logger::Instance()->Info("Keypoint {} : {} degrees", i, keypointAngles[i]);
//    }*/
//    
//   // cv::drawKeypoints(image, filteredKeyPoints, sift_image, cv::Scalar(0, 255, 0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
//    
//    cv::BFMatcher matcher(cv::NORM_L2);
//    std::vector<std::vector<cv::DMatch>> matches;
//    matcher.knnMatch(descriptors_baseline, descriptors, matches, 2);
//    
//    float ratioThreshold = 0.75f;
//    //int goodMatches = 0;
//    std::vector<cv::DMatch> goodMatches;
//    for (int i = 0; i < matches.size(); ++i) {
//        if (matches[i].size() >= 2 && matches[i][0].distance < ratioThreshold * matches[i][1].distance) {
//            goodMatches.push_back(matches[i][0]);
//        }
//    }
//    
//    cv::Mat sift_image_again;
//    cv::drawKeypoints(image, filteredKeyPoints, sift_image_again, cv::Scalar(0, 255, 0), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
//
//    cv::imwrite("/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/data/test/sift_image_again.jpeg", sift_image_again);
//    //Logger::Instance()->Info("Good Matches {}", goodMatches);
//    cv::Mat imageMatches;
//    ///Can't figure out what went wrong1
//    //cv::drawMatches(image, keypoints_baseline, test_image, filteredKeyPoints, goodMatches, imageMatches, cv::Scalar::all(-1), cv::Scalar::all(-1), std::vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
//    drawMatchingKeypoints(baseline_image, keypoints_baseline, image, filteredKeyPoints, goodMatches, imageMatches);
//   
//    //cv::imwrite("/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/data/test/sift_match_image.jpeg", imageMatches);
//    //return keypoints_baseline.empty() ? 0.0 : static_cast<double>(goodMatches);
//
//}


//void write_result_file(std::string labelFilename, auto homog) {
//    try {
//        std::ofstream file(labelFilename);
//        if (file.is_open())
//        {
//            // Build JSON object and stringify it to file
//            std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
//            file << result_json->dump();
//            file.close();
//            Logger::Instance()->Info("Wrote result file: {}", labelFilename);
//        }
//        else {
//            Logger::Instance()->Info("Could not open file: {}", labelFilename);
//        }
//    }
//    catch (...)
//    {
//        Logger::Instance()->Error("Could not write file: " + labelFilename);
//    }
//}

void TCSPerception::drawMatchingKeypoints(cv::Mat baseline,  std::vector<cv::KeyPoint> keypoints1,
                         cv::Mat testImage, std::vector<cv::KeyPoint> keypoints2,
                             std::vector<cv::DMatch> matches, cv::Mat siftImage){
    int rows = std::max(baseline.rows, testImage.rows);
    int columns = baseline.cols + testImage.cols;

    siftImage = cv::Mat::zeros(rows, columns, baseline.type());
    
    double maxSpatialDiff = 10.0;

    baseline.copyTo(siftImage(cv::Rect(0, 0, baseline.cols, baseline.rows)));
    testImage.copyTo(siftImage(cv::Rect(baseline.cols, 0, testImage.cols, testImage.rows)));

    for (int i = 0; i < matches.size(); ++i) {
        cv::Point2f point1 = keypoints1[matches[i].queryIdx].pt;
        cv::Point2f point2 = keypoints2[matches[i].trainIdx].pt;

        if (std::abs(point1.x - point2.x) <= maxSpatialDiff && std::abs(point1.y - point2.y) <= maxSpatialDiff) {

            cv::Point2f newPoint2 = point2;
            newPoint2.x += baseline.cols;

            cv::Scalar color(rand() % 256, rand() % 256, rand() % 256);

            cv::circle(siftImage, point1, 4, color, 1, cv::LINE_AA);
            cv::circle(siftImage, newPoint2, 4, color, 1, cv::LINE_AA);

            cv::line(siftImage, point1, newPoint2, color, 1, cv::LINE_AA);
        }
    }
    cv::imwrite("/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/data/test/sift_match.jpeg", siftImage);
}

float TCSPerception::computeDescriptorSimilarity(cv::Mat baselineDescriptors, cv::Mat descriptors, std::vector<cv::KeyPoint> keyPointsBaseline, std::vector<cv::KeyPoint> keyPoints, cv::Mat baseline, cv::Mat image) {
  
    float similarity = 0.0f;
    
    if (baselineDescriptors.empty() || descriptors.empty()) {
        Logger::Instance()->Info("No descriptors to compare with baseline!");
        return 0.0;
    }

    cv::BFMatcher matcher(cv::NORM_L2);
    std::vector<std::vector<cv::DMatch>> matches;
    matcher.knnMatch(baselineDescriptors, descriptors, matches, 2);

    float ratioThreshold = 0.75f;
    std::vector<cv::DMatch> baselineMatches;

    for (auto match : matches) {
        if (match.size() >= 2 && match[0].distance < ratioThreshold * match[1].distance) {
            baselineMatches.push_back(match[0]);
        }
    }
   
    cv::Mat outImg;
    drawMatchingKeypoints(baseline, keyPointsBaseline, image, keyPoints, baselineMatches, outImg);
    similarity = (float)baselineMatches.size() / (float)std::min(keyPointsBaseline.size(), keyPoints.size());
    Logger::Instance()->Info("Similarity with the baseline image : {}", similarity);
    
    return similarity;
}

//To-Do Priyanka - Fix/Improve the logic to detect structure accurately
//Ideas - Crop out only the rough bag area
bool TCSPerception::bagStructureSimilarity(cv::Mat image, PBLBaguetteOrientation orient, cv::Mat transformedImage) {

    bool bagproblem = false; //default false, doesn't have high accuracy yet
    float threshold = 0.8f;
    cv::Mat baseline;
    cv::Mat baseline1 = cv::imread("Fulfil.TCS/assets/baselines/Bag-front-right.jpeg", cv::IMREAD_COLOR);
    cv::Mat baseline2 = cv::imread("Fulfil.TCS/assets/baselines/Bag-back-left.jpeg", cv::IMREAD_COLOR);
    baseline = (orient == PBLBaguetteOrientation::FRONT_RIGHT_OPENING || orient == PBLBaguetteOrientation::FRONT_LEFT_OPENING) ? baseline1 : baseline2;
    
    cv::Ptr<cv::SIFT> sift = cv::SIFT::create();
    std::vector<cv::KeyPoint> keyPointsBaseline, keyPoints;
    cv::Mat descriptorsBaseline, descriptors;
   
    sift->detectAndCompute(baseline, cv::noArray(), keyPointsBaseline, descriptorsBaseline);
    sift->detectAndCompute(transformedImage, cv::noArray(), keyPoints, descriptors);

    float similarityRatio = computeDescriptorSimilarity(descriptorsBaseline, descriptors, keyPointsBaseline, keyPoints, baseline, transformedImage);

    if (similarityRatio > threshold) {
        bagproblem = false;
        Logger::Instance()->Info("Bag structure matches baseline!!");
    }
   
    return bagproblem;
}

cv::Point2f TCSPerception::transformRotation(cv::Point2f point, cv::Mat homography_matrix) {
    cv::Mat pointMatrix = (cv::Mat_<double>(3, 1) << point.x, point.y, 1.0);
    cv::Mat distanceMatrix = homography_matrix * pointMatrix;

    double w = distanceMatrix.at<double>(2, 0);
    double x = distanceMatrix.at<double>(0, 0) / w;
    double y = distanceMatrix.at<double>(1, 0) / w;

    return cv::Point2f(static_cast<float>(x), static_cast<float>(y));
}

//Maybe include this later if orientation result isn't accurate
double TCSPerception::calculateSlope(cv::Point2f p1, cv::Point2f p2) {
    if (p2.x == p1.x) {
        return std::numeric_limits<double>::infinity();
        Logger::Instance()->Info("Both points have the same x coordinate value");
    }
    return (p2.y - p1.y) / (p2.x - p1.x);
}

PBLBaguetteOrientation TCSPerception::calculateBagOrientation(std::shared_ptr<HomographyResult> homography, cv::Rect quadrant1) {
    PBLBaguetteOrientation orient;
    int idx = 0;
    std::shared_ptr<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>> corners = homography->imgMarkers->markers;
    cv::Mat homography_matrix = homography->transformMatrix;
    if (!corners || corners->empty()) {
        return PBLBaguetteOrientation::UNEXPECTED;
    }
    auto markerCorners = (*corners)[idx];
    if (!markerCorners || markerCorners->size() < 2) {
        return PBLBaguetteOrientation::UNEXPECTED;
    }
    cv::Point2f point1 = markerCorners->at(1);
    cv::Point2f point2 = markerCorners->at(0);

    cv::Point2f point1Transform = transformRotation(point1, homography_matrix);

    float length = std::abs(point1.x - point1Transform.x);
    float breadth = std::abs(point1.y - point1Transform.y);
    float pixelThreshold = 200.0f;
    if (quadrant1.contains(point1Transform)) {
        orient = (length < pixelThreshold && breadth < pixelThreshold) ? PBLBaguetteOrientation::FRONT_RIGHT_OPENING : PBLBaguetteOrientation::FRONT_LEFT_OPENING;       
    }
    else {
        orient = (length < pixelThreshold && breadth < pixelThreshold) ? PBLBaguetteOrientation::BACK_LEFT_OPENING : PBLBaguetteOrientation::BACK_RIGHT_OPENING;
    }

    return orient;
}

//To-Do: Add aditional checks for INSULATED bags when we have the final tag/id info
BagType TCSPerception::computeBagType(std::shared_ptr<HomographyResult> homography) {
    BagType bagType;
    std::shared_ptr<std::vector<int>> markerIds = homography->imgMarkers->ids;
    if (markerIds->size() == 0) return BagType::UNKNOWN;
    bagType = (markerIds->size()==1 && ((*markerIds)[0]==5 || (*markerIds)[0] == 0)) ? BagType::AMBIENT : BagType::INSULATED;
    return bagType;
}

//To-Do - Add logging/detailed function descriptions
std::shared_ptr<BagOrientationInference> TCSPerception::getBagOrientation(std::shared_ptr<cv::Mat> bag_image, std::string directoryPath) {
    // Prep save results to local file
    std::string labelFilename = directoryPath + "result.json";

    cv::Mat image = *bag_image;
    auto startTime = std::chrono::steady_clock::now();
     
    auto homog = TCSPerception::getTCSBagTypeAruco()->findImgHomographyFromMarkers(image, directoryPath);
    
    if (homog->transformMatrix.empty()) {
        auto errCode = homog->maxMatchesSeen > 0 ? (TCSErrorCodes::NotEnoughMarkersDetected) : (TCSErrorCodes::NoMarkersDetected);
        Logger::Instance()->Info("Aruco homograhy returned empty results");
        return std::make_shared<BagOrientationInference>(BagOrientationInference{ PBLBaguetteOrientation::UNEXPECTED, BagType::UNKNOWN, false});
    }
    //write_result_file(labelFilename, homog->maxMatchesSeen);
    // Aruco transform the image to a more baseline-like positioning
    if (homog->maxMatchesSeen > 1) {
        auto errCode = TCSErrorCodes::NotEnoughMarkersDetected;
        Logger::Instance()->Info("Detected more than 1 marker on the bag");
    }
   
    auto repinnedImg = TCSPerception::getTCSBagTypeAruco()->applyHomographyToImg(image, homog);
    BagType bagType = computeBagType(homog);
    
    int width = image.cols;
    int height = image.rows;
    cv::Rect quadrant1(width / 2, 0, width / 2, height / 2);
    PBLBaguetteOrientation orientation = calculateBagOrientation(homog, quadrant1);
    Logger::Instance()->Info("Detected Bag Orientation: {}", static_cast<int>(orientation));
    Logger::Instance()->Info("Detected Bag type: {}", static_cast<int>(bagType));
    
    bool bagProblem = bagStructureSimilarity(image, orientation, repinnedImg);
    return std::make_shared<BagOrientationInference>(BagOrientationInference{orientation, bagType, bagProblem});
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