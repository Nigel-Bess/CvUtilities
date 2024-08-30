//
// Created by nkaffine on 11/22/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality of the point cloud
 * that is in pixel coordinates. It contains a camera point
 * cloud and uses the extrinsics and intrinsics to convert
 * that data into pixels and pixels with depth data.
 */
#ifndef FULFIL_DEPTHCAM_SRC_POINT_CLOUD_DEPTH_PIXEL_POINT_CLOUD_H_
#define FULFIL_DEPTHCAM_SRC_POINT_CLOUD_DEPTH_PIXEL_POINT_CLOUD_H_

#include <Fulfil.DepthCam/point_cloud.h>
#include <librealsense2/rs.h>
#include <vector>
#include <opencv2/opencv.hpp>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
 class DepthPixelPointCloud : public fulfil::depthcam::pointcloud::PixelPointCloud,
     public std::enable_shared_from_this<DepthPixelPointCloud>
{
 private:
  /**
     * The depth sensor intrinsics for the color frames.
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
   * The camera point cloud that will be used to calculate
   * the pixel and pixel depth values.
   */
  std::shared_ptr<fulfil::depthcam::pointcloud::CameraPointCloud> camera_cloud;
  /**
   * Converts a 3d point in the camera's coordinate system to the
   * corresponding pixel.
   * @param point 3 dimensional eigen matrix column
   * @return a pointer to a OpenCV pixel that corresponds to the given point.
   */
  std::shared_ptr<cv::Point2f> camera_point_to_pixel(const fulfil::utils::eigen::Matrix3dPoint& point);
  /**
   * Converts a pixel with depth to the corresponding 3d camera coordinate.
   * @param pixel pointer to the opencv pixel coordinate.
   * @param depth the measured depth in the camera coordinate system.
   * @return a 3d vector representing the corresponding camera coordinate.
   */
  std::shared_ptr<Eigen::Vector3d> pixel_to_camera_point(std::shared_ptr<cv::Point2f> pixel, float depth);
 public:
  /**
   * DepthPixelPointCloud Constructor.
   * @param color_intrinsics pointer to the intrinsics for the color stream of
   * the depth sensor the point cloud is from.
   * @param color_to_depth_extrinsics pointer to the extrinsics containing
   * the transformation from the color stream to the depth stream of the depth
   * sensor the point cloud is from.
   * @param depth_to_color_exrtrinsics pointer to the extrinsics containing
   * the transformation from the depth stream to the color stream of the depth
   * sensor the point cloud is from.
   * @param camera_point_cloud pointer to the point cloud in camera coordinates
   * from the depth sensor.
   */
  DepthPixelPointCloud(std::shared_ptr<rs2_intrinsics> color_intrinsics,
                       std::shared_ptr<rs2_intrinsics> depth_intrinsics,
                       std::shared_ptr<rs2_extrinsics> color_to_depth_extrinsics,
                       std::shared_ptr<rs2_extrinsics> depth_to_color_exrtrinsics,
                       std::shared_ptr<CameraPointCloud> camera_point_cloud);

  std::shared_ptr<LocalPointCloud> as_local_cloud() override;

  std::shared_ptr<CameraPointCloud> as_camera_cloud() override;

  std::shared_ptr<PixelPointCloud> as_pixel_cloud() override;

  void encode_to_directory(std::shared_ptr<std::string> directory_path) override;

  std::shared_ptr<PointCloud> add_transformation(std::shared_ptr<Eigen::Affine3d> transformation) override;

  bool equal(std::shared_ptr<PointCloud> point_cloud) override;

  [[nodiscard]] cv::Size get_size_point_cloud_as_frame(float decimation_factor=8) const;

  std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> get_data() override;

  std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>>
  get_data_with_depth() override;

  std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> new_point_cloud(
      std::shared_ptr<std::vector<std::shared_ptr<std::pair<std::shared_ptr<cv::Point2f>, float>>>> cloud) override;
};
} // namespace fulfil
} // namespace depthcam
} // namespace pointcloud
#endif //FULFIL_DEPTHCAM_SRC_POINT_CLOUD_DEPTH_PIXEL_POINT_CLOUD_H_
