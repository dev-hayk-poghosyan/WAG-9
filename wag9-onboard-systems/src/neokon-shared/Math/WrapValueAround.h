#pragma once

#include <climits>

inline double
wrapValueAround(double value, double min, double max)
{
    if (value > max)
    {
        return min + std::abs(value - max);
    }
    if (value < min)
    {
        return max - std::abs(value - min);
    }
    return value;
}
