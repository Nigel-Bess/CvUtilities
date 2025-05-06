//
// Created by Priyanka on 9/6/24.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_PERCEPTION_H
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_PERCEPTION_H

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <Fulfil.CPPUtils/logging.h>
#include "aruco/aruco_utils.h"
#include "tcs_response.h"
#include "tcs_error_codes.h"

using fulfil::utils::aruco::ArucoTransforms;
using fulfil::utils::aruco::HomographyResult;

namespace fulfil::dispense::commands::tcs {

    /**
     * Result of inference on a top-down LFB view, specifying whether
     * the bag clip is open or closed
     */
    struct BagClipInference {
        /**
         * Final bool result of whether clip is open or closed
         */
        bool isClosed;
        /**
         * Number between 0-1 representing confidence in isClosed state
         */
        double confidence;
        /**
         * Error preventing any sort of meaningful result, isClosed should
         * be ignored if error is anything but success (0)
         */
        TCSErrorCodes status;
    };

    /**
     * Aggregate result of infering all 4 bag clip states
     */
    struct BagClipsInference {
        /**
         * True if all clips were validly detected in closed state, false if any errors
         */
        bool allClipsClosed;
        /**
         * True if all clips were validly detected in open state, false if any errors
         */
        bool allClipsOpen;

        std::shared_ptr<BagClipInference> topLeftInference;
        std::shared_ptr<BagClipInference> topRightInference;
        std::shared_ptr<BagClipInference> bottomLeftInference;
        std::shared_ptr<BagClipInference> bottomRightInference;
    };

    // TODO
    struct BagLoadInference {
        int x;
    };

    //TO BE REMOVED; if not used by Eric
    /*struct SiftBaseline {

        std::vector<cv::KeyPoint> keypoints;
        cv::Mat descriptors;
        std::vector<int> markerIds;
        std::vector<std::vector<cv::Point2f>> markerCorners;

    };*/

    /* Baguette Orinetation Codes for PBL CV Algorithms */
    enum PBLBaguetteOrientation {
        /* Unexpected bag state encountered in PBL Bag Orientation Algorithm */
        UNEXPECTED = 0,
        /* Bag facing front, opening towards right */
        FRONT_RIGHT_OPENING = 1,
        /* Bag facing front, opening towards left */
        FRONT_LEFT_OPENING = 2,
        /* Bag facing back, opening towards right */
        BACK_RIGHT_OPENING = 3,
        /* Bag facing back, opening towards left */
        BACK_LEFT_OPENING = 4,
    };

    /* Bag Type Codes for PBL CV Algorithms */
    enum BagType {
        /* Unknown */
        UNKNOWN = 0,
        /* Ambient bag type */
        AMBIENT = 1,
        /* Insulated bag type */
        INSULATED = 2, 
    };

    struct BagOrientationInference {
        /*
        * Bag orientation enum
        */
        PBLBaguetteOrientation bagOrientation;

        /*
        * BagType Enum
        */
        BagType bagType;

        /*
        * Flag for detecting problems with bag structure
        */
        bool bagProblem;

    };

    class TCSPerception {

        public:

        static std::shared_ptr<ArucoTransforms> getTCSLFRTopViewAruco();
        static std::shared_ptr<ArucoTransforms> getTCSBagTypeAruco();
        static std::shared_ptr<ArucoTransforms> getTCSToteIDAruco();

        /**
        * TCSPerception Constructor that takes in the lfb generation that will be used to
        */
        TCSPerception();

        /**
         * Inspect a loaded bag from 3 angles to determine if the bag was properly loaded in PBL.
         */
        std::shared_ptr<BagLoadInference> getBagLoadedState(std::shared_ptr<cv::Mat> top_image, std::shared_ptr<cv::Mat> side1_image, std::shared_ptr<cv::Mat> side2_image, std::string directoryPath);

        /**
         * Get the orientation of a bag, bag type and bag structure information.
         */
        std::shared_ptr<BagOrientationInference> getBagOrientation(std::shared_ptr<cv::Mat> bag_image, std::string directoryPath);

        /*
        * Return the visible state of all 4 bag clips.
        */
        std::shared_ptr<BagClipsInference> getBagClipStates(std::shared_ptr<cv::Mat> bag_image, std::string lfrGeneration, std::string directoryPath);

        /*
        * Draw sift structure match keypoints in baseline and test image
        */
        void drawMatchingKeypoints(cv::Mat baseline, std::vector<cv::KeyPoint> keypoints1,
            cv::Mat testImage, std::vector<cv::KeyPoint> keypoints2,
            std::vector<cv::DMatch> matches, cv::Mat siftImage);

        /*
        * Helper function - Calculate the sift descriptor similarity score
        */
        float computeDescriptorSimilarity(cv::Mat baselineDescriptors, cv::Mat descriptors, std::vector<cv::KeyPoint> keyPointsBaseline, std::vector<cv::KeyPoint> keyPoints, cv::Mat baseline, cv::Mat image);

        /*
        * Compare and calculate the similarity score between baseline and test bag image
        */
        bool bagStructureSimilarity(cv::Mat image, PBLBaguetteOrientation orientation, cv::Mat transformedImage);

        /*
        * Convert the test image marker points to baseline points using transformation matrix
        */
        cv::Point2f transformRotation(cv::Point2f point, cv::Mat homography_matrix);

        /*
        * Slope calculation - Basic Mathematic function
        */
        double calculateSlope(cv::Point2f p1, cv::Point2f p2);

        /*
        * Calculate the bag Orientation 
        */
        PBLBaguetteOrientation calculateBagOrientation(std::shared_ptr<HomographyResult> homography, cv::Rect quadrant1);

        /*
        * Calculate the bag type via the marker ids
        */
        BagType computeBagType(std::shared_ptr<HomographyResult> homography);

        //Check if Eric needs any of this functionality
        /*cv::Mat siftDetection(std::shared_ptr<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>> sharedCorners, cv::Mat img);

        void compareSift(cv::Mat baseline_image, cv::Mat image, cv::Mat descriptors_baseline, std::vector<cv::KeyPoint> keypoints_baseline, std::shared_ptr<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>> sharedCorners);

        void customDrawMatches(const cv::Mat img1, const std::vector<cv::KeyPoint> keypoints1,
            const cv::Mat img2, const std::vector<cv::KeyPoint> keypoints2,
            const std::vector<cv::DMatch> matches, cv::Mat outImg);

        bool compareOrientationWithBaseline(cv::Mat image,
            std::shared_ptr<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>> markerCorners, std::shared_ptr<std::vector<int>> markerIds);*/
    };


} // namespace fulfil::dispense::commands::tcs

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_PERCEPTION_H