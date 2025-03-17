//
// Created by Jess on 9/5/24.
//

#include "commands/tcs_response.h"
#include <json.hpp>
#include <iostream>

using fulfil::dispense::commands::TCSResponse;
using fulfil::utils::Logger;

void TCSResponse::encode_payload()
{
  std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
  (*result_json)["Error"] = this->success_code;
  (*result_json)["Error_Description"] = this->error_description;
  (*result_json)["Primary_Key_ID"] = *this->primary_key_id;
  if (this->always_approve_for_release) {
    (*result_json)["Is_Bag_Empty"] = true;
  } else {
    (*result_json)["Is_Bag_Empty"] = this->is_bag_empty;
  }
  (*result_json)["Is_Empty_Result"] = this->is_bag_empty;

  std::string json_string = result_json->dump();
  Logger::Instance()->Info("Encoding TCSResponse as: {}", json_string);
  int json_length = json_string.size();
  const char* json_text = json_string.c_str();
  char* response = new char[json_length + 1];
  memcpy(response, json_text, json_length);
  response[json_length] = 0;
  this->payload = std::make_shared<std::string>(response);
  delete [] response;
}


TCSResponse::TCSResponse(std::shared_ptr<std::string> command_id,
  std::shared_ptr<std::string> primary_key_id,
  bool always_approve_for_release,
  int success_code,
  bool is_bag_empty,
  std::string error_description)
{
  this->command_id = command_id;
  this->primary_key_id = primary_key_id;
  this->always_approve_for_release = always_approve_for_release;
  this->success_code = success_code;
  this->is_bag_empty = is_bag_empty;
  this->error_description = error_description;
  this->encode_payload();
}