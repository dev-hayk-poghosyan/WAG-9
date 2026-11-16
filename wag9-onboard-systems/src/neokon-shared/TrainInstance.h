#pragma once

#include "TrainFeatures.h"
#include "TrainInstance.pb.h"
#include "VehicleInstance.h"

#include <boost/uuid/uuid.hpp>

#include <vector>

/**
 * TrainInstance represents the train on the scene. On SMC, it could be created from the scenario
 * train or generated based on the message from the TrainSim.
 */
class TrainInstance
{
public:
    static std::shared_ptr<TrainInstance> fromProtoMessage(proto::TrainInstance const& protoMessage);

    TrainInstance(TrainFeatures const& trainFeatures,
        std::vector<std::shared_ptr<VehicleInstance>> const& vehicles);

    void setVehicles(std::vector<std::shared_ptr<VehicleInstance>> const& newVehicles);

    void removeVehicleInstance(boost::uuids::uuid const& vehicleInstanceUuid);

    const TrainFeatures& trainFeatures() const { return m_trainFeatures; }

    const std::vector<std::shared_ptr<VehicleInstance>>& vehicles() const { return m_vehicles; }

private:
    TrainFeatures m_trainFeatures;

    std::vector<std::shared_ptr<VehicleInstance>> m_vehicles;
};
