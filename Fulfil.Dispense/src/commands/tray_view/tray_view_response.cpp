//
// Created by jessv on 2/3/25.
//

#include "Fulfil.Dispense/commands/tray_view/tray_view_response.h"
#include <json.hpp>
#include <iostream>

using fulfil::dispense::commands::TrayViewResponse;
using fulfil::utils::Logger;

void TrayViewResponse::encode_payload()
{
    std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
    (*result_json)["Error"] = this->success_code;
    // only add the error description to responses that had an error to describe
    if(this->success_code != 0)
    {
        (*result_json)["Error_Description"] = *this->error_description;
    }

    std::string json_string = result_json->dump();
    Logger::Instance()->Info("Encoding TrayViewResponse: {}", json_string);
    int json_length = json_string.size();
    const char* json_text = json_string.c_str();
    char* response = new char[json_length + 1];
    memcpy(response, json_text, json_length);
    response[json_length] = 0;
    this->payload = std::make_shared<std::string>(response);
    delete [] response;
}

TrayViewResponse::TrayViewResponse(std::shared_ptr<std::string> request_id, int success_code, std::shared_ptr<std::string> error_description)
{
    this->success_code = success_code;
    this->request_id = request_id;
    this->error_description = error_description;
}

std::shared_ptr<std::string> TrayViewResponse::get_command_id()
{
    return this->request_id;
}

int TrayViewResponse::get_success_code()
{
    return this->success_code;
}

int TrayViewResponse::dispense_payload_size()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload->length() + 1;
}

std::shared_ptr<std::string> TrayViewResponse::dispense_payload()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload;
}


