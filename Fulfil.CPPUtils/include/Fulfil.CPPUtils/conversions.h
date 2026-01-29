#ifndef FULFIL_CPPUTILS_CONVERSIONS_H
#define FULFIL_CPPUTILS_CONVERSIONS_H

#include <memory>
#include <vector>

namespace fulfil::utils
{
    std::shared_ptr<std::vector<std::shared_ptr<std::vector<int>>>> convert_map_to_integers(std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> map);
    float meter_to_mm(float meters);
    float mm_to_meter(float millimeters);
    float to_meters(float millimeters);
    int round_to_nearest_int(float f);
}
#endif //FULFIL_CPPUTILS_CONVERSIONS_H
