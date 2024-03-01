//
// Created by steve on 12/27/21.
//

#include "Fulfil.Dispense/commands/nop/nop_response.h"
#include <Fulfil.Dispense/json.hpp>
#include <iostream>

using fulfil::dispense::commands::NopResponse;
using fulfil::utils::Logger;

void NopResponse::encode_payload()
{
  std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
  (*result_json)["Error"] = this->success_code;
  (*result_json)["Rail_Motor_Stationary"] = this->rail_motor_stationary;

  std::string json_string = result_json->dump();
  Logger::Instance()->Trace("Encoding NopResponse as: {}", json_string);
  int json_length = json_string.size();
  const char* json_text = json_string.c_str();
  char* response = new char[json_length + 1];
  memcpy(response, json_text, json_length);
  response[json_length] = 0;
  this->payload = std::make_shared<std::string>(response);
  delete [] response;
}


NopResponse::NopResponse(std::shared_ptr<std::string> command_id, int success_code, bool rail_motor_stationary)
{
  this->command_id = command_id;
  this->success_code = success_code;
  this->rail_motor_stationary = rail_motor_stationary;
}

std::shared_ptr<std::string> NopResponse::get_command_id()
{
  return this->command_id;
}

int NopResponse::get_success_code()
{
  return this->success_code;
}

int NopResponse::dispense_payload_size()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload->length() + 1;
}

std::shared_ptr<std::string> NopResponse::dispense_payload()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload;
}