//
// Created by amber on 10/12/20.
//

#include "Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_response.h"
#include <json.hpp>
#include <iostream>
#include <utility>

using fulfil::dispense::commands::ItemEdgeDistanceResponse;
using fulfil::utils::Logger;

void ItemEdgeDistanceResponse::encode_payload()
{

  nlohmann::json result_json(count_result);
  std::cout << "COUNT OUT:\n" << result_json << '\n';
    std::cout << "SIZE OF ROI, FIRST LAST:\n" << lane_distance_info.m_roi_points.vertices.size()
    << ", " << lane_distance_info.m_roi_points.vertices.front() << ", " << lane_distance_info.m_roi_points.vertices.back() << '\n';
    auto dist = nlohmann::json(lane_distance_info);
  std::cout << "DISTANCE OUT:\n" << dist << '\n';
    std::cout << "Updating...\n";
  result_json.update(dist);
  std::cout << "Result OUT:\n" << result_json << '\n';

  result_json["Error"] = 0;
  if (success_code > 0) {
      result_json["Errors"] = std::vector<int>{success_code};
  }
  //result_json["First_Item_Distance"] = this->fed_result;
  //result_json["First_Item_Length"] = this->detected_item_length;
  //result_json["Centers"] = this->transformed_lane_center_pixels;

  this->payload = std::make_shared<std::string>(result_json.dump());
  Logger::Instance()->Info("Encoding TrayDispenseLane Response: {}", *this->payload);
}

ItemEdgeDistanceResponse::ItemEdgeDistanceResponse(std::shared_ptr<std::string> command_id, int success_code)
{
  this->success_code = success_code;
  this->command_id = command_id;
  this->count_result = results_to_vlsg::TrayValidationCounts{};


}

std::shared_ptr<std::string> ItemEdgeDistanceResponse::get_command_id()
{
  return this->command_id;
}

int ItemEdgeDistanceResponse::dispense_payload_size()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload->length() + 1;
}

std::shared_ptr<std::string> ItemEdgeDistanceResponse::dispense_payload()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload;
}

ItemEdgeDistanceResponse::ItemEdgeDistanceResponse(
        std::vector<tray_count_api_comms::LaneCenterLine> transformed_pixel_centers,
        int fed_result, int cv_detected_item_length,
        results_to_vlsg::TrayValidationCounts lane_count_result,
        std::shared_ptr<std::string> command_id, int success_code) :
        command_id{std::move(command_id)}, count_result{std::move(lane_count_result)},
        lane_distance_info(success_code, fed_result, cv_detected_item_length), success_code{success_code} {}

int fulfil::dispense::commands::ItemEdgeDistanceResponse::get_fed_value() const {
    return lane_distance_info.m_first_item_distance;
}

fulfil::dispense::commands::ItemEdgeDistanceResponse::ItemEdgeDistanceResponse(
        results_to_vlsg::LaneItemDistance lane_item_distance, results_to_vlsg::TrayValidationCounts lane_count_result,
        std::shared_ptr<std::string> command_id) :
        command_id{std::move(command_id)}, count_result(std::move(lane_count_result)), lane_distance_info(lane_item_distance) {}


