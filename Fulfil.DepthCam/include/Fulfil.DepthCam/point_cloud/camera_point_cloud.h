//
// Created by nkaffine on 11/19/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to outline functionality that is specific to
 * point clouds in the coordinate system of the underlying sensor. This includes
 * filtering the inner point cloud data and creating a new point cloud object
 * from a new raw point cloud.
 */
#ifndef FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_CAMERA_POINT_CLOUD_H_
#define FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_CAMERA_POINT_CLOUD_H_

#include <memory>
#include <eigen3/Eigen/Geometry>
#include <Fulfil.CPPUtils/eigen/matrix3xd_filter.h>
#include <Fulfil.DepthCam/point_cloud/point_cloud.h>

namespace fulfil
{
namespace depthcam
{
namespace pointcloud
{

class CameraPointCloud : virtual public PointCloud
{
 public:
  /**
   * Returns a pointer to the inner matrix data in the
   * coordinate system of the camera.
   * @return a point to the inner matrix data.
   * @note this function returns a pointer, not a copy,
   * any mutations to this pointer will effect the
   * inner pointcloud of this object.
   */
  virtual std::shared_ptr<Eigen::Matrix3Xd> get_data() = 0;
  /**
   * Applies the given filter to the inner eigen matrix.
   * @param filter the filter object in the camera coordinate system.
   * @note this will be applied when the data is in the coordinate
   * system is the camera coordinate system. It should use values
   * in that coordinate system in the filter.
   */
  virtual void apply_filter(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter) = 0;
  /**
   * Creates a new point cloud in the camera coordinate system but with the given
   * set of points in the camera coordinate system.
   * @param inner_point_cloud the underlying data in the camera coordinate system
   * that will be used to create the new point cloud.
   * @return a pointer to the newly created point cloud containing the new data.
   */
  virtual std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> new_point_cloud(
      std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud) = 0;
};
} // namespace pointcloud
} // namespace core
} // namespace fulfil

#endif //FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_CAMERA_POINT_CLOUD_H_
