#include "TrainInstance.h"

#include "UuidFromBytes.h"
#include "UuidToBytes.h"

std::shared_ptr<TrainInstance>
TrainInstance::fromProtoMessage(proto::TrainInstance const& protoMessage)
{
    TrainFeatures features(protoMessage.train_features());

    std::vector<std::shared_ptr<VehicleInstance>> vehicleInstances;
    for (const auto& protoVehicle : protoMessage.vehicles())
    {
        vehicleInstances.push_back(std::make_shared<VehicleInstance>(protoVehicle));
    }

    return std::make_shared<TrainInstance>(features, vehicleInstances);
}

TrainInstance::TrainInstance(TrainFeatures const& trainFeatures,
    std::vector<std::shared_ptr<VehicleInstance>> const& vehicles)
    : m_trainFeatures(trainFeatures)
    , m_vehicles(vehicles)
{
}

void
TrainInstance::setVehicles(std::vector<std::shared_ptr<VehicleInstance>> const& newVehicles)
{
    m_vehicles = newVehicles;
}

void
TrainInstance::removeVehicleInstance(boost::uuids::uuid const& vehicleInstanceUuid)
{
    auto const it = std::remove_if(m_vehicles.begin(), m_vehicles.end(),[vehicleInstanceUuid](
        auto const& vehicleInstance)
    {
        return vehicleInstance->instanceUuid() == vehicleInstanceUuid;
    });

    assert(it != m_vehicles.end());

    m_vehicles.erase(it);
}
