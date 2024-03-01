//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of visualization functions
 * to display point clouds.
 */
#include <librealsense2/rsutil.h>
#include <opencv2/aruco.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <Fulfil.DepthCam/aruco/marker.h>
#include <Fulfil.DepthCam/point_cloud.h>
#include <Fulfil.DepthCam/visualization/session_visualizer.h>
#include "colored_depth_display.h"

using std::shared_ptr;
using std::string;
using std::make_shared;
using fulfil::depthcam::aruco::Marker;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::visualization::SessionVisualizer;

void SessionVisualizer::display_pixels(std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> pixels,
                                       int red,
                                       int green,
                                       int blue,
                                       int radius,
                                       int thickness)
{
    shared_ptr<cv::Mat> mat = session->get_color_mat();
    cv::Mat image(*mat);

    cv::Mat outImage;
    image.copyTo(outImage);

    for(int i = 0; i < pixels->size(); i++)
    {
        cv::circle(outImage, *pixels->at(i), radius, cv::Scalar(blue,green,red), thickness);
    }

    // // Display in a GUI
    cv::namedWindow(this->window_name->c_str(), cv::WINDOW_NORMAL );
    cv::imshow(this->window_name->c_str(), outImage);
    cv::resizeWindow(this->window_name->c_str(), this->window_size->first, this->window_size->second );
    cv::moveWindow(this->window_name->c_str(), this->window_location->first, this->window_location->second);
    cv::waitKey(this->wait_time);
}

SessionVisualizer::SessionVisualizer(shared_ptr<Session> session, shared_ptr<std::string> window_name, shared_ptr<std::pair <int, int>> window_location, shared_ptr<std::pair <int, int>> window_size, int wait_time)
{
    this->session = session;
    this->window_name = window_name;
    this->window_location = window_location;
    this->window_size = window_size;
    this->wait_time = wait_time;
}

//prints out the pixel of the image when double-clicked
void onMouse(int event, int x, int y, int flags, void* param)
{
  if (event == cv::EVENT_LBUTTONDBLCLK)
  {
    std::cout << "x= " << x << " y= " << y << std::endl;
  }
}


void SessionVisualizer::display_rgb_image(std::shared_ptr<cv::Mat> image)
{
  // // Display in a GUI
  cv::namedWindow(this->window_name->c_str(), cv::WINDOW_NORMAL);

  //allows for printing out pixel coordinate when image is double clicked
  cv::setMouseCallback(this->window_name->c_str(), onMouse, 0);

  cv::imshow(this->window_name->c_str(), *image);
  cv::resizeWindow(this->window_name->c_str(), this->window_size->first, this->window_size->second );
  cv::moveWindow(this->window_name->c_str(), this->window_location->first, this->window_location->second);
  cv::waitKey(this->wait_time);
}

void SessionVisualizer::display_rgb_image_with_crosshair(std::shared_ptr<cv::Mat> image)
{
  // // Display in a GUI
  cv::namedWindow(this->window_name->c_str(), cv::WINDOW_NORMAL);

  //allows for printing out pixel coordinate when image is double clicked
  cv::setMouseCallback(this->window_name->c_str(), onMouse, 0);


  cv::Point2f point1 = cv::Point2f(640,0);
  cv::Point2f point2 = cv::Point2f(640,720);
  cv::Point2f point3 = cv::Point2f(0,360);
  cv::Point2f point4 = cv::Point2f(1280,360);

  cv::Point2f point5 = cv::Point2f(60,20);
  cv::Point2f point6 = cv::Point2f(60,700);
  cv::Point2f point7 = cv::Point2f(1220,20);
  cv::Point2f point8 = cv::Point2f(1220,700);

  cv::line(*image, point1, point2, cv::Scalar(0,0,255), 1);
  cv::line(*image, point3, point4, cv::Scalar(0,0,255), 1);

  cv::line(*image, point5, point6, cv::Scalar(0,0,255), 1);
  cv::line(*image, point7, point8, cv::Scalar(0,0,255), 1);
  cv::line(*image, point5, point7, cv::Scalar(0,0,255), 1);
  cv::line(*image, point6, point8, cv::Scalar(0,0,255), 1);


  cv::imshow(this->window_name->c_str(), *image);
  cv::resizeWindow(this->window_name->c_str(), this->window_size->first, this->window_size->second );
  cv::moveWindow(this->window_name->c_str(), this->window_location->first, this->window_location->second);
  cv::waitKey(this->wait_time);
}

void SessionVisualizer::display_point_cloud(bool should_include_invalid_points)
{
  this->display_pixels(this->session->get_point_cloud(should_include_invalid_points)->as_pixel_cloud()->get_data(),
                       0,
                       0,
                       0,
                       0,
                       0);
}

void SessionVisualizer::display_points(shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud, int red, int green,int blue,int radius,int thickness)
{
  this->display_pixels(point_cloud->as_pixel_cloud()->get_data(), red, green, blue, radius, thickness);
}

void SessionVisualizer::display_rectangle(std::shared_ptr<fulfil::utils::Point3D> center,
    double width, double length)
{
    std::shared_ptr<Eigen::Matrix3Xd> point_cloud_data
            = std::shared_ptr<Eigen::Matrix3Xd>(new Eigen::Matrix3Xd(3, 2));
    (*point_cloud_data)(0,0) = center->x - width/2;
    (*point_cloud_data)(1,0) = center->y - length/2;
    (*point_cloud_data)(2,0) = center->z;
    (*point_cloud_data)(0,1) = center->x + width/2;
    (*point_cloud_data)(1,1) = center->y + length/2;
    (*point_cloud_data)(2,1) = center->z;
    this->display_points(this->session->get_point_cloud(true)->as_camera_cloud()
    ->new_point_cloud(point_cloud_data));
}

std::shared_ptr<cv::Mat> SessionVisualizer::draw_detected_markers(cv::Ptr<cv::aruco::Dictionary> dictionary, std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>>> markers,
                                              int region_max_x,
                                              int region_min_x,
                                              int region_max_y,
                                              int region_min_y)
{
    std::shared_ptr<cv::Mat> image = this->session->get_color_mat();

    /**
     *  Detect and display all markers on image
     */
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners, rejected;
    cv::Ptr<cv::aruco::DetectorParameters> detectorParams = cv::aruco::DetectorParameters::create();
    cv::aruco::detectMarkers(*image, dictionary, corners, ids, detectorParams, rejected);
    cv::Mat outImage;
    image->copyTo(outImage);
    cv::aruco::drawDetectedMarkers(outImage, corners, ids, cv::Scalar(100, 0, 255));

    /**
    *  Display valid search region on image
    */
    /**
    cv::Point2f point1 = cv::Point2f(region_min_x , region_min_y);
    cv::Point2f point2 = cv::Point2f(region_max_x , region_min_y);
    cv::Point2f point3 = cv::Point2f(region_min_x , region_max_y);
    cv::Point2f point4 = cv::Point2f(region_max_x , region_max_y);

    cv::line(outImage, point1, point2, cv::Scalar(0,255,0), 2);
    cv::line(outImage, point1, point3, cv::Scalar(0,255,0), 2);
    cv::line(outImage, point2, point4, cv::Scalar(0,255,0), 2);
    cv::line(outImage, point3, point4, cv::Scalar(0,255,0), 2);
    **/

   /**
   *  Mark valid markers with green circles
   */
    for(int i = 0; i < markers->size(); i++)
    {
      assert (markers->at(i) != nullptr); // include for dev purposes, can remove later

      std::shared_ptr<Marker> marker = markers->at(i);
      float marker_x = marker->get_coordinate(Marker::Coordinate::center)->x;
      float marker_y = marker->get_coordinate(Marker::Coordinate::center)->y;
      cv::circle(outImage, cv::Point2f(marker_x, marker_y), 25, cv::Scalar(0,255,0), 2);
    }
    return std::make_shared<cv::Mat>(outImage);
}

std::shared_ptr<cv::Mat> SessionVisualizer::display_points_with_depth_coloring(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud)
{
    auto depth_pixels = point_cloud->as_pixel_cloud()->get_data_with_depth();
    shared_ptr<cv::Mat> image = this->session->get_color_mat();

    cv::Mat outImage;
    image->copyTo(outImage);  //added to prevent persistance across visualizations

    // Display in a GUI
    float max_dist = -1000;
    float min_dist = 1000;
    for(int i = 0; i < depth_pixels->size(); i++)
    {
        max_dist = std::max(max_dist, (float) depth_pixels->at(i)->second);
        min_dist = std::min(min_dist, (float) depth_pixels->at(i)->second);
    }
    shared_ptr<ColoredDepthDisplay> calc = make_shared<ColoredDepthDisplay>(min_dist, max_dist);
    for(int i = 0; i < depth_pixels->size(); i++) {
        // color the max Z points in bright green
//        if ((int)(min_dist * 1000) == (int) (depth_pixels->at(i)->second * 1000)) {
//            cv::circle(outImage, *depth_pixels->at(i)->first, 1, cv::Scalar(0, 255, 0), 3);
//        } else {
            cv::circle(outImage, *depth_pixels->at(i)->first, 1,
                       calc->color_at_depth((float) (depth_pixels->at(i)->second)), 3);
        //}
    }

    return make_shared<cv::Mat>(outImage);
}


std::shared_ptr<cv::Mat> SessionVisualizer::display_points_with_local_depth_coloring(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud,
                                                                                     std::shared_ptr<cv::Mat> image_input)
{
  auto depth_pixels = point_cloud->as_pixel_cloud()->get_data_with_depth();
  std::shared_ptr<cv::Mat> image;
  if(image_input == nullptr)
  {
    image = this->session->get_color_mat();
  }
  else
  {
    image = image_input;
  }

  std::shared_ptr<LocalPointCloud> local_point_cloud = point_cloud->as_pixel_cloud()->new_point_cloud(depth_pixels)->as_local_cloud();
  auto local_depth_data = local_point_cloud->get_data();
  //std::cout << (*local_depth_data)(2,0) << "LOOK HERE" << std::endl;
  //std::cout << *local_depth_data << "LOOK HERE" << std::endl;

  cv::Mat outImage;
  image->copyTo(outImage);  //added to prevent persistance across visualizations

  // Display in a GUI
  float max_dist = -1000;
  float min_dist = 1000;
  for(int i = 0; i < depth_pixels->size(); i++)
  {
    //std::cout << depth_pixels->at(i)->second;
    max_dist = std::max(max_dist, (float) (*local_depth_data)(2,i));
    min_dist = std::min(min_dist, (float) (*local_depth_data)(2,i));
  }
  shared_ptr<ColoredDepthDisplay> calc = make_shared<ColoredDepthDisplay>(min_dist, max_dist);
  for(int i = 0; i < depth_pixels->size(); i++)
  {
    cv::circle(outImage, *depth_pixels->at(i)->first, 1, calc->color_at_depth((float)((*local_depth_data)(2,i))), 3);
  }

  return make_shared<cv::Mat>(outImage);
}

std::shared_ptr<cv::Mat> SessionVisualizer::display_perimeter(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> perimeter,
                                                              std::shared_ptr<cv::Mat> image, int color = 1,
                                                              int thickness = 2)
{
  shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> pixels = perimeter->as_pixel_cloud()->get_data();

  auto color_nums = cv::Scalar(0,0,0);
  if (color == 1) color_nums = cv::Scalar(0, 255, 0);
  if (color == 2) color_nums = cv::Scalar(0, 0, 255);
  if (color == 3) color_nums = cv::Scalar(255, 0, 0);

  for(int i = 0; i < pixels->size(); i++)
  {
    cv::line(*image, *pixels->at(i), *pixels->at((i+1)%(pixels->size())), color_nums, thickness);
  }

  return image;
}

void SessionVisualizer::display_perimeters(std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud>>> perimeters)
{
  shared_ptr<cv::Mat> image = this->session->get_color_mat();
  for(int i = 0; i < perimeters->size(); i++)
  {
    shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> pixels = perimeters->at(i)->as_pixel_cloud()->get_data();
    for(int i = 0; i < pixels->size(); i++)
    {
      cv::line(*image, *pixels->at(i), *pixels->at((i+1)%(pixels->size())), cv::Scalar(0,0,255));
    }
  }
  cv::namedWindow(this->window_name->c_str(), cv::WINDOW_NORMAL);
  cv::imshow(this->window_name->c_str(), *image);
  cv::resizeWindow(this->window_name->c_str(), this->window_size->first, this->window_size->second );
  cv::moveWindow(this->window_name->c_str(), this->window_location->first, this->window_location->second);
  cv::waitKey(0);
}

std::shared_ptr<cv::Mat> SessionVisualizer::display_color_filter(std::shared_ptr<cv::Mat> image, int green_min_intensity, int red_max_intensity, int blue_max_intensity)
{
  for (int i = 0; i < image->rows; i++)
  {
    for(int j = 0; j < image->cols; j++)
    {
      cv::Vec3b intensity = image->at<cv::Vec3b>(i, j);
      int blue = intensity.val[0];
      int green = intensity.val[1];
      int red = intensity.val[2];

      if (green > green_min_intensity && red < red_max_intensity && blue < blue_max_intensity)  //Todo: figure out why seg fault happens if run multiple images in a row and set pixels across entire image
      {
        //std::cout << "Row: " << i << "  Col: " << j <<  "  Blue value is: " << blue << "   Green value is: " << green << "   Red value is: " << red << std::endl;
        image->at<cv::Vec3b>(i, j)[2] = 255;
      }
    }
  }
  return image;
}

std::shared_ptr<cv::Mat> SessionVisualizer::display_HSV_filter(std::shared_ptr<cv::Mat> image)
{
  cv::Mat hsv_image;
  cvtColor(*image, hsv_image, cv::COLOR_BGR2HSV);

  cv::Mat mask1;

  inRange(hsv_image, cv::Scalar(60,0,0), cv::Scalar(85, 255, 255), mask1);

  return std::make_shared<cv::Mat>(mask1);
}



void SessionVisualizer::display_image(shared_ptr<cv::Mat> image)
{
  cv::namedWindow(this->window_name->c_str(), cv::WINDOW_NORMAL);
  cv::imshow(this->window_name->c_str(), *image);
  cv::resizeWindow(this->window_name->c_str(), this->window_size->first, this->window_size->second );
  cv::moveWindow(this->window_name->c_str(), this->window_location->first, this->window_location->second);
  cv::waitKey(this->wait_time);
}

void SessionVisualizer::display_image_normalized(shared_ptr<cv::Mat> image, float lower_input, float upper_input,
                                                 float lower_output, float upper_output)
{
  cv::Mat input_image = *image;

  float shift_amount = lower_output - lower_input;
  float scale_amount = upper_output / (upper_input + shift_amount);

  cv::Mat shift_matrix = cv::Mat(input_image.size(), CV_32F, shift_amount);
  cv::Mat output_image = (input_image + shift_matrix) * scale_amount;

  cv::namedWindow(this->window_name->c_str(), cv::WINDOW_NORMAL);
  cv::imshow(this->window_name->c_str(), output_image);
  cv::resizeWindow(this->window_name->c_str(), this->window_size->first, this->window_size->second );
  cv::moveWindow(this->window_name->c_str(), this->window_location->first, this->window_location->second);
  cv::waitKey(this->wait_time);
}

void SessionVisualizer::display_depth_image(shared_ptr<cv::Mat> image)
{
    cv::namedWindow(this->window_name->c_str(), cv::WINDOW_NORMAL);

    cv::Mat newImg, finalImg;
    (*image).convertTo(newImg, CV_8UC1);
    applyColorMap(newImg, finalImg, cv::COLORMAP_RAINBOW);

    cv::imshow(this->window_name->c_str(), finalImg);
    cv::resizeWindow(this->window_name->c_str(), this->window_size->first, this->window_size->second );
    cv::moveWindow(this->window_name->c_str(), this->window_location->first, this->window_location->second);
    cv::waitKey(this->wait_time);
}
