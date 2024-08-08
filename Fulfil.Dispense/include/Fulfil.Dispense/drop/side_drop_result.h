//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_SIDE_DROP_RESULT_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_SIDE_DROP_RESULT_H_
#include <memory>
#include <fstream>
#include <Fulfil.CPPUtils/point_3d.h>

namespace fulfil::dispense::drop
{
/**
 * The purpose of this class is to contain all of the desired
 * information about the result of a drop request.
 */
class SideDropResult
{
 private:
 
 public:

  /**
   * SideDropResult constructor that initializes the request id with
   * the given request id and marks the result as a failure.
   * @param request_id pointer to string with request id.
   */
  explicit SideDropResult(std::shared_ptr<std::string> request_id, int error_code, const std::string &error_description);

  // /**
  //  * SideDropResult constructor that takes in the center of the drop location
  //  * and the request and does all of the calculations to determine the
  //  * output dispense information to the VLS.
  //  * Uses request id as request id of the result and sets the result to
  //  * a success.
  //  * @param drop_center pointer to center of the drop zone.
  //  * @param request used to calculate the drop zone
  //  */
  // SideDropResult(std::shared_ptr<fulfil::utils::Point3D> drop_center, std::shared_ptr<fulfil::utils::Point3D> max_depth_point, bool Rotate_LFB, bool LFB_Currently_Rotated,
  //            bool Swing_Collision_Expected, float target_depth_range, float target_depth_variance, float interference_max_z,
  //            float interference_average_z, float target_region_max_z, std::shared_ptr<std::string> request_id, int success_code, const std::string &error_description);
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
  };
}

#endif // FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_SIDE_DROP_RESULT_H_