//
// Created by nkaffine on 11/19/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality for the translated
 * point cloud which inherits from the local point cloud. The
 * underlying data is an already translated matrix and the
 * object has a reference to the translation matrix used
 * to translate the point cloud from the camera point cloud
 * to the current state.
 */
#ifndef FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_POINT_CLOUD_TRANSLATED_POINT_CLOUD_H_
#define FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_POINT_CLOUD_TRANSLATED_POINT_CLOUD_H_

#include <Fulfil.DepthCam/point_cloud/local_point_cloud.h>
#include <Fulfil.DepthCam/point_cloud/point_cloud.h>
#include <memory>
#include <librealsense2/rs.h>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
class TranslatedPointCloud :
    public LocalPointCloud,
    public std::enable_shared_from_this<TranslatedPointCloud>
{
 private:
  /**
     * The camera intrinsics for the color frames.
     */
  std::shared_ptr<rs2_intrinsics> color_intrinsics;

    std::shared_ptr<rs2_intrinsics> depth_intrinsics;
  /**
   * The extrinsics that contain the transormation from the
   * color stream point system to the depth stream point system.
   */
  std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics;
  /**
   * The extrinsics that contain the tranformation from the depth
   * stream point system to the color stream point system.
   */
  std::shared_ptr<rs2_extrinsics> depth_to_color_extrinsics;
  /**
   * The inner point cloud that is the result of applying the stored
   * transformation to the point cloud in the camera coordinate
   * system.
   */
  std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud;
  /**
   * The transformation that was used to convert the
   * camera point cloud into the transformed local
   * point cloud.
   */
  std::shared_ptr<Eigen::Affine3d> transformation;
 public:
  /**
   * TranslatedPointCloud Constructor
   * @param inner_point_cloud pointer to point cloud data that
   * has been translated from the camera coordinate system
   * with the provided transformation.
   * @param transformation pointer to the transformation that
   * was used to translate the camera point cloud.
   * @param color_intrinsics pointer to the intrinsics of the
   * color stream of the depth sensor the point cloud is from.
   * @param color_to_depth_extrinsics pointer to the extrinsics
   * that contain the transformation from the color stream to
   * the depth stream.
   * @param depth_to_color_exrtrinsics pointer to the extriniscs
   * that contain the transformation from the depth stream to
   * the color stream.
   */
  TranslatedPointCloud(std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud,
                       std::shared_ptr<Eigen::Affine3d> transformation,
                       std::shared_ptr<rs2_intrinsics> color_intrinsics,
                       std::shared_ptr<rs2_intrinsics> depth_intrinsics,
                       std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics,
                       std::shared_ptr<rs2_extrinsics> depth_to_color_exrtrinsics);

  std::shared_ptr<Eigen::Matrix3Xd> get_data() override;

  std::shared_ptr<LocalPointCloud> as_local_cloud() override;

  std::shared_ptr<CameraPointCloud> as_camera_cloud() override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PixelPointCloud> as_pixel_cloud() override;

  void apply_filter(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter) override;

  void apply_filter_side_dispense(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter);
  
  void apply_filter_side_dispense_point_cloud_outside_cavity(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter);

  void encode_to_directory(std::shared_ptr<std::string> filepath) override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> add_transformation(
      std::shared_ptr<Eigen::Affine3d> transformation) override;

  bool equal(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud) override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> new_point_cloud(
      std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud) override;

  void set_depth_value(int index, float depth) override;
};
}
}
}

#endif //FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_POINT_CLOUD_TRANSLATED_POINT_CLOUD_H_
