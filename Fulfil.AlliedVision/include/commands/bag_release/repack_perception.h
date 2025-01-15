//
// Created by Priyanka on 9/6/24.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_BAG_RELEASE_REPACK_PERCEPTION_H
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_BAG_RELEASE_REPACK_PERCEPTION_H

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdexcept>
#include <numeric>
#include <Fulfil.CPPUtils/logging.h>
#include "bag_release_response.h"

namespace fulfil::dispense::commands {

    class RepackPerception {
        
        public:

        std::shared_ptr<std::string> bot_generation;

        /*
        ** Success_code - default 0 if the code runs without exceptions, 10 otherwise
        */
        int success_code;

        /*
        ** Bag Empty result flag
        */
        bool is_bag_empty;

        /*
        ** Error description string
        */
        std::string error_description;

        /**
        * RepackPerception Constructor that takes in the lfb generation that will be used to
        */
        RepackPerception(std::shared_ptr<std::string> lfb_generation);

        /*
        * Calculates the distance between 2 points in an image
        * @param 2 points p1 and p2 to calculate the distance
        * @return double representing the distance
        */
        double distance(const cv::Point2f& p1, const cv::Point2f& p2);

        /*
        * Calculates the area of the arucos marker
        * @param x & y pixels for 2 corner coordinates of an aruco
        * @return float representing the area of the aruco tag
        */
        float calculate_square_area(float x1, float x2, float y1, float y2);
        
        /*
        * Checks if the image has some color except just black
        * Useful in detecting if the image got processed without any errors
        * @param input image
        * @return boolean value that denotes if the image has color
        */
        bool image_has_color(cv::Mat image);

        /*
        * Saves the image to the given filepath
        * @param image, image_path
        */
        void SaveImages(cv::Mat image, std::string image_path);
        /*
        * Load the input image to the algorithm 
        * @param input image path
        * @return input image
        */
        cv::Mat load_image(std::string image_path);

        /*
        * Detect the Aruco markers and drawing bounding boxes around them
        * @param input image captured from the repack station camera
        * @return Marker Corner Coordinates for all the Aruco markers
        */
        std::vector<std::vector<cv::Point2f>> detect_aruco_markers(cv::Mat img, std::string directory_path);

        /*
        * Calculates area of each aruco and selects the top 8 arucos with the biggest area(>9500)
        * @param marker corners of all the arucos detecetd
        * @return Marker Corner Coordinates for all the selected Aruco markers
        */
        std::vector<std::vector<cv::Point2f>> marker_selection(std::vector<std::vector<cv::Point2f>> marker_corners);
        /*
        * Calculate centroid from aruco coordinates
        * @param Aruco marker's edge pixel coordinates
        * @return Centroid coordinates for each of the markers
        */
        cv::Point2f calculate_centroid(std::vector<cv::Point2f> points);

        /*
        * Shrink and plot the region of interest using shrinking factors along the x and y axes
        * @param Aruco marker's edge pixel coordinates
        * @param Shrinking factor values for X axes
        * @param Shrinking factor values for Y axes
        * @return Region of Interest coordinates calculated after shrinking the polygon
        */
        std::vector<cv::Point2f> shrink_polygon(std::vector<cv::Point2f> points, float shrink_factor_X, float shrink_factor_Y);

        /*
        * Process image before edge detection. Includes applying Gray scaling and 
        * the Gaussian Blurring to the image to reduce/remove the noise in the image
        * @param Region of Interest for the input image
        * @param Height of the Kernel
        * @param Width of the Kernel
        * @param SigmaX value
        * @return Image obtained after Gray Scaling and Gaussian Blurring
        */
        cv::Mat process_image(cv::Mat region_of_interest, int kernel_height, int kernel_width, int sigmaX);

        /*
        * Detect edges within the Region of Interest, using the Canny algorithm
        * @param Gaussian Blurred Image for Edge Detection
        * @param Edge Threshold
        * @param Lower Threshold for Canny Edge Detection
        * @param Upper Threshold for Canny Edge Detection
        * @param Directory path to save result images
        * @return Print the result statements based on the edge counts
        */
        bool canny_edge_detection(cv::Mat gaussian_image, int edge_threshold, double lower_threshold, double upper_threshold, std::string directory_path);
            
        /*
        * Sorting the pixel coordinates in ascending order
        * @param Coordinate Point Vector
        * @return Sorted Coordinate Point Vector
        */
        std::vector<cv::Point2f> sort_marker_pixel_coordinates(std::vector<cv::Point2f> points, bool with_respect_to_X);

        /*
        * Create a new vector with the 4 outer coordinates for drawing 
        * the Hull(rectangle bounding the region Of Interest)
        * @param Point Array with 8 inner coordinates for aruco tags on the right
        * @param Point Array with 8 inner coordinates for aruco tags on the left
        * @return Vector with 4 corner coordinates
        */
        std::vector<cv::Point2f> get_hull_coordinates(std::vector<cv::Point2f> points_right, std::vector<cv::Point2f> points_left);

        /*
        * Returns the count of markers in the given vector and handles all exceptions about marker count
        */
        int get_marker_count(std::vector<std::vector<cv::Point2f>> marker_coordinate_points);

        /*
        * Calculate the left inner corner coordinates for the Hull
        * @param Aruco Tag corner coordinates
        * @return Left 2 Hull Coordinates
        */
        std::vector<cv::Point2f> left_inner_corner_coordinates(std::vector<std::vector<cv::Point2f>> marker_corners);

        /*
        * Calculate the right inner corner coordinates for the Hull
        * @param Aruco Tag corner coordinates
        * @return Right 2 Hull Coordinates
        */
        std::vector<cv::Point2f> right_inner_corner_coordinates(std::vector<std::vector<cv::Point2f>> marker_Corners);

        /*
        * Calculate the Region of Interest in the image, which will be 
        * the focused to the bot cavity where there would potentially be items
        * @param input image
        * @param Hull Coordinates
        * @param Shrink Factor for X axes
        * @param Shrink Factor for Y axes
        * @param Directory path to save the result images
        * @return Region of Interest of the input image
        */
        cv::Mat calculate_roi(cv::Mat image, std::vector<cv::Point2f> hull_coordinates, float shrink_factor_X, float shrink_factor_Y, std::string directory_path);


        /*
        * Check to detect items in the bag based on the rgb mask
        * @param region of interest image
        * @param directory_path to save the result images
        * @return bool representing if the bag is empty
        */
        bool rgb_is_bag_empty_check(cv::Mat image, std::string directory_path);

        /*
        * Determines whether the bot in the image at the given filepath is ready for release into the factory
        * This is a check to make sure the bot is fully empty and no items or spills are present
        * @return a tuple with int representing the success_code, boolean value denoting is the bag empty and string for error description
        */
        bool is_bot_ready_for_release(std::shared_ptr<cv::Mat> bag_image, std::string directory_path);

        void write_result_file(std::string labelFilename, int markerCount);

};

} // namespace fulfil::dispense::commands

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_BAG_RELEASE_REPACK_PERCEPTION_H