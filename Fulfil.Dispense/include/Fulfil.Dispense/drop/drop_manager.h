//
// Created by nkaffine on 12/9/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#ifndef FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DROP_MANAGER_H_
#define FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DROP_MANAGER_H_
#include <memory>
#include <Fulfil.CPPUtils/logging.h>
#include <Fulfil.CPPUtils/point_3d.h>
#include <Fulfil.DepthCam/aruco.h>
#include <Fulfil.DepthCam/core.h>
#include <Fulfil.Dispense/commands/dispense_request.h>
#include <Fulfil.Dispense/commands/dispense_response.h>
#include <Fulfil.Dispense/commands/post_drop/post_LFR_response.h>
#include <Fulfil.Dispense/drop/drop_result.h>
#include <Fulfil.Dispense/mongo/mongo_bag_state.h>
#include <Fulfil.Dispense/visualization/live_viewer.h>
#include <experimental/filesystem>
#include "drop_zone_searcher.h"
#include "pre_post_compare.h"

namespace std_filesystem = std::experimental::filesystem;

namespace fulfil::dispense::drop
{
/**
 * The purpose of this class is to handle the process
 * of accepting drop requests and sending responses.
 */
class DropManager
{
 private:
  /**
   * The session that will be used to determine the optimal drop.
   */
  std::shared_ptr<fulfil::depthcam::Session> session;

  std::shared_ptr<fulfil::dispense::visualization::LiveViewer> drop_live_viewer;

  float acceptable_Z_above_marker_surface; //recipe value. for convenience and use in multiple functions


 public:
  /**
   * DropManager Constructor that takes in the session that will be used to
   * calculate the optimal drop location.
   * @param session
   */
  explicit DropManager(std::shared_ptr<fulfil::depthcam::Session> session, std::shared_ptr<INIReader> dispense_man_reader,
                       std::shared_ptr<fulfil::dispense::visualization::LiveViewer> drop_live_viewer);

  /**
   * Generates the data received before the handling of the drop target
   * @param base_directory directory to write data to
   * @param time_stamp used in creation of the final file path to write data to
   * @param request_json JSON data to be written
   */
  void generate_data_pre_drop_target(std_filesystem::path base_directory,
                                                  const std::shared_ptr<std::string> &time_stamp,
                                                  std::shared_ptr<nlohmann::json> request_json);
  /**
   * Generates the data resulting from the drop target handling
   * @param target_file destination filepath to write drop target coordinate data
   * @param error_code success or error code of the drop target algorithm
   * @param error_code_file destination filepath to write error code data
   * @param rover_position drop target coordinate data to write
   * @param dispense_position drop target coordinate data to write
   */
  void generate_drop_target_result_data(std::string target_file, std::string error_code_file, float rover_position,
                                        float dispense_position, int error_code);
  /**
   * Processes the given drop request and returns a drop result.
   * @param request containing details on requirements for drop location
   * @return a pointer to drop result which contains details about where to
   * drop an item based on the request.
   */
  std::shared_ptr<fulfil::dispense::drop::DropResult> handle_drop_request(std::shared_ptr<INIReader> LFB_config_reader,
                                                                          std::shared_ptr<nlohmann::json> request_json,
                                                                          std::shared_ptr<fulfil::dispense::commands::DropTargetDetails> details,
                                                                          const std::shared_ptr<std::string> &base_directory,
                                                                          const std::shared_ptr<std::string> &time_stamp,
                                                                          bool generate_data=true, bool bot_has_already_rotated = false);

  std::shared_ptr<fulfil::dispense::commands::PostLFRResponse> handle_post_LFR(std::shared_ptr<INIReader> LFB_config_reader,
                                                                               std::shared_ptr<nlohmann::json> request_json,
                                                                          const std::shared_ptr<std::string> &base_directory_input,
                                                                               std::shared_ptr<std::string> request_id,
                                                                          bool generate_data=true);

  std::pair<int, int> handle_pre_post_compare(std::shared_ptr<INIReader> LFB_config_reader, std::string PrimaryKeyID);

  std::vector<int> check_products_for_fit_in_bag(std::shared_ptr<INIReader> LFB_config_reader, std::shared_ptr<nlohmann::json> request_json);

  /**
   * Delegate to receive information from the drop manager.
   */

  std::shared_ptr<fulfil::dispense::drop::DropZoneSearcher> searcher;

  std::shared_ptr<fulfil::dispense::drop::PrePostCompare> pre_post_compare;

  std::shared_ptr<cv::Point2f> cached_drop_target;

  std::shared_ptr<int> cached_drop_damage_code;

  std::shared_ptr<depthcam::aruco::MarkerDetectorContainer> cached_drop_target_container;

  std::shared_ptr<depthcam::aruco::MarkerDetectorContainer> cached_pre_container;

  std::shared_ptr<depthcam::aruco::MarkerDetectorContainer> cached_post_container;

  std::shared_ptr<nlohmann::json> cached_drop_target_request;

  std::shared_ptr<nlohmann::json> cached_pre_request;

  std::shared_ptr<nlohmann::json> cached_post_request;

  std::shared_ptr<std::vector<std::string>> cached_info = nullptr; //for live visualization purposes

  /**
  *  Caches variables for mongo bag state functionality
  */
  std::shared_ptr<fulfil::mongo::MongoBagState> mongo_bag_state;



};

} // namespace fulfil

#endif //FULFIL_DISPENSE_INCLUDE_FULFIL_DISPENSE_DROP_MANAGER_H_
