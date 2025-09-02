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
#include <Fulfil.CPPUtils/aruco/aruco_utils.h>
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

        BagClipInference(bool is_closed, double confidence, TCSErrorCodes status, std::string best_name) : is_closed(is_closed), confidence(confidence), status(status), best_name(best_name) {}
        /**
         * Final bool result of whether clip is open or closed
         */
        bool is_closed;
        /**
         * Number between 0-1 representing confidence in is_closed state
         */
        double confidence;
        /**
         * Error preventing any sort of meaningful result, is_closed should
         * be ignored if error is anything but success (0)
         */
        TCSErrorCodes status;

        std::string best_name;
    };

    /**
     * Aggregate result of infering all 4 bag clip states
     */
    struct BagClipsInference {
        BagClipsInference(std::string lfb_cavity_type, bool all_clips_closed, bool all_clips_open, 
        TCSErrorCodes status,
        std::shared_ptr<BagClipInference> top_left_inference,
        std::shared_ptr<BagClipInference> top_right_inference,
        std::shared_ptr<BagClipInference> bottom_left_inference,
        std::shared_ptr<BagClipInference> bottom_right_inference) : status(status), lfb_cavity_type(lfb_cavity_type), all_clips_closed(all_clips_closed), all_clips_open(all_clips_open),
        top_left_inference(top_left_inference), top_right_inference(top_right_inference), bottom_left_inference(bottom_left_inference), bottom_right_inference(bottom_right_inference) {}

        TCSErrorCodes status;
        std::string lfb_cavity_type; // Ex. "LFP-B"
        /**
         * True if all clips were validly detected in closed state, false if any errors
         */
        bool all_clips_closed;
        /**
         * True if all clips were validly detected in open state, false if any errors
         */
        bool all_clips_open;

        std::shared_ptr<BagClipInference> top_left_inference;
        std::shared_ptr<BagClipInference> top_right_inference;
        std::shared_ptr<BagClipInference> bottom_left_inference;
        std::shared_ptr<BagClipInference> bottom_right_inference;
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

    struct SiftEncoding {
        SiftEncoding(std::shared_ptr<std::vector<
            std::shared_ptr<cv::KeyPoint>>> keypoints,
            std::shared_ptr<cv::Mat> descriptors) : keypoints(keypoints), descriptors(descriptors) {}
        std::shared_ptr<std::vector<std::shared_ptr<cv::KeyPoint>>> keypoints;
        std::shared_ptr<cv::Mat> descriptors;
    };

    struct SiftMatch {
        SiftMatch(std::shared_ptr<SiftEncoding> candidate, std::shared_ptr<SiftEncoding> baseline) : candidate(candidate), baseline(baseline) {};
        std::shared_ptr<SiftEncoding> candidate;
        std::shared_ptr<SiftEncoding> baseline;
    };

    struct BagClipBaseline {
        BagClipBaseline(std::shared_ptr<cv::Mat> img, std::string lfb_type_cavity, bool is_closed, int clip_index, std::shared_ptr<SiftEncoding> encoded) : img(img), lfb_type_cavity(lfb_type_cavity), is_closed(is_closed), clip_index(clip_index), encoded(encoded) {};

        std::shared_ptr<cv::Mat> img;
        std::string lfb_type_cavity;
        bool is_closed;
        int clip_index; // 0-4 in order: ul, ur, ll, lr
        std::shared_ptr<SiftEncoding> encoded;
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

        std::shared_ptr<BagClipInference> inferBagClipState(std::map<std::string, std::shared_ptr<cv::Mat>> baseline_to_repinned_img, int clipIndex, std::string lfr_version_and_cavity, std::string directoryPath);

        void write_clip_closed_result_file(std::shared_ptr<BagClipsInference> result, std::string request_id, std::string labelFilename);

        /**
         * Get the orientation of a bag, bag type and bag structure information.
         */
        std::shared_ptr<BagOrientationInference> getBagOrientation(std::shared_ptr<cv::Mat> bag_image, std::string directoryPath);

        /*
        * Return the visible state of all 4 bag clips.
        */
        std::shared_ptr<BagClipsInference> getBagClipStates(std::shared_ptr<cv::Mat> bag_image, std::string lfrGeneration, std::string request_id, std::string directoryPath);

        /*
        * Draw sift structure match keypoints in baseline and test image
        */
        void drawMatchingKeypoints(std::shared_ptr<cv::Mat> baseline, std::shared_ptr<std::vector<std::shared_ptr<cv::KeyPoint>>> keypoints1,
            std::shared_ptr<cv::Mat> testImage, std::shared_ptr<std::vector<std::shared_ptr<cv::KeyPoint>>> keypoints2,
            std::vector<cv::DMatch> matches);

        /*
        * Helper function - Calculate the sift descriptor similarity score
        */
        double computeDescriptorSimilarity(std::shared_ptr<cv::Mat> baselineDescriptors, std::shared_ptr<cv::Mat> descriptors, std::shared_ptr<std::vector<std::shared_ptr<cv::KeyPoint>>> keyPointsBaseline, std::shared_ptr<std::vector<std::shared_ptr<cv::KeyPoint>>> keyPoints, std::shared_ptr<cv::Mat> image, cv::Mat baseline);

        /*
        * Compare and calculate the similarity score between baseline and test bag image
        */
        bool bagStructureSimilarity(std::shared_ptr<cv::Mat> image, PBLBaguetteOrientation orientation, std::shared_ptr<cv::Mat> transformedImage);

        /*
        * Convert the test image marker points to baseline points using transformation matrix
        */
        std::shared_ptr<cv::Point2f> transformRotation(std::shared_ptr<cv::Point2f> point, cv::Mat homography_matrix);

        /*
        * Slope calculation - Basic Mathematic function
        */
        double calculateSlope(std::shared_ptr<cv::Point2f> p1, std::shared_ptr<cv::Point2f> p2);

        /*
        * Calculate the bag Orientation 
        */
        PBLBaguetteOrientation calculateBagOrientation(std::shared_ptr<HomographyResult> homography, cv::Rect quadrant1);

        /*
        * Calculate the bag type via the marker ids
        */
        BagType computeBagType(std::shared_ptr<HomographyResult> homography);

        private:

        /**
         * Crop a hardcoded rectangle out of a preprocessed image that hopefully contains one of 4 clip corners, the exact rectangle
         * is selected based on LFB+cavity+open/closed states that were defined based on baseline images.
         */
        std::shared_ptr<cv::Mat> crop_clip_arm(std::shared_ptr<cv::Mat> image, std::string baseline_name);

        /**
         * Returns a tuple of (is_closed, SiftEncoding)
         */
        std::shared_ptr<SiftEncoding> sift_encode_arm(std::shared_ptr<cv::Mat> image, std::string baseline_name);

        /**
         * Called in constructor only for initializing baseline SIFT data for corner clip detections
         */
        void register_clip_baselines(std::string lfr_and_cavity, bool is_closed, std::string name);

        std::string clip_index_to_hash(int clip_index);

        cv::Ptr<cv::SIFT> sift;

        // Map of LFB+clip state + corner type strings to a tuple of (baseline image name, SIFT keypoints, SIFT descriptors),
        // key name convention is: <LFBTYPE_SLOT_open/closed_ul/ur/ll/lr> to address each clip corner by baseline name / image
        std::map<std::string, std::shared_ptr<BagClipBaseline>>* bag_clip_baselines;
        // Map of orientation name to tuple of (SIFT keypoints, SIFT descriptors), key naming convention is by <orientation>
        std::map<std::string, std::shared_ptr<SiftEncoding>>* bag_orientation_baselines;

        // A mapping from LFB+clip state type to upper-left + lower-right rectangle coordinates containing the clip.
        // A well-cropped clip should extend from the edges of the clip by about 20 pixels on all 4 sides.
        std::map<std::string, std::tuple<cv::Point2f*, cv::Point2f*>*>* bag_clip_bounding_boxes;

    };


} // namespace fulfil::dispense::commands::tcs

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_PERCEPTION_H