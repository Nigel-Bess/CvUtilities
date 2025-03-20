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

    //TODO: Refine
    struct BagOrientationInference {
        int orientation; // probly an enum instead
        //int bagType; // Also should be enum
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
         * TODO: Probably not a bool, an object or enum maybe?
         * Get the orientation of a bag as well as bag type information.
         */
        std::shared_ptr<BagOrientationInference> getBagOrientation(std::shared_ptr<cv::Mat> bag_image, std::string directoryPath);

        /*
        * Return the visible state of all 4 bag clips.
        */
        std::shared_ptr<BagClipsInference> getBagClipStates(std::shared_ptr<cv::Mat> bag_image, std::string lfrGeneration, std::string directoryPath);

};

} // namespace fulfil::dispense::commands::tcs

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_PERCEPTION_H