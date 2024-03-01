//
// Created by steve on 2/25/21.
//

#include "Fulfil.Dispense/commands/post_drop/post_LFR_response.h"
#include <Fulfil.Dispense/json.hpp>
#include <iostream>

using fulfil::dispense::commands::PostLFRResponse;
using fulfil::utils::Logger;

void PostLFRResponse::encode_payload()
{
  std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
  (*result_json)["Error"] = this->success_code;
  (*result_json)["Max_Z"] = this->max_Z;
  (*result_json)["Items_Dispensed"] = this->items_dispensed;
  (*result_json)["Bag_Full_Percent"] = this->Bag_Full_Percent;
  (*result_json)["Item_On_Target_Percent"] = this->Item_On_Target_Percent;
  (*result_json)["Products_To_Overflow"] = this->Products_To_Overflow;

  std::string json_string = result_json->dump();
  Logger::Instance()->Info("Encoding PostLFRResponse as: {}", json_string);
  int json_length = json_string.size();
  const char* json_text = json_string.c_str();
  char* response = new char[json_length + 1];
  memcpy(response, json_text, json_length);
  response[json_length] = 0;
  this->payload = std::make_shared<std::string>(response);
  delete [] response;
}


PostLFRResponse::PostLFRResponse(std::shared_ptr<std::string> command_id, int success_code, float max_Z,
                                 int items_dispensed, int Bag_Full_Percent, int Item_On_Target_Percent)
{
  this->command_id = command_id;
  this->success_code = success_code;
  this->max_Z = std::round(max_Z * 1000); //conversion from meters to mm for VLSG response
  this->items_dispensed = items_dispensed;
  this->Bag_Full_Percent = Bag_Full_Percent;
  this->Item_On_Target_Percent = Item_On_Target_Percent;
  this->Products_To_Overflow = std::vector<int>();
}

std::shared_ptr<std::string> PostLFRResponse::get_command_id()
{
  return this->command_id;
}

int PostLFRResponse::get_success_code()
{
  return this->success_code;
}

void PostLFRResponse::set_items_dispensed(std::pair<int, int> values)
{
  this->items_dispensed = values.first;
  this->Item_On_Target_Percent = values.second;
}

void PostLFRResponse::set_products_to_overflow(std::vector<int> products)
{
    this->Products_To_Overflow = products;
}

int PostLFRResponse::dispense_payload_size()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload->length() + 1;
}

std::shared_ptr<std::string> PostLFRResponse::dispense_payload()
{
  if(!(this->payload))
  {
    this->encode_payload();
  }
  return this->payload;
}