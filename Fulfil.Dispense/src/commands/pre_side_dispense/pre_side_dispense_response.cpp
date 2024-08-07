//
// Created by Jess on 8/6/24.
//

#include "Fulfil.Dispense/commands/pre_side_dispense/pre_side_dispense_response.h"
#include <Fulfil.CPPUtils/logging.h>

#include <json.hpp>
#include <iostream>

using fulfil::dispense::commands::PreSideDispenseResponse;
using fulfil::utils::Logger;

void PreSideDispenseResponse::encode_payload()
{
    nlohmann::json result_json{};
    result_json["Error"] = this->success_code;

    std::string json_string = result_json.dump();
    Logger::Instance()->Info("Encoding PreSideDispenseResponse as: {}", json_string);
    int json_length = json_string.size();
    const char* json_text = json_string.c_str();
    char* response = new char[json_length + 1];
    memcpy(response, json_text, json_length);
    response[json_length] = 0;
    this->payload = std::make_shared<std::string>(response);
    delete [] response;
}


PreSideDispenseResponse::PreSideDispenseResponse(std::shared_ptr<std::string> request_id, 
                                                 SideDispenseErrorCodes success_code) : request_id(request_id), success_code(success_code){}

std::shared_ptr<std::string> PreSideDispenseResponse::get_command_id()
{
    return this->request_id;
}

int PreSideDispenseResponse::dispense_payload_size()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload->length() + 1;
}

std::shared_ptr<std::string> PreSideDispenseResponse::dispense_payload()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload;
}