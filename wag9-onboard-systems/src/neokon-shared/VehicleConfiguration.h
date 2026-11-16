#pragma once

#include "VehicleProperties.pb.h"
#include "VehicleInstance.pb.h"

#include <boost/uuid/uuid.hpp>
#include <optional>

/**
  * Represents a specialized vehicle in the train.
  * Corresponds a <Vehicle>...</Vehicle> in Trains.xml.
  * An exception is a vehicleNumber - it's taken not from Trains.xml but from scenario
  */
class VehicleConfiguration
{
public:
    VehicleConfiguration(
        std::string const& vehicleId,
        proto::VehicleProperties const& properties,
        bool reversed,
        bool brakeAirPresent = true);

    proto::VehicleInstance createProtoVehicleInstance(boost::uuids::uuid const& instanceUuid) const;

    std::string const& vehicleId() const;

    std::optional<double> payloadWeightT() const;
    std::optional<double> maxPayloadWeightT() const;

    /**
     * Undefined behaviour if the term payload is not applicable for this vehicle.
     * It means that for this vehicle a property <Aspect name="PayloadWeight"> not described.
     */
    void setPayloadWeightT(double weightT);

    bool reversed() const;
    void setReversed(bool val);

    bool brakeAirPresent() const;
    void setBrakeAirPresent(bool present);

    std::string const& nodePath() const;
    void setNodePath(std::string const& nodePath);

    void setProperties(proto::VehicleProperties const& properties);
    proto::VehicleProperties const& properties() const;

    std::string vehicleNumber() const;
    void setVehicleNumber(std::string const& vehicleNumber);

private:
    std::string m_vehicleId;
    proto::VehicleProperties m_properties;
    bool m_reversed = false;
    bool m_brakeAirPresent = true;
    std::string m_nodePath;
};
