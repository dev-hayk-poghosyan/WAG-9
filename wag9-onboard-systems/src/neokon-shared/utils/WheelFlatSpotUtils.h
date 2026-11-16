#pragma once

#include "Subfault.pb.h"
#include "VehicleFlatSpotInfo.pb.h"

#include "neokon-shared/LeftRightSide.h"
#include "neokon-shared/VehicleEndpoint.h"

#include <boost/uuid/uuid.hpp>

proto::VehicleFlatSpotInfo subfaultToWheelFlatSpotMsg(proto::Subfault const& subfault);

/**
 * @returns true if @p subfault can be converted to proto::VehicleFlatSpotInfo.
 */
bool isSubfaultHasWheelFlatSpotParameters(proto::Subfault const& subfault);

/**
 * @returns true if @p subfault contains values specified by given parameters.
 */
bool isWheelFlatSpotSubfault(boost::uuids::uuid const& trainUuid,
    boost::uuids::uuid const& vehicleUuid, VehicleEndpoint vehicleEndpoint,
    proto::Subfault const& subfault);

/**
 * Creates new subfault with given parameters.
 */
proto::Subfault createWheelFlatSpotSubfault(boost::uuids::uuid const& parentUuid,
    boost::uuids::uuid const& subfaultUuid, boost::uuids::uuid const& trainUuid,
    boost::uuids::uuid const& vehicleUuid, VehicleEndpoint vehicleEndpoint, bool value);
