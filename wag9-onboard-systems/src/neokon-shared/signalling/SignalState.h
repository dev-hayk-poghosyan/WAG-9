#pragma once

#include "../PlusEnum.h"

#include <boost/optional.hpp>

#include <memory>
#include <string>

class SignalStateAspectBot;
class SignalStateAspectObstacle;
class SignalStateAspectRu;
class SignalStateAspectRuEntrance;
class SignalStateAspectRuMetro;

/**
 * Interface.
 * An instance of this class represents the full state of a signal.
 *
 * The full state can be updated by a partial one. That way, only some aspects of the full state
 * may be updated. A partial state can be serialized to a string and deserialized from one.
 * Most (all?) of the sub-classes have a PartialState inner class. The base class does not
 * deal with those directly, but it does deal with their serialized versions.
 */
class SignalState
{
public:
    virtual ~SignalState() = default;

    virtual std::unique_ptr<SignalState> clone() const = 0;

    /**
     * Deserializes the partial state and updates some or all of the fields of this
     * full state.
     *
     * Returns true if changes were made to this state.
     */
    virtual bool mergeSerializedPartialState(std::string const& serialized) = 0;

    virtual SignalStateAspectBot const* botAspect() const { return nullptr; }

    virtual SignalStateAspectObstacle const* obstacleAspect() const { return nullptr; }

    virtual SignalStateAspectRu const* ruAspect() const { return nullptr; }

    virtual SignalStateAspectRuEntrance const* ruEntranceAspect() const { return nullptr; }

    virtual SignalStateAspectRuMetro const* ruMetroAspect() const { return nullptr; }
};
