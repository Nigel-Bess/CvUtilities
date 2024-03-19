//
// Created by steve on 5/11/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TRAY_VALIDATION_DETAILS_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TRAY_VALIDATION_DETAILS_H_
#include <memory>
#include <json.hpp>
#include <Fulfil.Dispense/tray/tray_lane.h>

namespace fulfil::dispense::commands
{
/**
 * The purpose of this class is to contain all of the required
 * information for processing a tray height request
 */
class TrayValidationDetails
{
 public:

  //basic constructor for use in offline_test
  TrayValidationDetails();
  /**
   * TrayValidationDetails constructor.
   * @param request_id pointer to string with the id for the request.
   */
  TrayValidationDetails(std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json);
  /**
   * A string containing the id for the request.
   */
  std::shared_ptr<std::string> request_id;

  double height_tolerance;

  std::shared_ptr<std::vector<std::shared_ptr<fulfil::dispense::tray::TrayLane>>> tray_lanes;

  //bool rigid;

  //std::shared_ptr<std::vector<std::shared_ptr<LaneDetails>>> lane_details;  Todo: handle lanes here

};
} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_TRAY_VALIDATION_DETAILS_H_
