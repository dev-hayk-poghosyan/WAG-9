#pragma once

#include "VehicleInstance.pb.h"
#include "VehicleConfiguration.h"

#include <optional>

/**
 * This is a vehicle spawned in the scene and related to exactly one train. Initial parameters are
 * taken from Trains.xml, but the instance UUID is generated or taken from the scenario's train
 * instance.
 */
class VehicleInstance : public VehicleConfiguration
{
public:

    VehicleInstance(
        boost::uuids::uuid const& instanceUuid,
        std::string const& vehicleId,
        proto::VehicleProperties const& properties,
        bool reversed,
        bool brakeAirPresent);

    VehicleInstance(proto::VehicleInstance const& protoMessage);

    proto::VehicleInstance toProtoMessage() const;

    boost::uuids::uuid const& instanceUuid() const;


private:
    boost::uuids::uuid m_instanceUuid;
};
