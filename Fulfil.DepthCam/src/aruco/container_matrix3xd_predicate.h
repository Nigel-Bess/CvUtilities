//
// Created by nkaffine on 11/20/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * The purpose of this file is outline the functionality for
 * a matrix3xd predicate that returns true for 3d matrix points
 * that are in the bounds of the container with the given width
 * and length.
 */
#ifndef FULFIL_DEPTHCAM_SRC_ARUCO_CONTAINER_MATRIX3XD_PREDICATE_H_
#define FULFIL_DEPTHCAM_SRC_ARUCO_CONTAINER_MATRIX3XD_PREDICATE_H_

#include <Fulfil.CPPUtils/eigen/matrix3d_predicate.h>
#include <memory>
#include <Fulfil.DepthCam/aruco/container.h>

namespace fulfil
{
namespace depthcam
{
namespace aruco
{
 class ContainerMatrix3xdPredicate : public fulfil::utils::eigen::Matrix3dPredicate
{
 private:
  /**
   * The width of the container for the predicate in local point cloud coordinates.
   */
  float width;
  /**
   * The length of the container for the predicate in local point cloud coordinates.
   */
   float length;

   /**
    *   X local coordinate of the center of the container matrix. X axis is in line with width
    */
   float center_x;
   /**
    *  Y local coordinate of the center of the container matrix. Y axis is in line with length
    */
   float center_y;

   /**
 *   Minimum local coordinate depth allowed for filtering
 */
   float min_depth;
   /**
    *   Maximum local coordinate depth allowed for filtering
    */
   float max_depth;

  public:
  /**
   * ContainerMatrix3xdPredicate Constructor.
   * @param width of the container in local point cloud coordinates.
   * @param length of the container in local point cloud coordinates.
   * @param center_x is the coordinate of the center of the matrix. X axis is in line with width
   * @param center_y is the coordinate of the center of the matrix. Y axis is in line with length
   */
  ContainerMatrix3xdPredicate(float width, float length, float center_x = 0.0, float center_y = 0.0, float min_depth = -100, float max_depth = 100);
  /**
   * Returns true if the point is within the bounds defined by the width and
   * length provided during the construction of this predcate.
   * @param point the matrix point being evaluated.
   * @return true if point is in bounds, false otherwise.
   */
  bool evaluate(const fulfil::utils::eigen::Matrix3dPoint& point) override;
};
} // namespace fulfil
} // namespace depthcam
} // namespace aruco
#endif //FULFIL_DEPTHCAM_SRC_ARUCO_CONTAINER_MATRIX3XD_PREDICATE_H_
