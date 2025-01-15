//
// Created by priyanka on 9/6/24.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <numeric>
#include <cmath>
#include <Fulfil.CPPUtils/logging.h>
#include "repack_perception.h"
#include <tuple>
#include "repack_error_codes.h"
#include <json.hpp>

using fulfil::dispense::commands::get_error_name_from_code;
using fulfil::dispense::commands::RepackErrorCodes;
using fulfil::dispense::commands::RepackPerception;
using fulfil::utils::Logger;

/**
** Threshold for the number of edges to decide object detected
** number of edges more than 4500 inidcate the presence of an object
*/
int edge_threshold;

/**
** Lower Threshold for the Canny Edge Detection Algorithm application.
*/
const double lower_threshold = 50.0;

/**
** Upper Threshold for the Canny Edge Detection Algorithm application.
*/
const double upper_threshold = 150.0;

/**
** Gaussian kernel height for applying gaussian blurring to the ROI
*/
const int kernel_height = 15;

/**
** Gaussian kernel width for applying gaussian blurring to the ROI
*/
const int kernel_width = 15;

/**
** Gaussian kernel standard deviation in X direction for applying gaussian blurring to the ROI
*/
const int sigmaX = 0;

/**
** Shrink Factor for the X axis to draw the Region of Interest by shrinking the
** polygon that maps the inner corners of all 8 aruco tags
*/
const float shrink_factor_X = 0.78f;

/*
** Shrink Factor for the Y axis to draw the Region of Interest by shrinking the
** polygon that maps the inner corners of all 8 aruco tags
*/
const float shrink_factor_Y = 0.92f;

RepackPerception::RepackPerception(std::shared_ptr<std::string> lfb_generation)
{
    this->bot_generation = lfb_generation;
    if (*bot_generation == "LFB-3.1")
    {
        edge_threshold = 200;
    }
    else
    {
        edge_threshold = 300;
    }

    // defaults for the BagReleaseResponse
    this->success_code = RepackErrorCodes::Success;
    this->is_bag_empty = false;
    this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code);
}

double RepackPerception::distance(const cv::Point2f &p1, const cv::Point2f &p2)
{
    return std::sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
}

float RepackPerception::calculate_square_area(float x1, float x2, float y1, float y2)
{
    float edge_length = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    return pow(edge_length, 2);
}

bool RepackPerception::image_has_color(cv::Mat image)
{
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            cv::Vec3b pixel_color = image.at<cv::Vec3b>(i, j);
            int pixel_color_blue = pixel_color[0];
            int pixel_color_green = pixel_color[1];
            int pixel_color_red = pixel_color[2];
            if (pixel_color_blue != 0 || pixel_color_green != 0 || pixel_color_red != 0)
            {
                return true;
            }
        }
    }
    return false;
}

void RepackPerception::SaveImages(cv::Mat image, std::string image_path)
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
        Logger::Instance()->Error("RepackPerception::SaveImages caught error: {}", ex.what());
    }
    catch (...)
    {
        Logger::Instance()->Error("RepackPerception::SaveImages hit error in catch(...)");
    }
}

cv::Mat RepackPerception::load_image(std::string image_path)
{
    try
    {
        cv::Mat image = cv::imread(image_path, cv::IMREAD_COLOR);
        if (image.empty() || !image_has_color(image))
        {
            throw std::invalid_argument("Error in load_image. Image not found");
        }
        std::string log_string = "Input Image has height " + std::to_string(image.rows) + " and width " + std::to_string(image.cols);
        Logger::Instance()->Info(log_string);
        return image;
    }
    catch (const std::exception &e)
    {
        this->success_code = RepackErrorCodes::UnspecifiedError;
        this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Caught exception in load_image:") + e.what();
        Logger::Instance()->Error(this->error_description);
        throw;
    }
}

std::vector<std::vector<cv::Point2f>> RepackPerception::detect_aruco_markers(cv::Mat img, std::string directory_path)
{
    try
    {
        cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
        parameters->minCornerDistanceRate = 0.24;
        cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);
        std::vector<std::vector<cv::Point2f>> corners;
        std::vector<int> ids;
        std::vector<std::vector<cv::Point2f>> rejected;
        cv::aruco::detectMarkers(img, dictionary, corners, ids, parameters, rejected);
        if (rejected.size() > 0)
        {
            cv::aruco::drawDetectedMarkers(img, rejected, ids, cv::Scalar(0, 255, 0));
            std::string image_path = directory_path + "markers_detected.jpeg";
            SaveImages(img, image_path);
        }
        int num_markers = get_marker_count(rejected);
        Logger::Instance()->Info("RepackPerception's detect_aruco_markers detected {} markers", num_markers);
        return rejected;
    }
    catch (const std::exception &e)
    {
        this->success_code = RepackErrorCodes::UnspecifiedError;
        this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Caught exception in detect_aruco_markers: ") + e.what();
        Logger::Instance()->Error(this->error_description);
        throw;
    }
}

std::vector<std::vector<cv::Point2f>> RepackPerception::marker_selection(std::vector<std::vector<cv::Point2f>> marker_corners)
{
    try
    {
        std::vector<std::vector<cv::Point2f>> markers;
        for (int i = 0; i < marker_corners.size(); i++)
        {
            float x1 = marker_corners[i][0].x;
            float x2 = marker_corners[i][1].x;
            float y1 = marker_corners[i][0].y;
            float y2 = marker_corners[i][1].y;
            float area = calculate_square_area(x1, x2, y1, y2);
            if (area > 9500.0)
                markers.push_back(marker_corners[i]);
            Logger::Instance()->Info("Area of marker " + std::to_string(i) + " : " + std::to_string(area));
        }
        return markers;
    }
    catch (const std::exception &e)
    {
        this->success_code = RepackErrorCodes::UnspecifiedError;
        this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Caught exception in marker_selection: ") + e.what();
        Logger::Instance()->Error(this->error_description);
        throw;
    }
}

cv::Point2f RepackPerception::calculate_centroid(std::vector<cv::Point2f> points)
{
    cv::Point2f centroid_points;
    try
    {
        if (points.size() == 0)
            throw std::invalid_argument("Invalid input in calculate_centroid function: points was empty");
        cv::Point2f centroid_points;
        float centroidX = 0.0;
        float centroidY = 0.0;
        for (cv::Point2f vertex : points)
        {
            centroidX = centroidX + vertex.x;
        }
        for (cv::Point2f vertex : points)
        {
            centroidY = centroidY + vertex.y;
        }
        centroidX = centroidX / points.size();
        centroidY = centroidY / points.size();
        centroid_points.x = centroidX;
        centroid_points.y = centroidY;
        std::string log_string = "Centroid calculated";
        Logger::Instance()->Info(log_string);
        return centroid_points;
    }
    catch (const std::exception &e)
    {
        this->success_code = RepackErrorCodes::UnspecifiedError;
        this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Caught exception in calculate_centroid: ") + e.what();
        Logger::Instance()->Error(this->error_description);
        throw;
    }
}

std::vector<cv::Point2f> RepackPerception::shrink_polygon(std::vector<cv::Point2f> points, float shrink_factor_X, float shrink_factor_Y)
{
    try
    {
        cv::Point2f centroid = calculate_centroid(points);
        std::vector<cv::Point2f> shrunk_polygon;
        for (cv::Point2f vertex : points)
        {
            float newX = centroid.x + (vertex.x - centroid.x) * shrink_factor_X;
            float newY = centroid.y + (vertex.y - centroid.y) * shrink_factor_Y;
            cv::Point2f point;
            point.x = newX;
            point.y = newY;
            shrunk_polygon.push_back(point);
        }
        std::string log_string = "Polygon shrink completed";
        Logger::Instance()->Info(log_string);
        return shrunk_polygon;
    }
    catch (const std::exception &e)
    {
        this->success_code = RepackErrorCodes::UnspecifiedError;
        this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Error in shrink_polygon : ") + std::to_string(this->success_code) + ": " + e.what();
        Logger::Instance()->Error(this->error_description);
        throw;
    }
}

cv::Mat RepackPerception::process_image(cv::Mat region_of_interest, int kernel_height, int kernel_width, int sigmaX)
{
    try
    {
        cv::Mat gray_scaling;
        cv::Mat gaussian_image;
        std::string log_string = "Processing the Region Of Interest";
        cv::cvtColor(region_of_interest, gray_scaling, cv::COLOR_BGR2GRAY);
        log_string += " | Image Gray Scaled";
        cv::GaussianBlur(gray_scaling, gaussian_image, cv::Size(kernel_width, kernel_height), sigmaX);
        Logger::Instance()->Info(log_string);
        return gaussian_image;
    }
    catch (const std::exception &e)
    {
        this->success_code = RepackErrorCodes::UnspecifiedError;
        this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Error in process_image : ") + std::to_string(this->success_code) + ": " + e.what();
        Logger::Instance()->Error(this->error_description);
        throw;
    }
}

bool RepackPerception::canny_edge_detection(cv::Mat gaussian_image, int edge_threshold, double lower_threshold, double upper_threshold, std::string directory_path)
{
    try
    {
        int height = gaussian_image.rows;
        int width = gaussian_image.cols;
        std::string log_string = "Executing canny_edge_detection on image of height " + std::to_string(height) + " and width " + std::to_string(width);
        cv::Mat canny_edges;
        cv::Canny(gaussian_image, canny_edges, lower_threshold, upper_threshold);
        log_string += " | Edge Detection implemention success";
        int non_zero_edges = cv::countNonZero(canny_edges);
        bool bag_empty = true;
        if (non_zero_edges > edge_threshold)
        {
            bag_empty = false;
        }
        log_string += " | Edges Detected: " + std::to_string(non_zero_edges) + " and Object Detected : " + std::to_string(!bag_empty);
        Logger::Instance()->Info(log_string);
        std::string image_path = directory_path + "canny_edges.jpeg";
        SaveImages(canny_edges, image_path);
        return bag_empty;
    }
    catch (const std::exception &e)
    {
        this->success_code = RepackErrorCodes::UnspecifiedError;
        this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Error in canny_edge_detection : ") + std::to_string(this->success_code) + ": " + e.what();
        Logger::Instance()->Error(this->error_description);
        throw;
    }
}

std::vector<cv::Point2f> RepackPerception::sort_marker_pixel_coordinates(std::vector<cv::Point2f> points, bool with_respect_to_X)
{
    std::vector<cv::Point2f> sorted = points;
    if (with_respect_to_X)
    {
        std::sort(sorted.begin(), sorted.end(), [](const cv::Point2f &a, const cv::Point2f &b)
                  { return a.x < b.x; });
    }
    else
    {
        std::sort(sorted.begin(), sorted.end(), [](const cv::Point2f &a, const cv::Point2f &b)
                  { return a.y < b.y; });
    }
    return sorted;
}

std::vector<cv::Point2f> RepackPerception::get_hull_coordinates(std::vector<cv::Point2f> points_right, std::vector<cv::Point2f> points_left)
{
    std::vector<cv::Point2f> coordinates;
    std::string log_left = "Size of Left : " + std::to_string(points_left.size());
    Logger::Instance()->Info(log_left);
    std::string log_right = "Size of Right : " + std::to_string(points_right.size());
    Logger::Instance()->Info(log_right);
    coordinates.push_back(points_right[7]);
    coordinates.push_back(points_left[7]);
    coordinates.push_back(points_left[0]);
    coordinates.push_back(points_right[0]);
    Logger::Instance()->Info("Got Hull coordinates successfully");
    return coordinates;
}

int RepackPerception::get_marker_count(std::vector<std::vector<cv::Point2f>> marker_coordinate_points)
{
    int detected_markers_count = marker_coordinate_points.size();
    std::string count_log = "RepackPerception's get_marker_count: Expecting 8 marker corners and got " + std::to_string(detected_markers_count);
    Logger::Instance()->Debug(count_log);
    // if less markers present than the bot number, the algorithm will have weird behavior
    if (detected_markers_count < 8)
    {
        // not enough markers detected code
        this->success_code = RepackErrorCodes::NotEnoughMarkersDetected;

        // 1 marker detected may be the grid's april tag, not a bot
        if (detected_markers_count < 2)
        {
            this->success_code = RepackErrorCodes::NoMarkersDetected;
        }
        this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Error in get_marker_count with success code ") + std::to_string(this->success_code) + ": " + count_log;
        Logger::Instance()->Error(this->error_description);
        throw std::runtime_error(this->error_description + " -> " + std::string("In get_marker_count: Is no bot in image? " + count_log));
    }
    return detected_markers_count;
}

std::vector<cv::Point2f> RepackPerception::left_inner_corner_coordinates(std::vector<std::vector<cv::Point2f>> marker_corners)
{
    try
    {
        std::vector<cv::Point2f> left_coordinates;
        std::vector<cv::Point2f> points_left;
        std::vector<cv::Point2f> points;
        bool with_respect_to_X = true;
        int num_markers_to_examine = std::min(8, get_marker_count(marker_corners));
        for (int i = 0; i < num_markers_to_examine; i++)
        {
            std::vector<cv::Point2f> corner = marker_corners[i];
            points_left = sort_marker_pixel_coordinates(corner, with_respect_to_X);
            if (points_left[0].x < 800)
            {
                left_coordinates.push_back(points_left[2]);
                left_coordinates.push_back(points_left[3]);
            }
        }
        with_respect_to_X = false;
        points = sort_marker_pixel_coordinates(left_coordinates, with_respect_to_X);
        Logger::Instance()->Info("Got Left Inner Coordinates");
        return points;
    }
    catch (const std::exception &e)
    {
        if (this->success_code == RepackErrorCodes::Success)
        {
            this->success_code = RepackErrorCodes::UnspecifiedError;
            this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Error in left_inner_corner_coordinates : ") + std::to_string(this->success_code) + ": " + e.what();
            Logger::Instance()->Error(this->error_description);
        }
        throw;
    }
}

std::vector<cv::Point2f> RepackPerception::right_inner_corner_coordinates(std::vector<std::vector<cv::Point2f>> marker_corners)
{
    try
    {
        std::vector<cv::Point2f> right_coordinates;
        std::vector<cv::Point2f> points_right;
        std::vector<cv::Point2f> points;
        bool with_respect_to_X = true;
        int num_markers_to_examine = std::min(8, get_marker_count(marker_corners));
        for (int i = 0; i < num_markers_to_examine; i++)
        {
            std::vector<cv::Point2f> corner = marker_corners[i];
            points_right = sort_marker_pixel_coordinates(corner, with_respect_to_X);
            if (corner[0].x > 800)
            {
                right_coordinates.push_back(points_right[0]);
                right_coordinates.push_back(points_right[1]);
            }
        }
        with_respect_to_X = false;
        points = sort_marker_pixel_coordinates(right_coordinates, with_respect_to_X);
        std::string log_string = "Got Right Inner Coordinates";
        Logger::Instance()->Info(log_string);
        return points;
    }
    catch (const std::exception &e)
    {
        if (this->success_code == RepackErrorCodes::Success)
        {
            this->success_code = RepackErrorCodes::UnspecifiedError;
            this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Error in right_inner_corner_coordinates : ") + std::to_string(this->success_code) + ": " + e.what();
            Logger::Instance()->Error(this->error_description);
        }
        throw;
    }
}

cv::Mat RepackPerception::calculate_roi(cv::Mat image, std::vector<cv::Point2f> hull_coordinates, float shrink_factor_X, float shrink_factor_Y, std::string directory_path)
{
    try
    {
        cv::Mat region_of_interest;
        std::vector<cv::Point2f> hull_points2f;
        cv::convexHull(hull_coordinates, hull_points2f);
        std::vector<cv::Point2f> shrink_hull2f = shrink_polygon(hull_points2f, shrink_factor_X, shrink_factor_Y);
        std::vector<cv::Point> hull_points;
        std::vector<cv::Point> shrink_hull;
        for (const auto &p : hull_points2f)
        {
            hull_points.emplace_back(cv::Point(cv::saturate_cast<int>(std::round(p.x)),
                                               cv::saturate_cast<int>(std::round(p.y))));
        }
        for (const auto &p : shrink_hull2f)
        {
            shrink_hull.emplace_back(cv::Point(cv::saturate_cast<int>(std::round(p.x)),
                                               cv::saturate_cast<int>(std::round(p.y))));
        }
        cv::polylines(image, hull_points, true, cv::Scalar(0, 255, 0), 0);
        cv::polylines(image, shrink_hull, true, cv::Scalar(0, 0, 255), 0);
        std::string path = directory_path + "hull_image.jpeg";
        SaveImages(image, path);
        shrink_hull2f = shrink_polygon(shrink_hull2f, 0.99f, 0.99f);
        // calculate lengths of the sides of the new ROI image
        double top = distance(shrink_hull2f[2], shrink_hull2f[3]);
        double bottom = distance(shrink_hull2f[0], shrink_hull2f[1]);
        double new_width = std::max(top, bottom);
        double left = distance(shrink_hull2f[1], shrink_hull2f[2]);
        double right = distance(shrink_hull2f[0], shrink_hull2f[3]);
        double new_height = std::max(left, right);
        std::vector<cv::Point2f> roiPoints;
        roiPoints.push_back(cv::Point2f(0, 0));
        roiPoints.push_back(cv::Point2f(new_width, 0));
        roiPoints.push_back(cv::Point2f(new_width, new_height));
        roiPoints.push_back(cv::Point2f(0, new_height));
        cv::Mat transformation_matrix = cv::getPerspectiveTransform(shrink_hull2f, roiPoints);
        cv::warpPerspective(image, region_of_interest, transformation_matrix, cv::Size(new_width, new_height));
        int height = region_of_interest.rows;
        int width = region_of_interest.cols;
        Logger::Instance()->Info("Region of Interest has height {} and width {}", std::to_string(height), std::to_string(width));
        std::string image_path = directory_path + "region_of_interest.jpeg";
        SaveImages(region_of_interest, image_path);
        return region_of_interest;
    }
    catch (const std::exception &e)
    {
        this->success_code = RepackErrorCodes::UnspecifiedError;
        this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Error in calculate_roi : ") + std::to_string(this->success_code) + ": " + e.what();
        Logger::Instance()->Error(this->error_description);
        throw;
    }
}

bool RepackPerception::rgb_is_bag_empty_check(cv::Mat region_of_interest, std::string directory_path)
{
    bool bag_empty = true;
    std::vector<cv::Mat> rgb_channels;
    cv::split(region_of_interest, rgb_channels);
    cv::Mat red, green, blue;
    // create a mask with the red, green and blue channels
    cv::inRange(rgb_channels[2], cv::Scalar(100), cv::Scalar(255), red);
    cv::inRange(rgb_channels[1], cv::Scalar(0), cv::Scalar(100), green);
    cv::inRange(rgb_channels[0], cv::Scalar(0), cv::Scalar(100), blue);
    cv::Mat rgb_mask = red & green & blue;
    std::vector<cv::Vec4i> hierarchy;
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(rgb_mask, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    cv::Mat result = region_of_interest.clone();
    cv::drawContours(result, contours, -1, cv::Scalar(0, 255, 0), 2);
    std::string image_path = directory_path + "rgb_contours.jpeg";
    SaveImages(rgb_mask, image_path);
    std::string log_string = "Contour Size from RGB mask: " + std::to_string(contours.size());
    Logger::Instance()->Info(log_string);
    if (contours.size() > 10)
    {
        bag_empty = false;
        Logger::Instance()->Info("Item detected in RGB check!");
    }
    else
    {
        Logger::Instance()->Info("No Item detected in RGB check. Bag may contain white/clear items or may be empty.");
    }
    return bag_empty;
}

void RepackPerception::write_result_file(std::string labelFilename, int markerCount) {
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

bool RepackPerception::is_bot_ready_for_release(std::shared_ptr<cv::Mat> bag_image, std::string directory_path)
{
    // default if any errors are encountered in the algorithm is true, to assume the bag is empty
    try
    {
        cv::Mat image = *bag_image;
        bool bag_has_no_items = true;
        // TODO exit cleanly between sequence if any point in the sequence fails instead of continuing on
        std::vector<std::vector<cv::Point2f>> marker_corners = detect_aruco_markers(image, directory_path);
        std::vector<std::vector<cv::Point2f>> marker_corners_selected = marker_selection(marker_corners);
        std::vector<cv::Point2f> points_left = left_inner_corner_coordinates(marker_corners_selected);
        std::vector<cv::Point2f> points_right = right_inner_corner_coordinates(marker_corners_selected);
        std::vector<cv::Point2f> hull_coordinates = get_hull_coordinates(points_right, points_left);
        cv::Mat region_of_interest = calculate_roi(image, hull_coordinates, shrink_factor_X, shrink_factor_Y, directory_path);
        bag_has_no_items = rgb_is_bag_empty_check(region_of_interest, directory_path);
        std::string info = std::string("Is bot ready to release after RGB check : ") + std::to_string(bag_has_no_items);
        Logger::Instance()->Info(info);
        if (bag_has_no_items == true)
        {
            Logger::Instance()->Info("Starting the Edge Threshold check!");
            cv::Mat gaussian_image = process_image(region_of_interest, kernel_height,
                                                   kernel_width, sigmaX);
            bag_has_no_items = canny_edge_detection(gaussian_image, edge_threshold,
                                                    lower_threshold, upper_threshold, directory_path);
            std::string info = std::string("Is bot ready to release after Edge Threshold check : ") + std::to_string(bag_has_no_items);
            Logger::Instance()->Info(info);
        }
        this->is_bag_empty = bag_has_no_items;

        // Save results to local file
        std::string labelFilename = directory_path + "result.json";
        write_result_file(labelFilename, marker_corners.size());

        Logger::Instance()->Info("Returning is_bag_empty: {}", std::to_string(bag_has_no_items));
        return this->is_bag_empty;
    }
    catch (const std::exception &e)
    {
        Logger::Instance()->Trace("RepackPerception is_bot_ready_for_release in catch exception");
        if (this->success_code == RepackErrorCodes::Success)
        {
            this->success_code = RepackErrorCodes::UnspecifiedError;
            this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Caught exception in is_bot_ready_for_release: ") + e.what();
            Logger::Instance()->Error(this->error_description);
        }
        else
        {
            Logger::Instance()->Debug("RepackPerception is_bot_ready_for_release in catch exception block already has success_code {} so no error fields will be updated", this->success_code);
        }
        return this->is_bag_empty;
    }
    catch (...)
    {
        Logger::Instance()->Trace("RepackPerception is_bot_ready_for_release in catch (...) block");
        if (this->success_code == RepackErrorCodes::Success)
        {
            this->success_code = RepackErrorCodes::UnspecifiedError;
            this->error_description = get_error_name_from_code((RepackErrorCodes)this->success_code) + " -> " + std::string("Caught error in is_bot_ready_for_release in catch(...)!");
            Logger::Instance()->Error(this->error_description);
        }
        else
        {
            Logger::Instance()->Debug("RepackPerception is_bot_ready_for_release in catch (...) block already has success_code {} so no error fields will be updated", this->success_code);
        }
    }
    return this->is_bag_empty;
}