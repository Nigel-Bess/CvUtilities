//
// Created by steve on 5/12/20.
//

#include "Fulfil.Dispense/commands/tray_validation/tray_validation_response.h"
#include <cmath>
#include <json.hpp>
#include <iostream>
#include <utility>

using fulfil::dispense::commands::TrayValidationResponse;
using fulfil::utils::Logger;


TrayValidationResponse::TrayValidationResponse(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> payload, std::shared_ptr<std::string> err_msg)
{
  this->command_id = command_id;
  this->error_description = err_msg;

  std::string json_string = *payload;
  Logger::Instance()->Info("Encoding TrayValidation Response: {}", json_string);
  int json_length = json_string.size();
  const char* json_text = json_string.c_str();
  char* response = new char[json_length + 1];
  memcpy(response, json_text, json_length);
  response[json_length] = '\0';
  this->payload = std::make_shared<std::string>(response);
  delete [] response;
}

TrayValidationResponse::TrayValidationResponse(std::shared_ptr<std::string> command_id, int success_code, std::shared_ptr<std::string> err_msg) {
  this->command_id = command_id;
  this->error_description = err_msg;
  
  nlohmann::json result_json;
  result_json["Error"] = success_code;
  result_json["Error_Description"] = this->error_description ? *(this->error_description) : "";

  this->payload = std::make_shared<std::string>(result_json.dump());
}

std::shared_ptr<std::string> TrayValidationResponse::get_command_id()
{
  return this->command_id;
}

int TrayValidationResponse::dispense_payload_size()
{
  return this->payload->length() + 1;
}

std::shared_ptr<std::string> TrayValidationResponse::dispense_payload()
{
  return this->payload;
}


