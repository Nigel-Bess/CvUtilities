//
// Created by amber on 6/20/24.
//

#include "Fulfil.Dispense/commands/post_side_dispense/post_side_dispense_response.h"
#include <Fulfil.CPPUtils/logging.h>

#include <json.hpp>
#include <iostream>

using fulfil::dispense::commands::PostSideDispenseResponse;
using fulfil::utils::Logger;

void PostSideDispenseResponse::encode_payload()
{
    nlohmann::json result_json{};
    result_json["Error"] = this->success_code;
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
        std::shared_ptr<std::string> request_handshake_id) : request_id(request_handshake_id){}

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