#pragma once

#include "CommonCabinSignalColor.h"

#include <boost/optional.hpp>

/**
 * The information from SignalState intended to be used by bots.
 */
class SignalStateAspectBot
{
public:
    virtual ~SignalStateAspectBot() = default;

    /**
     * XXX: Bots shouldn't rely on CommonCabinSignalColor, but on something more region-neutral.
     */
    virtual CommonCabinSignalColor commonCabinSignalColor() const = 0;
};
