//
// Created by nkaffine on 11/19/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality of a point cloud that
 * has no translation associated with it. It implements both
 * the local point cloud and the camera point cloud classes
 * because the local point cloud and the camera point cloud
 * are the same.
 */
#ifndef FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_DEPTH_SESSION_POINT_CLOUD_H_
#define FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_DEPTH_SESSION_POINT_CLOUD_H_

#include <Fulfil.DepthCam/point_cloud/point_cloud.h>
#include <Fulfil.DepthCam/point_cloud/camera_point_cloud.h>
#include <Fulfil.DepthCam/point_cloud/local_point_cloud.h>
#include <librealsense2/rs.h>


namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
class NoTranslationPointCloud
    : public CameraPointCloud,
      public LocalPointCloud,
      public std::enable_shared_from_this<NoTranslationPointCloud>
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
   * The inner point cloud data that is from the depth sensor that
   * is both the camera and local coordinate system because they
   * are the same.
   */
  std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud;
 public:
  /**
   * NoTranslationPointCloud Constructor
   * @param inner_point_cloud pointer to raw point cloud data from the sensor.
   * @param color_intrinsics pointer to the intrinsics of the color stream
   * of the depth sensor.
   * @param color_to_depth_extrinsics pointer to the extrinsics that contain
   * the transformation from the color stream to the depth stream.
   * @param depth_to_color_exrtrinsics pointer to the extrinsics that contain
   * the transformation from the depth stream to the color stream.
   */
  NoTranslationPointCloud(std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud,
                          std::shared_ptr<rs2_intrinsics> color_intrinsics,
                          std::shared_ptr<rs2_intrinsics> depth_intrinsics,
                          std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics,
                          std::shared_ptr<rs2_extrinsics> depth_to_color_exrtrinsics);

  std::shared_ptr<Eigen::Matrix3Xd> get_data() override;

  std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> as_local_cloud() override;

  std::shared_ptr<fulfil::depthcam::pointcloud::CameraPointCloud> as_camera_cloud() override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PixelPointCloud> as_pixel_cloud() override;

  void apply_filter(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter) override;

  void encode_to_directory(std::shared_ptr<std::string> filepath) override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> add_transformation(
      std::shared_ptr<Eigen::Affine3d> transformation) override;

  bool equal(std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> point_cloud) override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> new_point_cloud(
      std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud) override;

  void set_depth_value(int index, float depth) override;
};
} // namespace pointcloud
} // namespace core
} // namespace pointcloud

#endif //FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_DEPTH_SESSION_POINT_CLOUD_H_
