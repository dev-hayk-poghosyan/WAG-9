#pragma once

#include "RadioAudioEndpoint.pb.h"

enum class RadioCommunicationEndpoint
{
    /**
     * Input endpoint of communication line.
     */
    In,

    /**
     * Output endpoint of communication line.
     */
    Out,
};

inline
RadioCommunicationEndpoint oppositeEndpoint(RadioCommunicationEndpoint endpoint)
{
    return static_cast<RadioCommunicationEndpoint>(1 - static_cast<int>(endpoint));
}

inline RadioCommunicationEndpoint
endpointFromProto(proto::radio::AudioEndpoint::Endpoint endpoint)
{
    return static_cast<RadioCommunicationEndpoint>(endpoint);
}

inline proto::radio::AudioEndpoint::Endpoint
protoFromEndpoint(RadioCommunicationEndpoint endpoint)
{
    return static_cast<proto::radio::AudioEndpoint::Endpoint>(endpoint);
}

const char* radioCommunicationEndpointToString(RadioCommunicationEndpoint line);
