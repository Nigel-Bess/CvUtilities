//
// Created by nkaffine on 11/19/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of a point cloud
 * where the underlying data has already been transformed
 * by the associated translation matrix. This inherits
 * from the local point cloud because the data is
 * already translated.
 */
#include <iostream>
#include <Fulfil.DepthCam/point_cloud/untranslated_point_cloud.h>
#include <Fulfil.DepthCam/point_cloud/translated_point_cloud.h>
#include <Fulfil.DepthCam/point_cloud/depth_pixel_point_cloud.h>

using fulfil::depthcam::pointcloud::TranslatedPointCloud;
using fulfil::depthcam::pointcloud::UntranslatedPointCloud;
using std::make_shared;
using fulfil::depthcam::pointcloud::DepthPixelPointCloud;

TranslatedPointCloud::TranslatedPointCloud(std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud,
                                           std::shared_ptr<Eigen::Affine3d> transformation,
                                           std::shared_ptr<rs2_intrinsics> color_intrinsics,
                                           std::shared_ptr<rs2_intrinsics> depth_intrinsics,
                                           std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics,
                                           std::shared_ptr<rs2_extrinsics> depth_to_color_exrtrinsics)
{
  this->inner_point_cloud = inner_point_cloud;
  this->transformation = transformation;
  this->color_intrinsics = color_intrinsics;
  this->depth_intrinsics = depth_intrinsics;
  this->color_to_depth_extrinsics = color_to_depth_extrinsics;
  this->depth_to_color_extrinsics = depth_to_color_exrtrinsics;
}

std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> TranslatedPointCloud::as_local_cloud()
{
  return this->shared_from_this();
}

std::shared_ptr<fulfil::depthcam::pointcloud::CameraPointCloud> TranslatedPointCloud::as_camera_cloud()
{
  return make_shared<UntranslatedPointCloud>(
      make_shared<Eigen::Matrix3Xd>(this->transformation->inverse() * (*this->inner_point_cloud)),
      this->transformation, this->color_intrinsics, this->depth_intrinsics,
      this->color_to_depth_extrinsics, this->depth_to_color_extrinsics);
}

std::shared_ptr<Eigen::Matrix3Xd> TranslatedPointCloud::get_data()
{
  return this->inner_point_cloud;
}

void TranslatedPointCloud::apply_filter(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter)
{
  this->inner_point_cloud = filter->filter(inner_point_cloud);
}

void TranslatedPointCloud::apply_filter_side_dispense(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter)
{
    this->inner_point_cloud = filter->filter_side_dispense(inner_point_cloud);
}

void TranslatedPointCloud::apply_filter_side_dispense_point_cloud_outside_cavity(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter)
{
    std::shared_ptr<Eigen::Matrix3Xd> outer_point_cloud = filter->filter_side_dispense_point_cloud_outside_cavity(inner_point_cloud);
}

void TranslatedPointCloud::encode_to_directory(std::shared_ptr<std::string> filepath)
{
  this->as_camera_cloud()->encode_to_directory(filepath);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud>
    TranslatedPointCloud::add_transformation(std::shared_ptr<Eigen::Affine3d> transformation)
{
  std::shared_ptr<Eigen::Matrix3Xd> new_translated_cloud = std::make_shared<Eigen::Matrix3Xd>((*transformation) * (*this->inner_point_cloud));
  std::shared_ptr<Eigen::Affine3d> new_transformation = std::make_shared<Eigen::Affine3d>((*transformation) * (*this->transformation));
  return std::make_shared<TranslatedPointCloud>(new_translated_cloud, new_transformation,
      this->color_intrinsics, this->depth_intrinsics, this->color_to_depth_extrinsics, this->depth_to_color_extrinsics);
}

bool TranslatedPointCloud::equal(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud)
{
  std::shared_ptr<Eigen::Matrix3Xd> camera_cloud_data = point_cloud->as_camera_cloud()->get_data();
  if(this->inner_point_cloud->cols() != camera_cloud_data->cols())
  {
    return false;
  }
  std::shared_ptr<Eigen::Matrix3Xd> local_cloud_data = point_cloud->as_local_cloud()->get_data();
  return (*this->inner_point_cloud) == (*local_cloud_data)
    && (this->transformation->inverse() * (*this->inner_point_cloud)) == (*camera_cloud_data);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> TranslatedPointCloud::new_point_cloud(std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud)
{
  return make_shared<TranslatedPointCloud>(inner_point_cloud, this->transformation,
      this->color_intrinsics, this->depth_intrinsics, this->color_to_depth_extrinsics,
      this->depth_to_color_extrinsics);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PixelPointCloud> TranslatedPointCloud::as_pixel_cloud()
{
  return make_shared<DepthPixelPointCloud>(this->color_intrinsics, this->depth_intrinsics,
      this->color_to_depth_extrinsics, this->depth_to_color_extrinsics,
      this->as_camera_cloud());
}

void TranslatedPointCloud::set_depth_value(int index, float depth)
{
  std::shared_ptr<Eigen::Matrix3Xd> point_cloud = this->inner_point_cloud;
  //std::cout << (*point_cloud)(2, index) << std::endl;
  (*point_cloud)(2, index) = depth;
  //std::cout << (*point_cloud)(2, index) << std::endl; ;
}