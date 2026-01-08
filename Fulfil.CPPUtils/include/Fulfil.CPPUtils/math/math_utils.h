#pragma once
#include <vector>
#include <type_traits>
#include <algorithm>

/**
 * Collection of small math-related utility functions.
 */
namespace math_util
{
    /**
     * Computes the arithmetic mean of a vector of numbers.
     *
     * @param values  Input values
     * @return        Mean of the values, or T{} if the vector is empty
     */
    template <typename T>
    inline T mean(const std::vector<T>& values)
    {
        static_assert(std::is_floating_point<T>::value,
                      "math_util::mean<T> requires T to be a floating-point type (float/double/long double).");

        if (values.empty()) return T{};

        const auto sum = std::accumulate(values.begin(), values.end(), 0.0L,
        [](long double acc, T v) { return acc + static_cast<long double>(v); });

        return static_cast<T>(sum / static_cast<long double>(values.size()));
    }
}