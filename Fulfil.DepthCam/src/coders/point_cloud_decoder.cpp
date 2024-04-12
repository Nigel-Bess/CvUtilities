//
// Created by nkaffine on 11/20/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of decoding a point
 * cloud from a directory.
 */
#include "Fulfil.DepthCam/point_cloud/point_cloud_decoder.h"
#include "../point_cloud/untranslated_point_cloud.h"
#include "../point_cloud/no_translation_point_cloud.h"
#include "Fulfil.DepthCam/coders/affine3d_coder.h"
#include "Fulfil.DepthCam/coders/matrix3xd_coder.h"
#include "Fulfil.DepthCam/coders/intrinsics_coder.h"
#include <Fulfil.CPPUtils/file_system_util.h>
#include <vector>
#include "Fulfil.DepthCam/coders/extrinsics_coder.h"
#include <iostream>

using fulfil::depthcam::pointcloud::PointCloudDecoder;
using fulfil::utils::FileSystemUtil;
using fulfil::depthcam::pointcloud::Matrix3xdCoder;
using fulfil::depthcam::pointcloud::NoTranslationPointCloud;
using fulfil::depthcam::pointcloud::Affine3DCoder;
using std::make_shared;

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> PointCloudDecoder::decode_no_translation_point_cloud(
    std::shared_ptr<std::string> directory_path)
{
  std::shared_ptr<std::string> point_cloud_filename = std::make_shared<std::string>();
  point_cloud_filename->append(*directory_path);
  point_cloud_filename->append("/point_cloud");
  std::shared_ptr<Eigen::Matrix3Xd> mat = Matrix3xdCoder::decode_from_file(point_cloud_filename);

  std::shared_ptr<std::string> color_intrinsics_filename = std::make_shared<std::string>();
  color_intrinsics_filename->append(*directory_path);
  color_intrinsics_filename->append("/color_intrinsics");
  std::shared_ptr<rs2_intrinsics> color_stream_intrinsics = IntrinsicsCoder::decode_from_file(color_intrinsics_filename);

  std::shared_ptr<std::string> depth_intrinsics_filename = std::make_shared<std::string>();
  depth_intrinsics_filename->append(*directory_path);
  depth_intrinsics_filename->append("/depth_intrinsics");
  std::shared_ptr<rs2_intrinsics> depth_stream_intrinsics = IntrinsicsCoder::decode_from_file(depth_intrinsics_filename);

  std::shared_ptr<std::string> color_to_depth_extrinsics_filename = std::make_shared<std::string>();
  color_to_depth_extrinsics_filename->append(*directory_path);
  color_to_depth_extrinsics_filename->append("/color_to_depth_extrinsics");
  std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics = ExtrinsicsCoder::decode_from_file(color_to_depth_extrinsics_filename);

  std::shared_ptr<std::string> depth_to_color_extrinsics_filename = std::make_shared<std::string>();
  depth_to_color_extrinsics_filename->append(*directory_path);
  depth_to_color_extrinsics_filename->append("/depth_to_color_extrinsics");
  std::shared_ptr<rs2_extrinsics> depth_to_color_extrinsics = ExtrinsicsCoder::decode_from_file(depth_to_color_extrinsics_filename);

  return make_shared<NoTranslationPointCloud>(mat, color_stream_intrinsics, depth_stream_intrinsics,
      color_to_depth_extrinsics, depth_to_color_extrinsics);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> PointCloudDecoder::decode_untranslated_point_cloud(
    std::shared_ptr<std::string> directory_path)
{
  std::shared_ptr<std::string> point_cloud_filename = std::make_shared<std::string>();
  point_cloud_filename->append(*directory_path);
  point_cloud_filename->append("/point_cloud");
  std::shared_ptr<Eigen::Matrix3Xd> point_cloud = Matrix3xdCoder::decode_from_file(point_cloud_filename);

  std::shared_ptr<std::string> transformation_filename = std::make_shared<std::string>();
  transformation_filename->append(*directory_path);
  transformation_filename->append("/transformation");
  std::shared_ptr<Eigen::Affine3d> transformation = Affine3DCoder::decode_from_file(transformation_filename);

  std::shared_ptr<std::string> color_intrinsics_filename = std::make_shared<std::string>();
  color_intrinsics_filename->append(*directory_path);
  color_intrinsics_filename->append("/color_intrinsics");
  std::shared_ptr<rs2_intrinsics> color_stream_intrinsics = IntrinsicsCoder::decode_from_file(color_intrinsics_filename);

  std::shared_ptr<std::string> depth_intrinsics_filename = std::make_shared<std::string>();
  depth_intrinsics_filename->append(*directory_path);
  depth_intrinsics_filename->append("/depth_intrinsics");
  std::shared_ptr<rs2_intrinsics> depth_stream_intrinsics = IntrinsicsCoder::decode_from_file(depth_intrinsics_filename);

  std::shared_ptr<std::string> color_to_depth_extrinsics_filename = std::make_shared<std::string>();
  color_to_depth_extrinsics_filename->append(*directory_path);
  color_to_depth_extrinsics_filename->append("/color_to_depth_extrinsics");
  std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics = ExtrinsicsCoder::decode_from_file(color_to_depth_extrinsics_filename);

  std::shared_ptr<std::string> depth_to_color_extrinsics_filename = std::make_shared<std::string>();
  depth_to_color_extrinsics_filename->append(*directory_path);
  depth_to_color_extrinsics_filename->append("/depth_to_color_extrinsics");
  std::shared_ptr<rs2_extrinsics> depth_to_color_extrinsics = ExtrinsicsCoder::decode_from_file(depth_to_color_extrinsics_filename);

  return make_shared<UntranslatedPointCloud>(point_cloud, transformation, color_stream_intrinsics, depth_stream_intrinsics,
      color_to_depth_extrinsics, depth_to_color_extrinsics);
}

std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> PointCloudDecoder::decode_from_directory(
    std::shared_ptr<std::string> directory_path)
{
  return decode_no_translation_point_cloud(directory_path);
}
  /** previous version of this code would use saved transforms to get the point cloud. Broke when we changed LFB tag arrangement.
  if (flags->at(0))
  {
    return decode_untranslated_point_cloud(directory_path);
  }
  else
  {
    return decode_no_translation_point_cloud(directory_path);
  }

  **/