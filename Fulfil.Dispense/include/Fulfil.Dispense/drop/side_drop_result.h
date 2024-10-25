//
// Created by jessvdv on 8/8/24.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_SIDE_DROP_RESULT_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_SIDE_DROP_RESULT_H_
#include <memory>
#include <vector>
#include <fstream>
#include <Fulfil.CPPUtils/point_3d.h>
#include <Fulfil.DepthCam/aruco/marker_detector_container.h>

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
   explicit SideDropResult(std::shared_ptr<std::string> request_id,
    std::shared_ptr<std::vector<std::vector<float>>> occupancy_map,
    int error_code,
    const std::string &error_description);

  SideDropResult(std::shared_ptr<std::string> request_id,
   std::shared_ptr<std::vector<std::vector<float>>> occupancy_map,
   std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> container,
   float square_width,
   float square_height,
   int error_code,
   const std::string &error_description);



  /**
   * The id of the request.
   */
  std::shared_ptr<std::string> request_id;

  std::shared_ptr<std::vector<std::vector<float>>> occupancy_map;

  std::shared_ptr<fulfil::depthcam::aruco::MarkerDetectorContainer> container;

  float square_width;
  
  float square_height;

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

  std::string to_string();
  };
}

#endif // FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_SIDE_DROP_RESULT_H_