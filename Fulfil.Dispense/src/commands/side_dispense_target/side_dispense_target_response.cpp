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
    nlohmann::json result_json {};
    result_json["Error"] = this->success_code;
    result_json["Error_Description"] = this->error_description ? *(this->error_description) : "";
    result_json["X"] = this->x;
    result_json["Y"] = this->y;
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
        std::shared_ptr<std::string> command_handshake_id, 
        int success_code, 
        std::shared_ptr<std::string> error_desc) : 
        command_id(command_handshake_id), 
        success_code(success_code), 
        error_description(error_desc) {}

std::shared_ptr<std::string> SideDispenseTargetResponse::get_command_id()
{
    return this->command_id;
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