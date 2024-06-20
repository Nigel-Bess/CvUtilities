//
// Created by amber on 6/20/24.
//

#include "Fulfil.Dispense/commands/side_dispense_target/side_dispense_target_response.h"
#include <json.hpp>
#include <iostream>

using fulfil::dispense::commands::SideDispenseTargetResponse;
using fulfil::utils::Logger;

void SideDispenseTargetResponse::encode_payload()
{
    nlohmann::json result_json = std::make_shared<nlohmann::json>();
    result_json["Error"] = this->success_code;
    result_json["X"] = this->items_dispensed;
    result_json["Y"] = this->bag_full_percent;
    result_json["Extend_Contact_Distance"] = this->extend_contact_distance;
    std::string json_string = result_json.dump();
    Logger::Instance()->Info("Encoding SideDispenseTargetResponse as: {}", json_string);
    int json_length = json_string.size();
    const char* json_text = json_string.c_str();
    char* response = new char[json_length + 1];
    memcpy(response, json_text, json_length);
    response[json_length] = 0;
    this->payload = std::make_shared<std::string>(response);
    delete [] response;
}


SideDispenseTargetResponse::SideDispenseTargetResponse(
        std::shared_ptr<std::string> command_handshake_id) : command_id(command_handshake_id);
{}

std::shared_ptr<std::string> SideDispenseTargetResponse::get_command_id()
{
    return this->command_id;
}

int SideDispenseTargetResponse::get_success_code()
{
    return this->success_code;
}

int SideDispenseTargetResponse::dispense_payload_size()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload->length() + 1;
}

std::shared_ptr<std::string> SideDispenseTargetResponse::dispense_payload()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload;
}