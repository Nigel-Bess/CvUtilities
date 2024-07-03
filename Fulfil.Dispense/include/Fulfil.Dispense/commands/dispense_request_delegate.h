//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_DELEGATE_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_DELEGATE_H_

#include <Fulfil.Dispense/commands/drop_target/drop_target_details.h>
#include <Fulfil.Dispense/commands/floor_view/floor_view_response.h>
#include <Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_response.h>
#include <Fulfil.Dispense/commands/post_drop/post_LFR_response.h>
#include <Fulfil.Dispense/commands/post_side_dispense/post_side_dispense_response.h>
#include <Fulfil.Dispense/commands/side_dispense_target/side_dispense_target_response.h>
#include <Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_response.h>
#include <Fulfil.Dispense/commands/tray_validation/tray_validation_response.h>
#include <Fulfil.Dispense/drop/drop_result.h>
#include <Fulfil.Dispense/tray/tray_algorithm.h>

namespace fulfil::dispense::commands
{
/**
 * The purpose of this class is to separate the implementation for handling commands from the class that parses commands.
 */
class DispenseRequestDelegate
{
 public:
  /**
   * Given the details of a drop target request, performs the necessary processing and returns the result.
   */
  virtual std::shared_ptr<fulfil::dispense::drop::DropResult> handle_drop_target(std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details,
                                                                                  std::shared_ptr<nlohmann::json> request_json) = 0;

  virtual std::shared_ptr<ItemEdgeDistanceResponse> handle_item_edge_distance(std::shared_ptr<std::string> command_id,
                                                                              std::shared_ptr<nlohmann::json> request_json) = 0;

  /**
   * performs image data logging in response to a post-dispense request
   */
  virtual std::shared_ptr<fulfil::dispense::commands::PostLFRResponse> handle_post_LFR(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json) = 0;

  /**
   * performs image data logging in response to a floor-view request
   */
  virtual std::shared_ptr<fulfil::dispense::commands::FloorViewResponse> handle_floor_view(std::shared_ptr<std::string> PrimaryKeyID,
                                                                                           std::shared_ptr<std::string> command_id,
                                                                                           std::shared_ptr<nlohmann::json> request_json) = 0;

  /**
   * Begins image data logging in response to a request for LFB sessions
   */
  virtual void handle_start_lfb_video(std::shared_ptr<std::string> PrimaryKeyID) = 0;

  /**
   * Stops image data logging in response to a request for LFB sessions
   */
  virtual void handle_stop_lfb_video() = 0;

  /**
   * Begins image data logging in response to a request for tray sessions
   */

  virtual void handle_start_tray_video(std::shared_ptr<std::string> PrimaryKeyID) = 0;

  /**
   * Stops image data logging in response to a request for tray sessions
   */
  virtual void handle_stop_tray_video(int delay) = 0;

  /**
   * Given the id of the command to stop, handle the processes of stopping it.
   * @param command_id string with command id of the command to stop.
   */
  virtual void handle_stop_request(std::shared_ptr<std::string> command_id) = 0;

  virtual std::shared_ptr<fulfil::dispense::commands::TrayValidationResponse>
  handle_tray_validation(std::shared_ptr<std::string> command_id, std::shared_ptr<nlohmann::json> request_json) = 0;

  virtual std::string  handle_get_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json) = 0;

  virtual int handle_update_state(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json) = 0;

  virtual int handle_pre_LFR(std::shared_ptr<std::string> PrimaryKeyID, std::shared_ptr<nlohmann::json> request_json) = 0;

  virtual bool check_motor_in_position() = 0;


  virtual std::shared_ptr<fulfil::dispense::commands::SideDispenseTargetResponse> handle_side_dispense_target(std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json) = 0;
  virtual std::shared_ptr<fulfil::dispense::commands::PostSideDispenseResponse> handle_post_side_dispense(std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json) = 0;
  virtual int handle_pre_side_dispense(std::shared_ptr<std::string> request_id, std::shared_ptr<nlohmann::json> request_json) = 0;


};
} // namespace fulfil::dispense::commands

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_COMMANDS_DISPENSE_REQUEST_DELEGATE_H_
