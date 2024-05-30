//
// Created by nkaffine on 11/19/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of a point cloud that
 * doesn't have any transformation. It can be treated as both
 * a local point cloud and a camera point cloud because
 * they are the same data.
 */
#include "no_translation_point_cloud.h"
#include "Fulfil.DepthCam/coders/matrix3xd_coder.h"
#include <Fulfil.CPPUtils/file_system_util.h>
#include "untranslated_point_cloud.h"
#include <vector>
#include <eigen3/Eigen/Geometry>
#include "Fulfil.DepthCam/coders/intrinsics_coder.h"
#include "Fulfil.DepthCam/coders/extrinsics_coder.h"
#include "depth_pixel_point_cloud.h"

using fulfil::depthcam::pointcloud::NoTranslationPointCloud;
using fulfil::depthcam::pointcloud::Matrix3xdCoder;
using fulfil::utils::FileSystemUtil;
using fulfil::depthcam::pointcloud::UntranslatedPointCloud;
using fulfil::depthcam::pointcloud::DepthPixelPointCloud;

NoTranslationPointCloud::NoTranslationPointCloud(std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud,
                                                 std::shared_ptr<rs2_intrinsics> color_intrinsics,
                                                 std::shared_ptr<rs2_intrinsics> depth_intrinsics,
                                                 std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics,
                                                 std::shared_ptr<rs2_extrinsics> depth_to_color_exrtrinsics)
{
  this->inner_point_cloud = inner_point_cloud;
  this->color_intrinsics = color_intrinsics;
  this->depth_intrinsics = depth_intrinsics;
  this->color_to_depth_extrinsics = color_to_depth_extrinsics;
  this->depth_to_color_extrinsics = depth_to_color_exrtrinsics;
}

std::shared_ptr<fulfil::depthcam::pointcloud::CameraPointCloud> NoTranslationPointCloud::as_camera_cloud()
{
  return this->shared_from_this();
}

std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> NoTranslationPointCloud::as_local_cloud()
{
  return this->shared_from_this();
}

std::shared_ptr<Eigen::Matrix3Xd> NoTranslationPointCloud::get_data()
{
  return this->inner_point_cloud;
}

void NoTranslationPointCloud::apply_filter(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter)
{
  this->inner_point_cloud = filter->filter(this->inner_point_cloud);
}

void NoTranslationPointCloud::encode_to_directory(std::shared_ptr<std::string> filepath)
{

  std::shared_ptr<std::string> point_cloud_filepath = std::make_shared<std::string>();
  point_cloud_filepath->append(*filepath);
  point_cloud_filepath->append("/point_cloud");
  Matrix3xdCoder::encode_to_file(this->inner_point_cloud, point_cloud_filepath);

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

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> NoTranslationPointCloud::add_transformation(
    std::shared_ptr<Eigen::Affine3d> transformation)
{
  return std::make_shared<UntranslatedPointCloud>(this->inner_point_cloud, transformation,
      this->color_intrinsics, this->depth_intrinsics, this->color_to_depth_extrinsics,
      this->depth_to_color_extrinsics);
}

bool NoTranslationPointCloud::equal(std::shared_ptr<PointCloud> point_cloud)
{
  std::shared_ptr<Eigen::Matrix3Xd> camera_cloud_data = point_cloud->as_camera_cloud()->get_data();
  if(camera_cloud_data->cols() != this->inner_point_cloud->cols())
  {
    return false;
  }
  std::shared_ptr<LocalPointCloud> local_cloud = point_cloud->as_local_cloud();
  std::shared_ptr<Eigen::Matrix3Xd> local_cloud_data = local_cloud->get_data();
  return (*inner_point_cloud) == (*local_cloud_data) && (*inner_point_cloud) == (*camera_cloud_data);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> NoTranslationPointCloud::new_point_cloud(std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud)
{
  return std::make_shared<NoTranslationPointCloud>(inner_point_cloud, this->color_intrinsics, this->depth_intrinsics,
      this->color_to_depth_extrinsics, this->depth_to_color_extrinsics);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PixelPointCloud> NoTranslationPointCloud::as_pixel_cloud()
{
  return std::make_shared<DepthPixelPointCloud>(this->color_intrinsics, this->depth_intrinsics,
      this->color_to_depth_extrinsics, this->depth_to_color_extrinsics,
      this->shared_from_this());
}


void NoTranslationPointCloud::set_depth_value(int index, float depth)
{
  std::shared_ptr<Eigen::Matrix3Xd> point_cloud = this->inner_point_cloud;
  //std::cout << (*point_cloud)(2, index);
  //*point_cloud(2, index) = depth;
}