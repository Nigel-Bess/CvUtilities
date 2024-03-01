//
// Created by nkaffine on 11/22/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of the depth pixel
 * point cloud which is an implementaion fo the abstract class
 * PixelPointCloud. It has an inner camera point cloud and using
 * the extrinsics and intrinsics to convert that into depth pixel
 * data.
 */
#include "depth_pixel_point_cloud.h"
#include <librealsense2/rsutil.h>
#include <Fulfil.CPPUtils/eigen/matrix3xd_builder.h>

using fulfil::depthcam::pointcloud::DepthPixelPointCloud;
using fulfil::depthcam::pointcloud::CameraPointCloud;
using fulfil::depthcam::pointcloud::LocalPointCloud;
using fulfil::depthcam::pointcloud::PixelPointCloud;
using fulfil::depthcam::pointcloud::PointCloud;
using fulfil::utils::eigen::Matrix3XdBuilder;

DepthPixelPointCloud::DepthPixelPointCloud(std::shared_ptr<rs2_intrinsics> color_intrinsics,
                                           std::shared_ptr<rs2_intrinsics> depth_intrinsics,
                                           std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics,
                                           std::shared_ptr<rs2_extrinsics> depth_to_color_exrtrinsics,
                                           std::shared_ptr<CameraPointCloud> camera_point_cloud)
{
  this->color_intrinsics = color_intrinsics;
  this->depth_intrinsics = depth_intrinsics;
  this->color_to_depth_extrinsics = color_to_depth_extrinsics;
  this->depth_to_color_extrinsics = depth_to_color_exrtrinsics;
  this->camera_cloud = camera_point_cloud; // by passing this in as a shared pointer, we may have it change from underneath us
}

std::shared_ptr<CameraPointCloud> DepthPixelPointCloud::as_camera_cloud()
{
  return this->camera_cloud;
}

std::shared_ptr<LocalPointCloud> DepthPixelPointCloud::as_local_cloud()
{
  return this->camera_cloud->as_local_cloud();
}

std::shared_ptr<PixelPointCloud> DepthPixelPointCloud::as_pixel_cloud()
{
  return this->shared_from_this();
}

void DepthPixelPointCloud::encode_to_directory(std::shared_ptr<std::string> directory_path)
{
  this->camera_cloud->encode_to_directory(directory_path);
}
std::shared_ptr<PointCloud> DepthPixelPointCloud::add_transformation(std::shared_ptr<Eigen::Affine3d> transformation)
{
  return this->camera_cloud->add_transformation(transformation);
}

bool DepthPixelPointCloud::equal(std::shared_ptr<PointCloud> point_cloud)
{
  return this->camera_cloud->equal(point_cloud);
}

std::shared_ptr<cv::Point2f> DepthPixelPointCloud::camera_point_to_pixel(const fulfil::utils::eigen::Matrix3dPoint &point)
{
  float color_point[3];
  float depth_point[3] = {(float)point(0),(float)point(1),(float)point(2)};
  rs2_transform_point_to_point(color_point, this->depth_to_color_extrinsics.get(), depth_point);

  //Converting from color point to color pixel
  float pixel[2];
  std::shared_ptr<rs2_intrinsics> intrinsics = this->color_intrinsics;
  rs2_project_point_to_pixel(pixel, intrinsics.get(), color_point);
  return std::make_shared<cv::Point2f>(pixel[0], pixel[1]);
}

std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> DepthPixelPointCloud::get_data()
{
  std::shared_ptr<Eigen::Matrix3Xd> camera_cloud_data = this->camera_cloud->get_data();
  std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> pixels =
      std::make_shared<std::vector<std::shared_ptr<cv::Point2f>>>();
  for(int i = 0; i < camera_cloud_data->cols(); i++)
  {
    pixels->push_back(this->camera_point_to_pixel(camera_cloud_data->col(i)));
  }
  return pixels;
}

std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>> DepthPixelPointCloud::get_data_with_depth()
{
  std::shared_ptr<Eigen::Matrix3Xd> camera_cloud_data = this->camera_cloud->get_data();
  std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>>
  pixels = std::make_shared<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>>();
  for(int i = 0; i < camera_cloud_data->cols(); i++)
  {
    pixels->push_back(
        std::make_shared<std::pair<std::shared_ptr<cv::Point2f>, float>>(
            this->camera_point_to_pixel(camera_cloud_data->col(i)),
            (*camera_cloud_data)(2,i)));
  }
  return pixels;
}

std::shared_ptr<Eigen::Vector3d> DepthPixelPointCloud::pixel_to_camera_point(std::shared_ptr<cv::Point2f> pixel,
                                                                             float depth)
{
  //Converting color pixel to color point
  float color_point[3];
  const float raw_pixel[2] = {pixel->x, pixel->y};
  rs2_deproject_pixel_to_point(color_point, this->color_intrinsics.get(), raw_pixel, depth);

  //Converting color point to depth point
  float depth_point[3];
  rs2_transform_point_to_point(depth_point, this->color_to_depth_extrinsics.get(), color_point);

  //Return the new point
  return std::make_shared<Eigen::Vector3d>(depth_point[0], depth_point[1], depth_point[2]);
}

// TODO @Amber THE HARD CODING OF SENSOR OPTIONS MUST BE FIXED!!! Decimation factor should come
// from that object directly using something like this:
//    float decimation_factor = filter.get_option(RS2_OPTION_FILTER_MAGNITUDE);
// Currently passing in as a float because that is the expected type from filter.get_option (which is where we should
// get this info from in the future
cv::Size DepthPixelPointCloud::get_size_point_cloud_as_frame(float decimation_factor) const{

  // account for padding in decimated frame calc
  auto decimated_dimension = [](uint16_t original_dimension, uint16_t dec_mag) {
    uint16_t decimated_dim = (original_dimension / dec_mag) + 3;
    decimated_dim /= 4;
    decimated_dim *= 4;
    return decimated_dim;
  };

  return {decimated_dimension(this->depth_intrinsics->width, decimation_factor),
      decimated_dimension(this->depth_intrinsics->height, decimation_factor)};
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> DepthPixelPointCloud::new_point_cloud(std::shared_ptr<std::vector<
    std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>> cloud)
{
  std::shared_ptr<Matrix3XdBuilder> builder = std::make_shared<Matrix3XdBuilder>();
  for(int i = 0; i < cloud->size(); i++)
  {
    std::shared_ptr<Eigen::Vector3d> vector = this->pixel_to_camera_point(cloud->at(i)->first, cloud->at(i)->second);
    builder->add_row((*vector)(0), (*vector)(1), (*vector)(2));
  }
  return this->camera_cloud->new_point_cloud(builder->get_matrix());
}

