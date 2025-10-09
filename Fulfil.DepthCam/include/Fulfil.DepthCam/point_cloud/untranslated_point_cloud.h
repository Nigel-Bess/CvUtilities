//
// Created by nkaffine on 11/19/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality for an untranslated point cloud.
 * The untranslated point cloud inherits from the CameraPointCloud and the
 * underlying data is in the camera coordinate system. It also contains the
 * transformation to use to convert the data into the coordinate system
 * of the local cloud.
 */
#ifndef FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_POINT_CLOUD_UNTRANSLATED_POINT_CLOUD_H_
#define FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_POINT_CLOUD_UNTRANSLATED_POINT_CLOUD_H_

#include <memory>
#include <eigen3/Eigen/Geometry>
#include <librealsense2/rs.h>

#include <Fulfil.DepthCam/point_cloud/point_cloud.h>
#include <Fulfil.DepthCam/point_cloud/camera_point_cloud.h>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
class UntranslatedPointCloud :
    public CameraPointCloud,
    public std::enable_shared_from_this<UntranslatedPointCloud>
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
   * The point cloud data that is in the camera coordinate system.
   */
  std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud;
  /**
   * The transformation that will be used to convert from the
   * camera coordinate system to the local coordinate system.
   */
  std::shared_ptr<Eigen::Affine3d> transformation;
 public:
  /**
   * UntranslatedPointCloud
   * @param inner_point_cloud pointer to the point cloud data
   * that is in the camera coordinate system.
   * @param transformation pointer to the transformation that
   * will be used to convert the camera coordinate system to
   * the local coordinate system.
   * @param color_intrinsics pointer to the intrinsics for the
   * color stream of the depth sensor the point cloud is from
   * @param color_to_depth_extrinsics pointer to the extrinsics
   * that contains the transformation from the color stream
   * to the depth stream of the depth sensor.
   * @param depth_to_color_exrtrinsics pointer to the extriniscs
   * that contains the transformation from the depth stream
   * to the color stream of the depth sensor.
   */
  UntranslatedPointCloud(std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud,
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
};
} // namespace pointcloud
} // namespace core
} // namespace fulfil

#endif //FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_POINT_CLOUD_UNTRANSLATED_POINT_CLOUD_H_
