#ifndef FULFIL_CPPUTILS_CONVERSIONS_H
#define FULFIL_CPPUTILS_CONVERSIONS_H

#include <memory>
#include <vector>

namespace fulfil::utils
{
    std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> convert_map_to_millimeters(std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> map);
    std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> convert_map_to_millimeters1(std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> map);
    int to_millimeters(float meters);
    float to_millimeters_float(float meters);
    float to_meters(float millimeters);
}
#endif //FULFIL_CPPUTILS_CONVERSIONS_H
