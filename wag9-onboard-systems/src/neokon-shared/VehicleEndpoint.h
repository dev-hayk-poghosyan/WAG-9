#pragma once

#include "neokon-shared/PlusEnum.h"
#include "VehicleEndpoint.pb.h"

enum class VehicleEndpoint
{
    /**
     * The side of the vehicle that's closer to the head of the train.
     *
     * @see TrainEndpoint::Head
     */
    Head = proto::VehicleEndpoint::HEAD,

    /**
     * The side of the vehicle that's closer to the tail of the train.
     *
     * @see TrainEndpoint::Tail
     */
    Tail = proto::VehicleEndpoint::TAIL
};

inline VehicleEndpoint invertedVehicleEndpoint(VehicleEndpoint value)
{
    static_assert(+VehicleEndpoint::Head == 0);
    static_assert(+VehicleEndpoint::Tail == 1);

    return static_cast<VehicleEndpoint>((+VehicleEndpoint::Tail) - (+value));
}

inline VehicleEndpoint endpointFromProto(proto::VehicleEndpoint::Endpoint endpoint)
{
    return static_cast<VehicleEndpoint>(endpoint);
}

inline proto::VehicleEndpoint::Endpoint protoFromEndpoint(VehicleEndpoint endpoint)
{
    return static_cast<proto::VehicleEndpoint::Endpoint>(endpoint);
}

enum class VehicleEndpoints
{
    None = 0,
    Head = 1 << +VehicleEndpoint::Head,
    Tail = 1 << +VehicleEndpoint::Tail,
    Both = (+Head) | (+Tail)
};

inline bool operator&(VehicleEndpoints mask, VehicleEndpoint endpoint)
{
    static_assert(+VehicleEndpoints::Head == 1 << +VehicleEndpoint::Head);
    static_assert(+VehicleEndpoints::Tail == 1 << +VehicleEndpoint::Tail);

    return (+mask & (1 << +endpoint)) != 0;
}

inline VehicleEndpoints operator|(VehicleEndpoints mask, VehicleEndpoint endpoint)
{
    static_assert(+VehicleEndpoints::Head == 1 << +VehicleEndpoint::Head);
    static_assert(+VehicleEndpoints::Tail == 1 << +VehicleEndpoint::Tail);

    return static_cast<VehicleEndpoints>(+mask | (1 << +endpoint));
}
