//
// Created by nkaffine on 11/19/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of the untranslated point
 * cloud. This class represents a point cloud that has a translation
 * matrix associated with it but has not been translated.
 */
#include "untranslated_point_cloud.h"
#include "translated_point_cloud.h"
#include <Fulfil.CPPUtils/file_system_util.h>
#include "Fulfil.DepthCam/coders/matrix3xd_coder.h"
#include "Fulfil.DepthCam/coders/affine3d_coder.h"
#include <vector>
#include <iostream>
#include "Fulfil.DepthCam/coders/intrinsics_coder.h"
#include "Fulfil.DepthCam/coders/extrinsics_coder.h"
#include "depth_pixel_point_cloud.h"

using fulfil::depthcam::pointcloud::UntranslatedPointCloud;
using std::make_shared;
using fulfil::utils::FileSystemUtil;
using fulfil::depthcam::pointcloud::Matrix3xdCoder;
using fulfil::depthcam::pointcloud::Affine3DCoder;
using fulfil::depthcam::pointcloud::DepthPixelPointCloud;

UntranslatedPointCloud::UntranslatedPointCloud(std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud,
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

std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> UntranslatedPointCloud::as_local_cloud()
{
  return make_shared<TranslatedPointCloud>(
      make_shared<Eigen::Matrix3Xd>((*this->transformation) * (*this->inner_point_cloud)),
      this->transformation, this->color_intrinsics, this->depth_intrinsics,
      this->color_to_depth_extrinsics, this->depth_to_color_extrinsics);
}

std::shared_ptr<fulfil::depthcam::pointcloud::CameraPointCloud> UntranslatedPointCloud::as_camera_cloud()
{
  return this->shared_from_this();
}

std::shared_ptr<Eigen::Matrix3Xd> UntranslatedPointCloud::get_data()
{
  return this->inner_point_cloud;
}

void UntranslatedPointCloud::apply_filter(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter)
{
  this->inner_point_cloud = filter->filter(this->inner_point_cloud);
}

void UntranslatedPointCloud::encode_to_directory(std::shared_ptr<std::string> filepath)
{

  std::shared_ptr<std::string> point_cloud_filepath = make_shared<std::string>();
  point_cloud_filepath->append(*filepath);
  point_cloud_filepath->append("/point_cloud");
  Matrix3xdCoder::encode_to_file(this->inner_point_cloud, point_cloud_filepath);

  std::shared_ptr<std::string> rotation_filepath = make_shared<std::string>();
  rotation_filepath->append(*filepath);
  rotation_filepath->append("/transformation");
  Affine3DCoder::encode_to_file(this->transformation, rotation_filepath);

  std::shared_ptr<std::string> color_intrinsics_filepath = std::make_shared<std::string>();
  color_intrinsics_filepath->append(*filepath);
  color_intrinsics_filepath->append("/color_intrinsics");
  IntrinsicsCoder::encode_to_file(this->color_intrinsics, color_intrinsics_filepath);

  std::shared_ptr<std::string> depth_intrinsics_filepath = std::make_shared<std::string>();
  depth_intrinsics_filepath->append(*filepath);
  depth_intrinsics_filepath->append("/depth_intrinsics");
  IntrinsicsCoder::encode_to_file(this->depth_intrinsics, depth_intrinsics_filepath);

  std::shared_ptr<std::string> color_to_depth_extrinsics_filepath = std::make_shared<std::string>();
  color_to_depth_extrinsics_filepath->append(*filepath);
  color_to_depth_extrinsics_filepath->append("/color_to_depth_extrinsics");
  ExtrinsicsCoder::encode_to_file(this->color_to_depth_extrinsics, color_to_depth_extrinsics_filepath);

  std::shared_ptr<std::string> depth_to_color_extrinsics_filepath = std::make_shared<std::string>();
  depth_to_color_extrinsics_filepath->append(*filepath);
  depth_to_color_extrinsics_filepath->append("/depth_to_color_extrinsics");
  ExtrinsicsCoder::encode_to_file(this->depth_to_color_extrinsics, depth_to_color_extrinsics_filepath);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> UntranslatedPointCloud::add_transformation(std::shared_ptr<
    Eigen::Affine3d> transformation)
{
  std::shared_ptr<Eigen::Affine3d> new_transform = make_shared<Eigen::Affine3d>((*transformation) * (*this->transformation));
  return std::make_shared<UntranslatedPointCloud>(this->inner_point_cloud, new_transform, this->color_intrinsics, this->depth_intrinsics,
      this->color_to_depth_extrinsics, this->depth_to_color_extrinsics);
}

bool UntranslatedPointCloud::equal(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud)
{
  std::shared_ptr<Eigen::Matrix3Xd> camera_cloud_data = point_cloud->as_camera_cloud()->get_data();
  if(camera_cloud_data->cols() != this->inner_point_cloud->cols())
  {
    return false;
  }
  std::shared_ptr<Eigen::Matrix3Xd> local_cloud_data = point_cloud->as_local_cloud()->get_data();
  return (*this->inner_point_cloud) == (*camera_cloud_data)
    && ((*this->transformation) * (*this->inner_point_cloud)) == (*local_cloud_data);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> UntranslatedPointCloud::new_point_cloud(
    std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud)
{
  return make_shared<UntranslatedPointCloud>(inner_point_cloud, this->transformation,
      this->color_intrinsics, this->depth_intrinsics, this->color_to_depth_extrinsics, this->depth_to_color_extrinsics);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PixelPointCloud> UntranslatedPointCloud::as_pixel_cloud()
{
  return make_shared<DepthPixelPointCloud>(this->color_intrinsics, this->depth_intrinsics,
      this->color_to_depth_extrinsics, this->depth_to_color_extrinsics,
      this->shared_from_this());
}