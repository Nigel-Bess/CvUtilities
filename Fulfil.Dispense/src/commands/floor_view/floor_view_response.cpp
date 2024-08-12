//
// Created by jessv on 5/24/24.
//

#include "Fulfil.Dispense/commands/floor_view/floor_view_response.h"
#include <json.hpp>
#include <iostream>

using fulfil::dispense::commands::FloorViewResponse;
using fulfil::utils::Logger;

void FloorViewResponse::encode_payload()
{
    std::shared_ptr<nlohmann::json> result_json = std::make_shared<nlohmann::json>();
    (*result_json)["Error"] = this->success_code;
    // only add the error description to responses that had an error to describe
    if(this->success_code != 0)
    {
        (*result_json)["Error_Description"] = this->error_description;
    }
    // for the success codes that do not early fail the algorithm, fill the JSON with the results
    if(this->success_code == 0 or this->success_code == 9)
    {
        (*result_json)["Anomaly_Present"] = this->anomaly_present;
        (*result_json)["Item_On_Ground"] = this->item_on_ground;
        (*result_json)["Floor_Analysis_Confidence_Score"] = (int)this->floor_analysis_confidence_score;
    }

    std::string json_string = result_json->dump();
    Logger::Instance()->Info("Encoding FloorViewResponse: {}", json_string);
    int json_length = json_string.size();
    const char* json_text = json_string.c_str();
    char* response = new char[json_length + 1];
    memcpy(response, json_text, json_length);
    response[json_length] = 0;
    this->payload = std::make_shared<std::string>(response);
    delete [] response;
}

FloorViewResponse::FloorViewResponse(std::shared_ptr<std::string> request_id, int success_code, std::string error_description,
                                     bool anomaly_present, bool item_on_ground, float floor_analysis_confidence_score)
{
    this->success_code = success_code;
    this->request_id = request_id;
    this->error_description = error_description;
    this->anomaly_present = anomaly_present;
    this->item_on_ground = item_on_ground;
    this->floor_analysis_confidence_score = floor_analysis_confidence_score;
}

std::shared_ptr<std::string> FloorViewResponse::get_command_id()
{
    return this->request_id;
}

int FloorViewResponse::get_success_code()
{
    return this->success_code;
}

int FloorViewResponse::dispense_payload_size()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload->length() + 1;
}

std::shared_ptr<std::string> FloorViewResponse::dispense_payload()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload;
}


