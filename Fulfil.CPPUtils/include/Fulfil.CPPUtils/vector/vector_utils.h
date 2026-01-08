#pragma once
#include <vector>
#include <algorithm>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <string>

using std::vector;
using std::pair;

namespace vector_util
{
    template<class T1, class F>
    /**
     * Applies a transformation function to each element of the input vector
     * and returns a new vector containing the transformed values.
     *
     * The callable is invoked once per element and its return type determines
     * the element type of the resulting vector.
     *
     * @tparam T1  Element type of the input vector
     * @tparam F   Callable type accepting a T1 and returning any type
     * @param values    Input vector whose elements are transformed
     * @param selector    Callable applied to each element (perfectly forwarded)
     * @return     Vector containing the results of applying f to each element of v
     */
    auto map(const vector<T1>& values, F&& selector)
    {
        using T2 = std::invoke_result_t<F, const T1&>;
        std::vector<T2> retVar;
        retVar.reserve(values.size());
        for(const T1& v: values)
        {
            retVar.push_back(std::forward<F>(selector)(v));
        }
        return retVar;
    }

    template<class T1, class P>
    /**
     * Filters the input vector using a predicate and returns a new vector
     * containing only the elements for which the predicate returns true.
     *
     * @tparam T1  Element type of the input vector
     * @tparam P   Predicate type accepting a const T1& and returning bool-like
     * @param values     Input vector to filter
     * @param predicate  Predicate applied to each element (perfectly forwarded)
     * @return           Vector containing all elements that match the predicate
     */
    auto filter(const vector<T1>& values, P&& predicate)
    {
        std::vector<T1> out;
        std::copy_if(values.begin(), values.end(), std::back_inserter(out), std::move(predicate));
        return out;
    }

    template <class T1, class T2>
    /**
     * Combines two input vectors element-wise into a vector of pairs.
     *
     * Each pair contains elements from the same index in the left and right
     * vectors. Both input vectors must have the same length; otherwise an
     * exception is thrown.
     *
     * @tparam T1  Element type of the left input vector
     * @tparam T2  Element type of the right input vector
     * @param left   First input vector
     * @param right  Second input vector
     * @return       Vector of pairs formed by combining corresponding elements
     * @throws std::invalid_argument if the input vectors have different lengths
     */
    vector<pair<T1,T2>> zip(const vector<T1>& left, const vector<T2>& right)
    {
        vector<pair<T1, T2>> retVar;
        const auto n = left.size();
        if(right.size() != n)     throw std::invalid_argument("zip: vectors must have the same length, got " + std::to_string(n) + " and " + std::to_string(right.size()));
        retVar.reserve(n);
        for(int i = 0; i < n; i++)
        {
            retVar.emplace_back(left[i], right[i]);
        }
        return retVar;
    }

    namespace detail
    {
        struct identity
        {
            template<class T>
            constexpr const T& operator()(const T& v) const noexcept { return v; }
        };
    }

    template<class T1, class F = detail::identity>
    /**
     * Returns the minimum projected value from the input vector.
     *
     * Uses the selector to project each element to a comparable value and returns
     * the smallest projected result. If selector is omitted, the element itself
     * is used (identity projection). The vector must be non-empty.
     *
     * @tparam T1       Element type of the input vector
     * @tparam F        Callable type accepting a const T1& and returning a comparable type (optional)
     * @param values    Input vector to search
     * @param selector  Projection applied to each element (optional; defaults to identity)
     * @return          The minimum value produced by selector over all elements
     * @throws std::invalid_argument if values is empty
     */
    auto min_value(const vector<T1>& values, F&& selector = std::remove_reference_t<F>{})
    {
        if(values.empty()) throw std::invalid_argument("min: values must not be empty");
        auto&& sel = selector;
        return sel(*std::min_element(values.begin(), values.end(), [&](const T1& a, const T1& b){ return sel(a) < sel(b); }));
    }

    template<class T1, class F = detail::identity>
    /**
     * Returns the maximum projected value from the input vector.
     *
     * Uses the selector to project each element to a comparable value and returns
     * the largest projected result. If selector is omitted, the element itself
     * is used (identity projection). The vector must be non-empty.
     *
     * @tparam T1       Element type of the input vector
     * @tparam F        Callable type accepting a const T1& and returning a comparable type (optional)
     * @param values    Input vector to search
     * @param selector  Projection applied to each element (optional; defaults to identity)
     * @return          The maximum value produced by selector over all elements
     * @throws std::invalid_argument if values is empty
     */
    auto max_value(const vector<T1>& values, F&& selector = std::remove_reference_t<F>{})
    {
        if(values.empty()) throw std::invalid_argument("max: values must not be empty");
        auto&& sel = selector;
        return sel(*std::max_element(values.begin(), values.end(), [&](const T1& a, const T1& b){ return sel(a) < sel(b); }));
    }
}
