#pragma once

/**
 * The information from SignalState about a Russian (or similar) Entrance signal.
 */
class SignalStateAspectRuEntrance
{
public:
    enum class RepeaterState
    {
        Off,
        Green,
        Yellow,
        YellowFlash,
    };

    virtual ~SignalStateAspectRuEntrance() = default;

    /**
     * The state for the repeaters of this signal.
     */
    virtual RepeaterState repeaterState() const = 0;
};
