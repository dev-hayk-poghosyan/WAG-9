#pragma once

#include "CommonCabinSignalColor.h"

/**
 * The information from SignalState related to the Russian (and similar)
 * overground signalling system.
 */
class SignalStateAspectRu
{
public:
    virtual ~SignalStateAspectRu() = default;

    virtual CommonCabinSignalColor commonCabinSignalColor() const = 0;
};
