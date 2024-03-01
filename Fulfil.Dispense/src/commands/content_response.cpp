//
// Created by mstarkey on 01/18/24.
//

#include "Fulfil.Dispense/commands/content_response.h"
#include <Fulfil.Dispense/json.hpp>
#include <iostream>

using fulfil::dispense::commands::ContentResponse;


void ContentResponse::encode_payload()
{
  std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
  (*result_json)["Content"] = *this->content.get();

  std::string json_string = result_json->dump();
  std::cout << "content response: " << json_string << std::endl;
  int json_length = json_string.size();
  const char* json_text = json_string.c_str();
  char* response = new char[json_length + 1];
  memcpy(response, json_text, json_length);
  response[json_length] = 0;
  this->payload = std::make_shared<std::string>(response);
  delete [] response;
}

ContentResponse::ContentResponse(std::shared_ptr<std::string> command_id, std::shared_ptr<std::string> reply, DepthCameras::MessageType msg_type)
{
  this->content = reply;
  this->command_id = command_id;
  this->message_type = msg_type;
}

std::shared_ptr<std::string> ContentResponse::get_command_id()
{
  return this->command_id;
}

int ContentResponse::dispense_payload_size()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload->length() + 1;
}

std::shared_ptr<std::string> ContentResponse::dispense_payload()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload;
}


