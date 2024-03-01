//
// Created by nkaffine on 11/20/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//
/**
 * This file contains the implementation of a predicate that returns
 * true for 3d matrix points that are inside the box bounded by the
 * width and length with (0,0,0) at the center
 */
#include "container_matrix3xd_predicate.h"

using fulfil::depthcam::aruco::ContainerMatrix3xdPredicate;


/**
 *
 *  NOTES:  Width and X local coordinate axis are always along the image VERTICAL (this is opposite of OpenCV)
 */
ContainerMatrix3xdPredicate::ContainerMatrix3xdPredicate(float width, float length, float center_x, float center_y, float min_depth, float max_depth)
{
  this->width = width;
  this->length = length;
  this->center_x = center_x;
  this->center_y = center_y;
  this->min_depth = min_depth;
  this->max_depth = max_depth;
}

bool ContainerMatrix3xdPredicate::evaluate(const fulfil::utils::eigen::Matrix3dPoint&point)
{
  return point(0) >= center_x - this->width/2
      && point(0) <= center_x + this->width/2
      && point(1) >= center_y - this->length/2
      && point(1) <= center_y + this->length/2
      && point(2) <= max_depth
      && point(2) >= min_depth;
}
