#include "WheelFlatSpotUtils.h"

#include "neokon-shared/TogglerState.h"
#include "neokon-shared/UuidFromBytes.h"
#include "neokon-shared/UuidToBytes.h"

proto::VehicleFlatSpotInfo subfaultToWheelFlatSpotMsg(proto::Subfault const& subfault)
{
    proto::VehicleFlatSpotInfo flatSpotMsg;

    assert(isSubfaultHasWheelFlatSpotParameters(subfault));

    flatSpotMsg.set_train_uuid_bytes(subfault.train_uuid_bytes());
    flatSpotMsg.set_vehicle_uuid_bytes(subfault.vehicle_uuid_bytes());
    flatSpotMsg.set_vehicle_endpoint(subfault.bogie_endpoint());
    flatSpotMsg.set_switch_command(boolToProtoTogglerState(subfault.bool_value()));

    return flatSpotMsg;
}

bool isSubfaultHasWheelFlatSpotParameters(proto::Subfault const& subfault)
{
    return  subfault.has_train_uuid_bytes() &&
            subfault.has_vehicle_uuid_bytes() &&
            subfault.has_bogie_endpoint() &&
            subfault.has_bool_value();
}

bool isWheelFlatSpotSubfault(boost::uuids::uuid const& trainUuid,
    boost::uuids::uuid const& vehicleUuid, VehicleEndpoint vehicleEndpoint,
    proto::Subfault const& subfault)
{
    assert(isSubfaultHasWheelFlatSpotParameters(subfault));

    return uuidFromBytes(subfault.train_uuid_bytes()) == trainUuid &&
        uuidFromBytes(subfault.vehicle_uuid_bytes()) == vehicleUuid &&
        endpointFromProto(subfault.bogie_endpoint()) == vehicleEndpoint;
}

proto::Subfault createWheelFlatSpotSubfault(boost::uuids::uuid const& parentUuid,
    boost::uuids::uuid const& subfaultUuid, boost::uuids::uuid const& trainUuid,
    boost::uuids::uuid const& vehicleUuid, VehicleEndpoint vehicleEndpoint, bool value)
{
    proto::Subfault propoSubfault;
    propoSubfault.set_train_uuid_bytes(uuidToBytes(trainUuid));
    propoSubfault.set_vehicle_uuid_bytes(uuidToBytes(vehicleUuid));
    propoSubfault.set_bogie_endpoint(protoFromEndpoint(vehicleEndpoint));
    propoSubfault.set_bool_value(value);
    propoSubfault.set_parent_uuid_bytes(uuidToBytes(parentUuid));
    propoSubfault.set_uuid_bytes(uuidToBytes(subfaultUuid));

    return propoSubfault;
}
