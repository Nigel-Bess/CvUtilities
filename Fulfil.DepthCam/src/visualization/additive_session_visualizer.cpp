//
// Created by amber on 8/25/20.
//

#include "Fulfil.DepthCam/visualization/additive_session_visualizer.h"
#include <opencv2/opencv.hpp>
#include <librealsense2/rsutil.h>
#include <opencv2/aruco.hpp>
#include "colored_depth_display.h"
#include <Fulfil.DepthCam/point_cloud.h>
#include <Fulfil.DepthCam/aruco.h>
#include "Fulfil.DepthCam/aruco/marker.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <Fulfil.DepthCam/frame/filtering.h>
#include <stdio.h>

using fulfil::depthcam::visualization::AdditiveSessionVisualizer;
using std::shared_ptr;
using std::string;
using std::make_shared;
using fulfil::depthcam::aruco::Marker;
using fulfil::depthcam::pointcloud::LocalPointCloud;
namespace filterUtils = fulfil::depthcam::filtering;



void AdditiveSessionVisualizer::clear_image_state(bool rgb_base)
{
    if (rgb_base){
      this->session->get_color_mat()->copyTo(this->current_image);
    } else {
      this->current_image = cv::Mat(session->get_color_mat()->size(), CV_8UC3, cv::Scalar(0, 0, 0));
    }

}


void AdditiveSessionVisualizer::map_rgb_to_pointcloud(shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud)
{

  auto pixels = point_cloud->as_pixel_cloud()->get_data();
  auto rgb_image = session->get_color_mat();
    for(int i = 0; i < pixels->size(); i++)
    {
        this->current_image.at<cv::Vec3b>(*pixels->at(i)) = rgb_image->at<cv::Vec3b>(*pixels->at(i));
    }

}


void AdditiveSessionVisualizer::add_pixels(std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> pixels,
                                       int red,
                                       int green,
                                       int blue,
                                       int radius,
                                       int thickness)
{

    for(int i = 0; i < pixels->size(); i++)
    {
        cv::circle(this->current_image, *pixels->at(i), radius, cv::Scalar(blue,green,red), thickness);
    }
}

void AdditiveSessionVisualizer::add_circle(cv::Point pixel,
                                           int red,
                                           int green,
                                           int blue,
                                           int radius,
                                           int thickness)
{
    cv::circle(this->current_image, pixel, radius, cv::Scalar(blue,green,red), thickness);
}

void AdditiveSessionVisualizer::add_crosshair(cv::Point pixel,
    int red,
    int green,
    int blue,
    int radius,
    int thickness)
{
  radius += 3;
  cv::line(this->current_image, {pixel.x-radius, pixel.y}, {pixel.x+radius, pixel.y}, cv::Scalar(blue,green,red), thickness);
  cv::line(this->current_image, {pixel.x, pixel.y-radius}, {pixel.x, pixel.y+radius}, cv::Scalar(blue,green,red), thickness);
  cv::circle(this->current_image, pixel, (radius)/2, cv::Scalar(blue,green,red), (thickness+1)/2);

}

AdditiveSessionVisualizer::AdditiveSessionVisualizer(shared_ptr<Session> session, shared_ptr<std::string> window_name,
                                                     std::pair<int, int> window_location, std::pair<int, int> window_size,
                                                     int wait_time, bool rgb_base)
{
    this->session = session;
    this->window_name = window_name;
    this->window_location = window_location;
    this->window_size = window_size;
    this->wait_time = wait_time;

    //TODO Necessary??? 
    this->current_image = cv::Mat();
    this->clear_image_state(rgb_base);
}

//prints out the pixel of the image when double-clicked
void AdditiveSessionVisualizer::onMouse(int event, int x, int y, int flags, void* param)
{
    if (event == cv::EVENT_LBUTTONDBLCLK)
    {
        std::cout << "x= " << x << " y= " << y << std::endl;
    }
}


void AdditiveSessionVisualizer::display_underlying_rgb_image()
{
    // // Display in a GUI
    cv::namedWindow(this->window_name->c_str(), cv::WINDOW_NORMAL);

    //allows for printing out pixel coordinate when image is double clicked
    cv::setMouseCallback(this->window_name->c_str(), onMouse, 0);

    cv::imshow(this->window_name->c_str(), *(session->get_color_mat()));
    cv::resizeWindow(this->window_name->c_str(), this->window_size.first, this->window_size.second );
    cv::moveWindow(this->window_name->c_str(), this->window_location.first, this->window_location.second);
    cv::waitKey(this->wait_time);
}

void AdditiveSessionVisualizer::add_point_cloud(bool should_include_invalid_points)
{
    this->add_pixels(this->session->get_point_cloud(should_include_invalid_points, __FUNCTION__)->as_pixel_cloud()->get_data(),
                         0,
                         0,
                         0,
                         0,
                         0);
}

void AdditiveSessionVisualizer::add_points(shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud, int red, int green,int blue,int radius,int thickness)
{
    this->add_pixels(point_cloud->as_pixel_cloud()->get_data(), red, green, blue, radius, thickness);
}

void AdditiveSessionVisualizer::add_rectangle(std::shared_ptr<fulfil::utils::Point3D> center,
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
    this->add_points(this->session->get_point_cloud(true, __FUNCTION__)->as_camera_cloud()
                                 ->new_point_cloud(point_cloud_data));
}

void AdditiveSessionVisualizer::add_rectangle(int x_pix_min, int y_pix_min, int x_pix_max, int y_pix_max)
{

  cv::rectangle(this->current_image, cv::Point2i(x_pix_min, y_pix_min), cv::Point2i(x_pix_max, y_pix_max), cv::Scalar(0,255,0), 2);
}

void AdditiveSessionVisualizer::add_perimeter(const std::vector<cv::Point2i> &vertices,
    const cv::Scalar& color, int line_thickness)
{
  const cv::Point* pts = &vertices[0];
  int num_vertices = static_cast<int>(vertices.size());
  cv::polylines(this->current_image, &pts, &num_vertices, 1, true, color, line_thickness);
}


void AdditiveSessionVisualizer::add_line(cv::Point2i start, cv::Point2i end){
  cv::line(this->current_image, start, end, cv::Scalar(228, 148,191), 2);
}

void AdditiveSessionVisualizer::add_detected_markers(cv::Ptr<cv::aruco::Dictionary> dictionary, std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::aruco::Marker>>> markers,
                                              int region_max_x,
                                              int region_min_x,
                                              int region_max_y,
                                              int region_min_y)
{
    this->session->get_color_mat()->copyTo(this->current_image);

    /**
     *  Detect and display all markers on image
     */
    std::vector<int> ids;
    std::vector<std::vector<cv::Point2f>> corners, rejected;
    cv::Ptr<cv::aruco::DetectorParameters> detectorParams = cv::aruco::DetectorParameters::create();
    cv::aruco::detectMarkers(this->current_image, dictionary, corners, ids, detectorParams, rejected);
    //cv::Mat outImage;
    //image->copyTo(outImage);
    cv::aruco::drawDetectedMarkers(this->current_image, corners, ids, cv::Scalar(100, 0, 255));

    /**
    *  Display valid search region on image
    */
    cv::Point2f point1 = cv::Point2f(region_min_x , region_min_y);
    cv::Point2f point2 = cv::Point2f(region_max_x , region_min_y);
    cv::Point2f point3 = cv::Point2f(region_min_x , region_max_y);
    cv::Point2f point4 = cv::Point2f(region_max_x , region_max_y);

    cv::line(this->current_image, point1, point2, cv::Scalar(0,255,0), 2);
    cv::line(this->current_image, point1, point3, cv::Scalar(0,255,0), 2);
    cv::line(this->current_image, point2, point4, cv::Scalar(0,255,0), 2);
    cv::line(this->current_image, point3, point4, cv::Scalar(0,255,0), 2);

    /**
    *  Mark valid markers with green circles
    */
    for(int i = 0; i < markers->size(); i++)
    {
        assert (markers->at(i) != nullptr); // include for dev purposes, can remove later

        std::shared_ptr<Marker> marker = markers->at(i);
        float marker_x = marker->get_coordinate(Marker::Coordinate::center)->x;
        float marker_y = marker->get_coordinate(Marker::Coordinate::center)->y;
        cv::circle(this->current_image, cv::Point2f(marker_x, marker_y), 25, cv::Scalar(0,255,0), 2);
    }
}

void AdditiveSessionVisualizer::add_points_with_depth_coloring(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud)
{
    auto depth_pixels = point_cloud->as_pixel_cloud()->get_data_with_depth();

    // Display in a GUI
    float max_dist = -1000;
    float min_dist = 1000;

    for(int i = 0; i < depth_pixels->size(); i++)
    {
        max_dist = std::max(max_dist, (float) depth_pixels->at(i)->second);
        min_dist = std::min(min_dist, (float) depth_pixels->at(i)->second);
    }
    std::shared_ptr<ColoredDepthDisplay> calc = std::make_shared<ColoredDepthDisplay>(min_dist, max_dist);
    for(int i = 0; i < depth_pixels->size(); i++)
    {
        cv::circle(this->current_image, *depth_pixels->at(i)->first, 1, calc->color_at_depth((float)(depth_pixels->at(i)->second)), 3);
    }
}


void AdditiveSessionVisualizer::add_points_with_local_depth_coloring(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud)
{
    auto depth_pixels = point_cloud->as_pixel_cloud()->get_data_with_depth();

    std::shared_ptr<LocalPointCloud> local_point_cloud = point_cloud->as_pixel_cloud()->new_point_cloud(depth_pixels)->as_local_cloud();
    auto local_depth_data = local_point_cloud->get_data();
    // Display in a GUI
    if (local_depth_data->cols() > 0) {
      auto max_dist = local_depth_data->row(2).maxCoeff();
      auto min_dist = local_depth_data->row(2).minCoeff();

      std::shared_ptr<ColoredDepthDisplay> calc = std::make_shared<ColoredDepthDisplay>(min_dist, max_dist);
      for(int i = 0; i < depth_pixels->size(); i++)
      {
        cv::circle(this->current_image, *depth_pixels->at(i)->first, 1, calc->color_at_depth((float)((*local_depth_data)(2,i))), 3);
      }
    }

}

void AdditiveSessionVisualizer::add_perimeter(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> perimeter,
                                                              int thickness = 2)
{
    shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> pixels = perimeter->as_pixel_cloud()->get_data();
    for(int i = 0; i < pixels->size(); i++)
    {
        cv::line(this->current_image, *pixels->at(i), *pixels->at((i+1)%(pixels->size())), cv::Scalar(0,255,0), thickness);
    }
}

void AdditiveSessionVisualizer::add_perimeters(std::shared_ptr<std::vector<std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud>>> perimeters)
{

    for(int i = 0; i < perimeters->size(); i++)
    {
        shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> pixels = perimeters->at(i)->as_pixel_cloud()->get_data();
        for(int i = 0; i < pixels->size(); i++)
        {
            cv::line(this->current_image, *pixels->at(i), *pixels->at((i+1)%(pixels->size())), cv::Scalar(0,0,255));
        }
    }
}



void AdditiveSessionVisualizer::display_image(shared_ptr<cv::Mat> image)
{
    cv::namedWindow(this->window_name->c_str(), cv::WINDOW_NORMAL);
    cv::imshow(this->window_name->c_str(), *image);
    cv::resizeWindow(this->window_name->c_str(), this->window_size.first, this->window_size.second );
    cv::moveWindow(this->window_name->c_str(), this->window_location.first, this->window_location.second);
    cv::waitKey(this->wait_time);
}

void AdditiveSessionVisualizer::display()
{
    cv::namedWindow(this->window_name->c_str(), cv::WINDOW_NORMAL);
    cv::imshow(this->window_name->c_str(), this->current_image);
    cv::resizeWindow(this->window_name->c_str(), this->window_size.first, this->window_size.second );
    cv::moveWindow(this->window_name->c_str(), this->window_location.first, this->window_location.second);
    cv::waitKey(this->wait_time);
}

void AdditiveSessionVisualizer::save(const std::string& path)
{
    if (!cv::imwrite(path, this->current_image))
        throw std::runtime_error("Issue saving AdditiveSessionViewer Image!");
}

AdditiveSessionVisualizer::AdditiveSessionVisualizer(cv::Mat init_image, std::shared_ptr<Session> session,
                                                     std::shared_ptr<std::string> window_name,
                                                     std::pair<int, int> window_location,
                                                     std::pair<int, int> window_size,
                                                     int wait_time, bool rgb_base)
{
  this->session = session;
  this->window_name = window_name;
  this->window_location = window_location;
  this->window_size = window_size;
  this->wait_time = wait_time;

  //TODO Necessary???
  this->current_image = init_image;
}

void AdditiveSessionVisualizer::add_base_image(const std::shared_ptr<cv::Mat>& new_image)
{
  this->current_image = cv::Mat(new_image->size(), new_image->type(), cv::Scalar(0, 0, 0));
  new_image->copyTo(this->current_image);

}

void AdditiveSessionVisualizer::apply_mask(const cv::Mat& mask){
  filterUtils::filter_out_selection(this->current_image, mask);

}

cv::Mat AdditiveSessionVisualizer::get_current_base_image_state() const { return this->current_image.clone(); }
