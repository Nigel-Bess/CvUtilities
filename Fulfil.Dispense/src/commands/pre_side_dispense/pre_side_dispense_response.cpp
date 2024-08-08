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
    result_json["Primary_Key_ID"] = *this->primary_key_id;
    result_json["Error"] = (int)this->success_code;
    result_json["Error_Description"] = this->error_description;
    // TODO: test encoding
    std::vector<std::vector<int>> map;
    for (int x = 0; x < this->occupancy_map->size(); x++) {
        map.push_back(*this->occupancy_map.at(x));
    }
    result_json["Occupancy_Map"] = map;

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
                                                 std::shared_ptr<std::string> primary_key_id,
                                                 std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> occupancy_map,
                                                 SideDispenseErrorCodes success_code,
                                                 std::string error_description) : 
                                                 request_id(request_id), 
                                                 primary_key_id(primary_key_id), 
                                                 occupancy_map(occupancy_map),
                                                 success_code(success_code), 
                                                 error_description(error_description){}

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