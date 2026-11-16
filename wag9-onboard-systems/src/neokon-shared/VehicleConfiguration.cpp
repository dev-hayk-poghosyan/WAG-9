#include "VehicleConfiguration.h"

#include "UuidToBytes.h"

VehicleConfiguration::VehicleConfiguration(
    std::string const& vehicleId,
    proto::VehicleProperties const& properties,
    bool reversed,
    bool brakeAirPresent)
    : m_vehicleId(vehicleId)
    , m_properties(properties)
    , m_reversed(reversed)
    , m_brakeAirPresent(brakeAirPresent)
{
}

proto::VehicleInstance
VehicleConfiguration::createProtoVehicleInstance(boost::uuids::uuid const& instanceUuid) const
{
    proto::VehicleInstance result;

    result.set_instance_uuid_bytes(uuidToBytes(instanceUuid));
    result.set_vehicle_id(m_vehicleId);
    *result.mutable_properties() = properties();
    result.set_reversed(m_reversed);
    result.set_brake_air_present(m_brakeAirPresent);
    result.set_node_path(m_nodePath);

    return result;
}

std::string const&
VehicleConfiguration::vehicleId() const
{
    return m_vehicleId;
}

std::optional<double>
VehicleConfiguration::payloadWeightT() const
{
    return m_properties.has_payload_weight()
        ? m_properties.payload_weight().payload_weight_t()
        : std::optional<double>();
}

std::optional<double>
VehicleConfiguration::maxPayloadWeightT() const
{
    return m_properties.has_payload_weight()
        ? m_properties.payload_weight().max_load_weight_t()
        : std::optional<double>();
}

void
VehicleConfiguration::setPayloadWeightT(double weightT)
{
    if (m_properties.has_payload_weight())
    {
        float const clampedWeightT = std::clamp(static_cast<float>(weightT),
            0.f, m_properties.mutable_payload_weight()->max_load_weight_t());
        m_properties.mutable_payload_weight()->set_payload_weight_t(clampedWeightT);

        return;
    }

    assert(!"Cannot set payload for this vehicle");
}

bool
VehicleConfiguration::reversed() const
{
    return m_reversed;
}

void
VehicleConfiguration::setReversed(bool val)
{
    m_reversed = val;
}

bool
VehicleConfiguration::brakeAirPresent() const
{
    return m_brakeAirPresent;
}

void
VehicleConfiguration::setBrakeAirPresent(bool present)
{
    m_brakeAirPresent = present;
}

std::string const&
VehicleConfiguration::nodePath() const
{
    return m_nodePath;
}

void
VehicleConfiguration::setNodePath(std::string const& nodePath)
{
    m_nodePath = nodePath;
}

void
VehicleConfiguration::setProperties(proto::VehicleProperties const& properties)
{
    m_properties = properties;
}

proto::VehicleProperties const&
VehicleConfiguration::properties() const
{
    return m_properties;
}

std::string
VehicleConfiguration::vehicleNumber() const
{
    return m_properties.has_number()
        ? m_properties.number()
        : std::string();
}

void
VehicleConfiguration::setVehicleNumber(std::string const& vehicleNumber)
{
    *m_properties.mutable_number() = vehicleNumber;
}
