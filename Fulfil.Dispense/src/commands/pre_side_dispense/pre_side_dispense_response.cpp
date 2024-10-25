//
// Created by Jess on 8/6/24.
//

#include "Fulfil.Dispense/commands/pre_side_dispense/pre_side_dispense_response.h"
#include <Fulfil.CPPUtils/logging.h>

#include <json.hpp>
#include <iostream>
#include <memory>
#include <vector>

using fulfil::dispense::commands::PreSideDispenseResponse;
using fulfil::utils::Logger;

int to_millimeters(float meters) {
	return (int)((meters) * 1000);
}

std::shared_ptr<std::vector<std::vector<int>>> convert_map_to_millimeters(std::shared_ptr<std::vector<std::vector<float>>> map) {
    if (map == nullptr) return nullptr;
    // NOTE if the vector ever doesn't have same-sized vectors inside, this will be bogus
    std::vector output(map->size(), std::vector<int>(map->at(0).size(), -99999));

    for (int y = 0; y < map->at(0).size(); y++) {
        for (int x = 0; x < map->size(); x++) {
            output.at(x).at(y) = to_millimeters(map->at(x).at(y));
        }
    }
    return std::make_shared<std::vector<std::vector<int>>>(output);
}

void PreSideDispenseResponse::encode_payload() {
    nlohmann::json result_json{};
    result_json["Primary_Key_ID"] = *this->primary_key_id;
    result_json["Error"] = (int) this->success_code;
    result_json["Error_Description"] = this->error_description;
    result_json["Grid_Square_Width_Mm"] = to_millimeters(this->square_width);
    result_json["Grid_Square_Height_Mm"] = to_millimeters(this->square_height);
    // if there were obstacles that couldn't be overcome making the map, don't return any
    if (this->occupancy_map != nullptr) {
        result_json["Occupancy_Map"] = *convert_map_to_millimeters(this->occupancy_map);
    }

    std::string json_string = result_json.dump();
    Logger::Instance()->Info("Encoding PreSideDispenseResponse as: {}", json_string);
    int json_length = json_string.size();
    const char *json_text = json_string.c_str();
    char *response = new char[json_length + 1];
    memcpy(response, json_text, json_length);
    response[json_length] = 0;
    this->payload = std::make_shared<std::string>(response);
    delete [] response;
}

PreSideDispenseResponse::PreSideDispenseResponse(std::shared_ptr<std::string> request_id,
                                                 std::shared_ptr<std::string> primary_key_id,
                                                 std::shared_ptr<std::vector<std::vector<float>>> occupancy_map,
                                                 float square_width,
                                                 float square_height,
                                                 SideDispenseErrorCodes success_code,
                                                 std::string error_description) : request_id(request_id),
    primary_key_id(primary_key_id),
    occupancy_map(occupancy_map),
    square_width(square_width),
    square_height(square_height),
    success_code(success_code),
    error_description(error_description) {
}

std::shared_ptr<std::string> PreSideDispenseResponse::get_command_id() {
    return this->request_id;
}

int PreSideDispenseResponse::dispense_payload_size() {
    if (!(this->payload)) {
        this->encode_payload();
    }
    return this->payload->length() + 1;
}

std::shared_ptr<std::string> PreSideDispenseResponse::dispense_payload() {
    if (!(this->payload)) {
        this->encode_payload();
    }
    return this->payload;
}

std::string grid_map_to_string(std::shared_ptr<std::vector<std::vector<float>>> map) {
    if (map == nullptr) return "nullptr";
    std::string output = "";
    std::string val;
    for (int y = 0; y < map->at(0).size(); y++) {
        output += "| ";
        for (int x = 0; x < map->size(); x++) {
            val = std::to_string(map->at(x).at(y));
            output += val + std::string(4 - val.size(), ' ') + "| ";
        }
        output += "\n";
    }
    return output;
}
