//
// Created by amber on 10/12/20.
//

#include "Fulfil.Dispense/commands/item_edge_distance/item_edge_distance_response.h"
#include <cmath>
#include <Fulfil.Dispense/json.hpp>
#include <iostream>

using fulfil::dispense::tray::TrayResult;
using fulfil::dispense::commands::ItemEdgeDistanceResponse;
using fulfil::utils::Logger;

void ItemEdgeDistanceResponse::encode_payload()
{
  std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
  if(this->success_code != 0)
  {
    (*result_json)["Error"] = 0;
    (*result_json)["Errors"] = {success_code};
    (*result_json)["First_Item_Distance"] = -1;
    (*result_json)["Lanes"] = nlohmann::json::array({});

    std::string final_string = result_json->dump();
    Logger::Instance()->Info("Encoding TrayDispenseLane Response: {}", final_string);
    this->payload = std::make_shared<std::string>(final_string);
  }
  else
  {
    if (this->tray_result != nullptr) {
      result_json = this->tray_result->encode_all();
    }
    (*result_json)["Error"] = 0;
    (*result_json)["First_Item_Distance"] = this->tray_result->get_first_item_edge_distance();

    std::string final_string = this->tray_result->encode_all()->dump();
    Logger::Instance()->Info("Encoding TrayDispenseLane Response: {}", final_string);
    this->payload = std::make_shared<std::string>(final_string);
  }
}

ItemEdgeDistanceResponse::ItemEdgeDistanceResponse(std::shared_ptr<std::string> command_id, int success_code)
{
  this->success_code = success_code;
  this->command_id = command_id;
}

ItemEdgeDistanceResponse::ItemEdgeDistanceResponse(std::shared_ptr<std::string> command_id, std::shared_ptr<TrayResult> tray_result)
{
  this->success_code = tray_result->success_code;
  this->command_id = command_id;
  this->tray_result = tray_result;
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


