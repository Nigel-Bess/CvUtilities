//
// Created by sfburke on 4/21/20.
// Copyright (c) 2020 Fulfil Solutions, Inc. All rights reserved.
//

#include "Fulfil.Dispense/commands/code_response.h"
#include <json.hpp>
#include <iostream>

using fulfil::dispense::commands::CodeResponse;


void CodeResponse::encode_payload()
{
  std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
  (*result_json)["Error"] = this->success_code;
  (*result_json)["Error_Description"] = this->error_description ? *(this->error_description) : "";

  std::string json_string = result_json->dump();
  int json_length = json_string.size();
  const char* json_text = json_string.c_str();
  char* response = new char[json_length + 1];
  memcpy(response, json_text, json_length);
  response[json_length] = 0;
  this->payload = std::make_shared<std::string>(response);
  delete [] response;
}

CodeResponse::CodeResponse(std::shared_ptr<std::string> command_id, int success_code, std::shared_ptr<std::string> err_msg)
{
  this->success_code = success_code;
  this->command_id = command_id;
  this->error_description = err_msg;
}

std::shared_ptr<std::string> CodeResponse::get_command_id()
{
  return this->command_id;
}

int CodeResponse::dispense_payload_size()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload->length() + 1;
}

std::shared_ptr<std::string> CodeResponse::dispense_payload()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload;
}


