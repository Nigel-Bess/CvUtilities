#include "Fulfil.CPPUtils/conversions.h"
#include "Fulfil.CPPUtils/logging.h"

using fulfil::utils::Logger;

float fulfil::utils::meter_to_mm(float meters) {
    return (float)((meters) * 1000);
}
float fulfil::utils::mm_to_meter(float millimeters) {
    return (float)((millimeters) / 1000);
}
float fulfil::utils::to_meters(float millimeters) {
    return (float)((millimeters) / 1000);
}

int fulfil::utils::round_to_nearest_int(float f){
    return (int)std::lround(f);
}

namespace {
constexpr float pi = 3.14159265358979323846f;
}

float fulfil::utils::deg_to_rad(float deg)
{
    return deg * pi / 180.0f;
}

float fulfil::utils::rad_to_deg(float rad)
{
    return rad * 180.0f / pi;
}


std::shared_ptr<std::vector<int>> convert_map_row_to_integers(std::shared_ptr<std::vector<float>> input) {
    std::vector<int> row;

    for (int j = 0; j < input->size(); j++) {
        row.push_back(fulfil::utils::round_to_nearest_int(input->at(j)));
    }
    return std::make_shared<std::vector<int>>(row);
}


std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> fulfil::utils::convert_map_to_integers(std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> map) {
    if (map == nullptr) {
        Logger::Instance()->Debug("convert_map_to_integers: Given map was nullptr!");
        return nullptr;
    }

    std::vector<std::shared_ptr<std::vector<int>>> output;
    for (int i = 0; i < map->size(); i++) {
        output.push_back(convert_map_row_to_integers(map->at(i)));
    }

    // TODO add count
    Logger::Instance()->Debug("Number of null squares in the occupancy map before converting them: IDK out of total sqaures: {}", map->size() * map->at(0)->size());
    return std::make_shared<std::vector<std::shared_ptr<std::vector<int>>>>(output);
}