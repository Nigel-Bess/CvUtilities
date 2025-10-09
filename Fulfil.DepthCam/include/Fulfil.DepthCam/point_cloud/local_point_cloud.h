//
// Created by nkaffine on 11/19/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is to outline the functionality specific to
 * a point cloud that represents the coordinate system after applying
 * any translations stored in the point cloud object.
 */
#ifndef FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_LOCAL_POINT_CLOUD_H_
#define FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_LOCAL_POINT_CLOUD_H_

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
class LocalPointCloud : virtual public PointCloud
{
 public:
  /**
   * Returns a pointer to the inner matrix data in
   * the local coordinate system
   * @return a pointer to the inner matrix data in
   * the local coordinate system.
   * @note this function returns a point to the inner
   * data, not a copy. If any mutation is performed on the pointer
   * it will effect the underlying data as well.
   */
  virtual std::shared_ptr<Eigen::Matrix3Xd> get_data() = 0;
  /**
   * Applies the given filter to the inner eigen matrix.
   * @param filter that determines which points are kept. The filter
   * should use values that are in the local coordinate system.
   */
  virtual void apply_filter(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter) = 0;

  virtual void apply_filter_side_dispense(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter) = 0;

  virtual void apply_filter_side_dispense_point_cloud_outside_cavity(std::shared_ptr<fulfil::utils::eigen::Matrix3XdFilter> filter) = 0;
  /**
   * Creates a new point cloud in the local coordinate system but with the given set of
   * points in the local coordinate system.
   * @param inner_point_cloud the points that are in the local coordinate system that will
   * be the inner data of the new local point cloud.
   * @return a pointer to the new point cloud.
   */
  virtual std::shared_ptr<fulfil::depthcam::pointcloud::PointCloud> new_point_cloud(
      std::shared_ptr<Eigen::Matrix3Xd> inner_point_cloud) = 0;

  /**
   * This function allows for changing the depth value at the given point cloud index to the given depth input value
   */
  virtual void set_depth_value(int index, float depth) = 0;
};
} // namespace pointcloud
} // namespace core
} // namespace fulfil

#endif //FULFIL_DEPTHCAM_INCLUDE_FULFIL_DEPTHCAM_DEPTHCAM_LOCAL_POINT_CLOUD_H_
