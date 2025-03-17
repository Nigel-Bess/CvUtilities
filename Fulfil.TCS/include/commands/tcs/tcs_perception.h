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

using fulfil::utils::aruco::ArucoTransforms;

namespace fulfil::dispense::commands {

    class TCSPerception {

        public:

        static std::shared_ptr<ArucoTransforms> getTCSArucoTransforms();
        std::shared_ptr<std::string> bot_generation;

        /**
        * TCSPerception Constructor that takes in the lfb generation that will be used to
        */
        TCSPerception(std::shared_ptr<std::string> lfb_generation);

        /*
        * Saves the image to the given filepath
        * @param image, image_path
        */
        void SaveImages(cv::Mat image, std::string image_path);

        /*
        * Determines whether the bot in the image at the given filepath is ready for release into the factory
        * This is a check to make sure the bot is fully empty and no items or spills are present
        * @return a tuple with int representing the success_code, boolean value denoting is the bag empty and string for error description
        */
        bool is_bag_clips_closed(std::shared_ptr<cv::Mat> bag_image, std::string directory_path);

        void write_result_file(std::string labelFilename, int markerCount);

};

} // namespace fulfil::dispense::commands

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TCS_PERCEPTION_H