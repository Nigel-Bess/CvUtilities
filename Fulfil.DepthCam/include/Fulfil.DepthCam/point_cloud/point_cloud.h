//
// Created by nkaffine on 11/19/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to outline the required functionality
 * for point clouds in this library. This includes outlining
 * how to convert between different coordinate systems, encoding
 * the point cloud, adding additional transformations to point clouds,
 * and checking the equality of point clouds.
 */
#ifndef FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_POINT_CLOUD_H_
#define FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_POINT_CLOUD_H_

#include <memory>
#include <eigen3/Eigen/Geometry>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{
class LocalPointCloud;
class CameraPointCloud;
class PixelPointCloud;
class PointCloud
{
 public:
  /**
   * Returns a local point cloud from the current point cloud
   * @return a pointer to the local point cloud
   */
  virtual std::shared_ptr<LocalPointCloud> as_local_cloud() = 0;
  /**
   * Returns a camera point cloud from the current point cloud
   * @return a pointer to the camera point cloud
   */
  virtual std::shared_ptr<CameraPointCloud> as_camera_cloud() = 0;
  /**
   * Returns a point cloud in the pixel coordinate system from
   * the current point cloud.
   * @return a pointer to the pixel point cloud
   */
  virtual std::shared_ptr<PixelPointCloud> as_pixel_cloud() = 0;
  /**
   * Encodes the information for this point cloud to the directory at the given path.
   * @param directory_path the path to the directory where the data will be stored
   * (this directory should exists)
   */
  virtual void encode_to_directory(std::shared_ptr<std::string> directory_path) = 0;
  /**
   * Returns a point cloud with the given transformation added to it. The application of the new
   * transformation will be combined with any transformation currently used in the point cloud
   * and the combination will be treated as the local point cloud.
   * @param transformation the transformation that will be applied to the local coordinate system.
   * @return a pointer to a new point cloud with the added transformation.
   */
  virtual std::shared_ptr<PointCloud> add_transformation(std::shared_ptr<Eigen::Affine3d> transformation) = 0;
  /**
   * Returns whether the two point clouds are functionally equivalent (not if they are the same object).
   * @param point_cloud the point cloud that is being compared to this one.
   * @return bool for equality.
   */
  virtual bool equal(std::shared_ptr<PointCloud> point_cloud) = 0;

};
} // namespace pointcloud
} // namespace core
} // namespace fulfil

#endif //FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_POINT_CLOUD_H_
