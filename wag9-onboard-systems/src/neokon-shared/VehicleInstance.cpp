#include "VehicleInstance.h"
#include "UuidFromBytes.h"

VehicleInstance::VehicleInstance(boost::uuids::uuid const& instanceUuid,
    std::string const& vehicleId,
    proto::VehicleProperties const& properties,
    bool reversed,
    bool brakeAirPresent)
    : VehicleConfiguration(vehicleId, properties, reversed, brakeAirPresent)
    , m_instanceUuid(instanceUuid)
{
}

VehicleInstance::VehicleInstance(proto::VehicleInstance const& protoMessage)
    : VehicleConfiguration(
        protoMessage.vehicle_id(),
        protoMessage.properties(),
        protoMessage.reversed(), protoMessage.brake_air_present())
    , m_instanceUuid(uuidFromBytes(protoMessage.instance_uuid_bytes()))
{
    setNodePath(protoMessage.node_path());
}

proto::VehicleInstance
VehicleInstance::toProtoMessage() const
{
    return createProtoVehicleInstance(m_instanceUuid);
}

boost::uuids::uuid const&
VehicleInstance::instanceUuid() const
{
    return m_instanceUuid;
}
