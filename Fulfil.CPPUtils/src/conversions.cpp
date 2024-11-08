#include "Fulfil.CPPUtils/conversions.h"
#include "Fulfil.CPPUtils/logging.h"

using fulfil::utils::Logger;

/* Converts the given meter measurement to a rounded int millimeter value */
int fulfil::utils::to_millimeters(float meters) {
	return (int)((meters) * 1000);
}

int convert_to_bag_contents_depth(float z_depth) {
    z_depth = fulfil::utils::to_millimeters(z_depth);
    if (z_depth < -999) {
        // TODO base on config //HACK
        z_depth = -400;
    }
    // TODO base on config //HACK
    return z_depth + 400;
}

std::vector<int> convert_row(std::vector<float> input) {
    std::vector<int> row(input.size());
    for (int j = 0; j < input.size(); j++) {
        row.push_back(convert_to_bag_contents_depth(input.at(j)));
    }
    return row;
}

/* Generates a converted millimeter equivalent of the given grid map in meters */
std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> fulfil::utils::convert_map_to_millimeters(std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> map) {
    if (map == nullptr) {
        Logger::Instance()->Debug("convert_map_to_millimeters: Given map was nullptr!");
        return nullptr;
    }

    std::vector<std::shared_ptr<std::vector<int>>> output;
    for (int i = 0; i < map->size(); i++) {
      output.push_back(std::make_shared<std::vector<int>>(convert_row(*map->at(i))));
    }

    // TODO add count
    Logger::Instance()->Debug("Number of null squares in the occupancy map before converting them: IDK out of total sqaures: {}", map->size()*map->at(0)->size());
    return std::make_shared<std::vector<std::shared_ptr<std::vector<int>>>>(output);
}