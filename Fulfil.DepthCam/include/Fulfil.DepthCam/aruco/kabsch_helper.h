//
// Created by nkaffine on 12/2/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file outlines the functionality of the implementation of
 * the kabsch algorithm which finds the best transformation from
 * points in one coordinate system to a corresponding set of
 * points in another coordinate system.
 */
#ifndef FULFIL_DEPTHCAM_KABSCH_HELPER_H_
#define FULFIL_DEPTHCAM_KABSCH_HELPER_H_
#include<memory>
#include<eigen3/Eigen/Dense>
#include<eigen3/Eigen/Geometry>
#include<vector>

namespace fulfil
{
namespace depthcam
{
/**
 * Class to use the kabsch algorithm to find the ideal rotation
 * matrix and transformation vector for two sets of points.
 */
class KabschHelper
{
 public:
  /**
   * Finds the most efficient translation that transforms the initial_points to the destination_points.
   * @param initial_points 3xN matrix of doubles.
   * @param destination_points 3xN matrix of doubles.
   * @return the most efficient transformation from the intial points to the destination points.
   */
  std::shared_ptr<Eigen::Affine3d> find_translation_between_points(std::shared_ptr<Eigen::Matrix3Xd> initial_points,
                                                                   std::shared_ptr<Eigen::Matrix3Xd> destination_points);

  Eigen::Affine3d fit_transform_between_points(Eigen::Matrix3Xd initial_points,
                                                       Eigen::Matrix3Xd destination_points);

};
} // namespace fulfil
} // namespace depthcam
#endif