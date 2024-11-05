//
// Created by amber on 6/20/24.
//

#include <Fulfil.CPPUtils/conversions.h>
#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/commands/post_side_dispense/post_side_dispense_response.h"

#include <iostream>
#include <json.hpp>

using fulfil::dispense::commands::PostSideDispenseResponse;
using fulfil::utils::convert_map_to_millimeters;
using fulfil::utils::Logger;
using fulfil::utils::to_millimeters;


void PostSideDispenseResponse::encode_payload()
{
    nlohmann::json result_json{};
    result_json["Error"] = (int) this->success_code;
    result_json["Error_Description"] = this->error_description;
    result_json["Grid_Square_Width_Mm"] = to_millimeters(this->square_width);
    result_json["Grid_Square_Height_Mm"] = to_millimeters(this->square_height);
    // if there were obstacles that couldn't be overcome making the map, don't return any
    if (this->occupancy_map != nullptr) {
        result_json["Occupancy_Map"] = *convert_map_to_millimeters(this->occupancy_map);
    }
    result_json["Items_Dispensed"] = this->items_dispensed;
    result_json["Bag_Full_Percent"] = this->bag_full_percent;
    result_json["Item_On_Target_Percent"] = this->item_on_target_percent;
    result_json["Products_To_Overflow"] = this->products_to_overflow;

    std::string json_string = result_json.dump();
    Logger::Instance()->Info("Encoding PostSideDispenseResponse as: {}", json_string);
    int json_length = json_string.size();
    const char* json_text = json_string.c_str();
    char* response = new char[json_length + 1];
    memcpy(response, json_text, json_length);
    response[json_length] = 0;
    this->payload = std::make_shared<std::string>(response);
    delete [] response;
}


PostSideDispenseResponse::PostSideDispenseResponse(
    std::shared_ptr<std::string> request_id,
    std::shared_ptr<std::string> primary_key_id,
    std::shared_ptr<std::vector<std::vector<float>>> occupancy_map,
    float square_width,
    float square_height,
    SideDispenseErrorCodes success_code,
    std::string error_description) : 
    request_id(request_id),
    primary_key_id(primary_key_id),
    occupancy_map(occupancy_map),
    square_width(square_width),
    square_height(square_height),
    success_code(success_code),
    error_description(error_description) {}

std::shared_ptr<std::string> PostSideDispenseResponse::get_command_id()
{
    return this->request_id;
}

int PostSideDispenseResponse::dispense_payload_size()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload->length() + 1;
}

std::shared_ptr<std::string> PostSideDispenseResponse::dispense_payload()
{
    if(!(this->payload))
    {
        this->encode_payload();
    }
    return this->payload;
}