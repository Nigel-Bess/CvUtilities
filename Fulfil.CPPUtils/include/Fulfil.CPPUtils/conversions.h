#ifndef FULFIL_CPPUTILS_CONVERSIONS_H
#define FULFIL_CPPUTILS_CONVERSIONS_H

#include <memory>
#include <vector>

namespace fulfil::utils
{
    std::shared_ptr<std::vector<std::vector<int>>> convert_map_to_millimeters(std::shared_ptr<std::vector<std::vector<float>>> map);
    int to_millimeters(float meters);
}
#endif //FULFIL_CPPUTILS_CONVERSIONS_H
