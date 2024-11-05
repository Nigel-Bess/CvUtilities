#include "Fulfil.CPPUtils/conversions.h"

/* Converts the given meter measurement to a rounded int millimeter value */
int fulfil::utils::to_millimeters(float meters) {
	return (int)((meters) * 1000);
}

/* Generates a converted millimeter equivalent of the given grid map in meters */
std::shared_ptr<std::vector<std::vector<int>>> fulfil::utils::convert_map_to_millimeters(std::shared_ptr<std::vector<std::vector<float>>> map) {
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