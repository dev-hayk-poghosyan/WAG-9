#pragma once

#include "RuMetroAlsCode.h"
#include "neokon-shared/signalling/ru-metro/RuMetroAspectState.h"

/**
 * The information from SignalState related to the Russian underground signalling system.
 */
class SignalStateAspectRuMetro
{
public:
    virtual ~SignalStateAspectRuMetro() = default;

    virtual RuMetroAlsCode alsCode() const = 0;

    /**
     * True when the signal's main aspect allows train to drive, i.e. is not undefined
     * and doesn't contain red light.
     * That means that the train might pass even when there is no ALS frequency present.
     */
    virtual bool invitationActive() const = 0;

    virtual bool noFrequency() const = 0;

    virtual bool absoluteStop() const = 0;

    virtual RuMetroAspectState aspectState() const = 0;
};
