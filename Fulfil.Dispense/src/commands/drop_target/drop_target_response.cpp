//
// Created by nkaffine on 12/11/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/drop_target/drop_target_response.h"
#include <Fulfil.Dispense/json.hpp>
#include <iostream>

using fulfil::dispense::commands::DropTargetResponse;
using fulfil::utils::Logger;

void DropTargetResponse::encode_payload()
{
  std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
  (*result_json)["Error"] = this->success_code;
  // only add the error description to responses that had an error to describe
  if(this->success_code != 0)
  {
    (*result_json)["Error_Description"] = this->error_description;
  }
  // for the success codes that do not early fail the algorithm, fill the JSON with the results
  if(this->success_code == 0 or this->success_code == 9)
  {
    (*result_json)["x"] = this->rover_position;
    (*result_json)["y"] = this->dispense_position;
    (*result_json)["z"] = this->depth_result;
    (*result_json)["Max_Z"] = this->max_Z;
    (*result_json)["Rotate_LFB"] = this->Rotate_LFB;
    (*result_json)["LFB_Currently_Rotated"] = this->LFB_Currently_Rotated;
    (*result_json)["Swing_Collision_Expected"] = this->Swing_Collision_Expected;
  }

  std::string json_string = result_json->dump();
  Logger::Instance()->Info("Encoding DropTargetResponse: {}", json_string);
  int json_length = json_string.size();
  const char* json_text = json_string.c_str();
  char* response = new char[json_length + 1];
  memcpy(response, json_text, json_length);
  response[json_length] = 0;
  this->payload = std::make_shared<std::string>(response);
  delete [] response;
}

DropTargetResponse::DropTargetResponse(std::shared_ptr<std::string> command_id, int success_code, std::string error_description)
{
  this->success_code = success_code;
  this->command_id = command_id;
  this->error_description = error_description;
}

DropTargetResponse::DropTargetResponse(std::shared_ptr<std::string> command_id, int success_code,
                                       float rover_position, float dispense_position, float depth_result, float max_Z,
                                       bool Rotate_LFB, bool LFB_Currently_Rotated, bool Swing_Collision_Expected,
                                       std::string error_description)
{
  this->success_code = success_code;
  this->command_id = command_id;
  this->rover_position = rover_position;
  this->dispense_position = dispense_position;
  this->depth_result = depth_result;
  this->max_Z = max_Z;
  this->Rotate_LFB = Rotate_LFB;
  this->LFB_Currently_Rotated = LFB_Currently_Rotated;
  this->Swing_Collision_Expected = Swing_Collision_Expected;
  this->error_description = error_description;
}

std::shared_ptr<std::string> DropTargetResponse::get_command_id()
{
  return this->command_id;
}

int DropTargetResponse::dispense_payload_size()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload->length() + 1;
}

std::shared_ptr<std::string> DropTargetResponse::dispense_payload()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload;
}


