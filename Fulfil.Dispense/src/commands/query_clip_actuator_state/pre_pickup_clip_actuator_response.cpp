//
// Created by amber on 6/20/24.
//

#include <Fulfil.CPPUtils/conversions.h>
#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/commands/pre_pickup_clip_actuator/pre_pickup_clip_actuator_response.h"

#include <iostream>
#include <json.hpp>

using fulfil::dispense::commands::PrePickupClipActuatorResponse;
using fulfil::utils::Logger;

void PrePickupClipActuatorResponse::encode_payload()
{
    nlohmann::json result_json{};
    result_json["Error"] = (int) this->success_code;
    result_json["Error_Description"] = this->error_description;
    result_json["Tote_Id"] = this->tote_id;
    result_json["Facility_Id"] = this->facility_id;
    // TODO depending on the object type serialize it
    result_json["Clip_Open_States"] = "";

    std::string json_string = result_json.dump();
    Logger::Instance()->Info("Encoding PrePickupClipActuatorResponse as: {}", json_string);
    int json_length = json_string.size();
    const char* json_text = json_string.c_str();
    char* response = new char[json_length + 1];
    memcpy(response, json_text, json_length);
    response[json_length] = 0;
    this->payload = std::make_shared<std::string>(response);
    delete [] response;
}

PrePickupClipActuatorResponse::PrePickupClipActuatorResponse(
    std::shared_ptr<std::string> request_id,
    std::shared_ptr<std::string> primary_key_id,
    int tote_id,
    int facility_id,
    std::shared_ptr<std::vector<bool>> clip_open_states,
    DcApiErrorCode success_code,
    std::string error_description) :
    request_id(request_id),
    primary_key_id(primary_key_id),
    tote_id(tote_id),
    facility_id(facility_id),
    clip_open_states(clip_open_states),
    success_code(success_code),
    error_description(error_description) {}

std::shared_ptr<std::string> PrePickupClipActuatorResponse::get_command_id()
{
    return this->request_id;
}

int PrePickupClipActuatorResponse::dispense_payload_size()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload->length() + 1;
}

std::shared_ptr<std::string> PrePickupClipActuatorResponse::dispense_payload()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload;
}