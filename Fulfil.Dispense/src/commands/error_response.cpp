//
// Created by nkaffine on 12/12/19.
// Copyright (c) 2019 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/error_response.h"
#include <string>
#include <json.hpp>

using fulfil::dispense::commands::ErrorResponse;

void ErrorResponse::encode_payload()
{
  std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
  (*result_json)["Error"] = 1;
  std::string json_string = result_json->dump();
  int json_length = json_string.size();
  const char* json_text = json_string.c_str();
  char* response = new char[json_length + 1];
  std::memcpy(response, json_text, json_length);
  response[json_length] = 0;
  this->payload = std::make_shared<std::string>(response);
  delete [] response;
}

ErrorResponse::ErrorResponse()
{
  this->command_id = nullptr;
}

ErrorResponse::ErrorResponse(std::shared_ptr<std::string> command_id)
{
  this->command_id = command_id;
}

std::shared_ptr<std::string> ErrorResponse::get_command_id()
{
  return this->command_id;
}

int ErrorResponse::dispense_payload_size()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload->length() + 1;
}

std::shared_ptr<std::string> ErrorResponse::dispense_payload()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload;
}
