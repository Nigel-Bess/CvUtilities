//
// Created by amber on 10/12/20.
//

#include "Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_response.h"
#include <Fulfil.CPPUtils/logging.h>
#include <json.hpp>
#include <iostream>
#include <utility>

using fulfil::dispense::commands::ItemEdgeDistanceResponse;
using fulfil::utils::Logger;

void ItemEdgeDistanceResponse::encode_payload()
{
    nlohmann::json result_json(count_result);
    auto dist = nlohmann::json(lane_distance_info);
    json_parser::mongo_utils::array_extend(dist["Errors"], result_json["Errors"]);
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


int fulfil::dispense::commands::ItemEdgeDistanceResponse::get_fed_value() const {
    return this->lane_distance_info.m_first_item_distance;
}


ItemEdgeDistanceResponse::ItemEdgeDistanceResponse(std::shared_ptr<std::string> command_id, int success_code)
    : command_id{std::move(command_id)}, success_code{success_code} {}


fulfil::dispense::commands::ItemEdgeDistanceResponse::ItemEdgeDistanceResponse(
        results_to_vlsg::LaneItemDistance lane_item_distance, results_to_vlsg::TrayValidationCounts lane_count_result,
        std::shared_ptr<std::string> command_id) :
        lane_distance_info(std::move(lane_item_distance)), count_result(std::move(lane_count_result)), command_id{std::move(command_id)} {}

