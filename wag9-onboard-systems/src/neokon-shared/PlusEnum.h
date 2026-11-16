#pragma once

#include <type_traits>

/**
 * A unary + operator for enums to convert them to their underlying type.
 */
template<typename T>
constexpr std::enable_if_t<std::is_enum<T>::value, typename std::underlying_type_t<T>>
operator+(T val)
{
    return static_cast<std::underlying_type_t<T>>(val);
}
