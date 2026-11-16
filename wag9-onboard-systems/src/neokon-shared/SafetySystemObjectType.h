#pragma once

#include "SafetySystemObject.pb.h"

/**
 * A list of all possible Safety System Object types.
 */
enum class SafetySystemObjectType
{
    AWS         = proto::SafetySystemObject::AWS,
    TPWSTrigger = proto::SafetySystemObject::TPWS_TRIGGER,
    TPWSDouble  = proto::SafetySystemObject::TPWS_DOUBLE,
    SDO         = proto::SafetySystemObject::SDO,
    ADD         = proto::SafetySystemObject::ADD,
    TASSSpeed   = proto::SafetySystemObject::TASS_SPEED,

    /**
     * See proto::SafetySystemObject::TPWS_ARMING for purporse of this.
     */
    TPWSArming  = proto::SafetySystemObject::TPWS_ARMING,

    MetroTrigger = proto::SafetySystemObject::METRO_TRIGGER,
};

inline SafetySystemObjectType safetySystemObjectTypeFromProto(
    proto::SafetySystemObject::Type type)
{
    return static_cast<SafetySystemObjectType>(type);
}

inline proto::SafetySystemObject::Type safetySystemObjectTypeToProto(
    SafetySystemObjectType const& type)
{
    return static_cast<proto::SafetySystemObject::Type>(type);
}
