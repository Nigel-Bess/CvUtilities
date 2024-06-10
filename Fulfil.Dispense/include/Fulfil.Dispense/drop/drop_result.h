//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DROP_RESULT_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DROP_RESULT_H_
#include <memory>
#include <fstream>
#include <Fulfil.CPPUtils/point_3d.h>

namespace fulfil::dispense::drop
{
/**
 * The purpose of this class is to contain all of the desired
 * information about the result of a drop request.
 */
class DropResult
{
 private:

  /** Converts the value in meters to a value in mm units and rounds  *
  * @param double value in meter units
  * @return rounded value to the nearest unit mm
  */
  float to_rounded_millimeters(float meters);

 public:

  /**
   * DropResult constructor that initializes the request id with
   * the given request id and marks the result as a failure.
   * @param request_id pointer to string with request id.
   */
  explicit DropResult(std::shared_ptr<std::string> request_id, int error_code, const std::string &error_description);

  /**
   * DropResult constructor that takes in the center of the drop location
   * and the request and does all of the calculations to determine the
   * output dispense information to the VLS.
   * Uses request id as request id of the result and sets the result to
   * a success.
   * @param drop_center pointer to center of the drop zone.
   * @param request used to calculate the drop zone
   */
  DropResult(std::shared_ptr<fulfil::utils::Point3D> drop_center, std::shared_ptr<fulfil::utils::Point3D> max_depth_point, bool Rotate_LFB, bool LFB_Currently_Rotated,
             bool Swing_Collision_Expected, float target_depth_range, float target_depth_variance, float interference_max_z,
             float interference_average_z, float target_region_max_z, std::shared_ptr<std::string> request_id, int success_code, const std::string &error_description);
  /**
   * The id of the request.
   */
  std::shared_ptr<std::string> request_id;
  /**
   * Defines success of drop search algorithm
   *  0 = success
   *  > 0 specifies individual errors that occurred during the algorithm
   */
  int success_code;
  /**
   * Description of the error code thrown. Will be empty string if code is success.
   */
  std::string error_description;
  /**
   * The output dispense offset from the center of the LFB bag (X axis) in mm units
   */
  float rover_position;
  /**
   * The output dispense offset from the center of the LFB bag (Y axis) in mm units
   */
  float dispense_position;

  /**
   * The Y-coordinate of the tallest detected item in the bag in mm units
   */
  float max_depth_point_X;
  /**
  * The Y-coordinate of the tallest detected item in the bag in mm units
  */
  float max_depth_point_Y;
  /**
  * The depth of the tallest detected item in the bag in mm units, as measured from the top surface of the LFB bag (negative = below top of LFB)
  */
  float max_Z;

  /**
  * Indicates that target is being sent to VLSG with understanding that LFB will be rotated 180 degrees before the dispense
  */
  bool Rotate_LFB;

  /**
   * Indicates that CV detected the bot as rotated 180 degrees (= 1) or nominal orientation (= 0) or unsure (= -1)
   */
  bool LFB_Currently_Rotated;

  /**
  * Indicates that dispensed item is expected to collide with item already in bag during swing part of the dispense
  */
  bool Swing_Collision_Expected;
  /**
   * The output depth, in meters from the plane of the aruco markers on the top
   *  of the LFB rover. -Z is deeper (further away from the depth cam). mm units
   */
  float depth_result;

  /**
   * Metrics of the drop target region for evaluation of drop target selection quality
   */
  float target_depth_range;
  float target_depth_variance;
  float interference_max_z;
  float interference_average_z;
  float target_region_max_z;
  };
}

#endif // FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DROP_RESULT_H_