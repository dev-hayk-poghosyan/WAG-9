#pragma once

#include <cstddef>
#include <type_traits>

/**
 * Returns true if the given sequence is a sequence of integer (or enum, or similar) types
 * with values starting from zero and each subsequent element is one greater than the previous one.
 *
 * The use case is checking if iterating over an array of all enum values will be equivalent
 * to iterating from zero to the size of that array (exclusive).
 */
template<typename Sequence>
constexpr bool isIndexSequence(Sequence const& seq)
{
    size_t idx = 0;

    for (auto el : seq)
    {
        static_assert(std::is_integral<decltype(el)>::value || std::is_enum<decltype(el)>::value);
        if (static_cast<size_t>(el) != idx)
        {
            return false;
        }
        ++idx;
    }

    return true;
}
