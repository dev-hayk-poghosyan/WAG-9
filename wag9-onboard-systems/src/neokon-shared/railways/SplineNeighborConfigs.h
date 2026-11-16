#pragma once

#include <string>
#include <optional>
/**
 * MeshGen config file paths for previous and next railway parts for single spline
 * std::nullopt = no connection point for other parts, it can be middle or overlapped spline, 
 * "" = has connection, but nothing is connected
 */
struct SplineNeighborConfigs
{
    std::optional<std::string> previous;
    std::optional<std::string> next;
};

inline bool operator!=(const SplineNeighborConfigs& first, const SplineNeighborConfigs& second)
{
    if (first.previous != second.previous)
    {
        return true;
    }
    if (first.next != second.next)
    {
        return true;
    }
    if ((first.previous != std::nullopt) && (*first.previous != *second.previous))
    {
        return true;
    }
    if ((first.next != std::nullopt) && (*first.next != *second.next))
    {
        return true;
    }
    return false;
}
