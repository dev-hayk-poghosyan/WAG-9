#include "LoadSpeedLimitMap.h"

#include "JsonStreamToProtoMessage.h"
#include "SpeedLimitMap.pb.h"
#include "UuidFromString.h"
#include "neokon-shared/ProtobufGlmInterop.h"

#include <boost/format.hpp>

#include <fstream>

WorldSpeedLimitMap
loadSpeedLimitMap(std::filesystem::path const& speedLimitMapJsonFile,
    PointOnRoadResolver const& resolver)
{
    WorldSpeedLimitMap result;

    std::ifstream strm(speedLimitMapJsonFile);
    if (!strm)
        return result;

    proto::SpeedLimitMap speedLimitMap;
    jsonStreamToProtoMessage(strm, speedLimitMap);

    result.speedMap.reserve(speedLimitMap.intervals_size());

    try
    {
        for (auto const& interval : speedLimitMap.intervals())
        {
            static double const resolveMaxDistance = 3.0;

            auto const startData = resolver(protoToGlm(interval.start_point()), resolveMaxDistance);
            if (!startData)
                continue;

            auto const stopData = resolver(protoToGlm(interval.stop_point()), resolveMaxDistance);
            if (!stopData)
                continue;

            auto const [startUuid, startDistance] = *startData;
            auto const [stopUuid, stopDistance] = *stopData;

            if (startUuid != stopUuid && startDistance >= stopDistance)
                continue;

            IntervalPropertyMap<SpeedLimitProperty>& intervalMap = result.speedMap[startUuid];

            intervalMap.setPropertyOnInterval(SpeedLimitProperty::fromProto(
                interval.forward_direction_limits(), interval.backward_direction_limits()),
                startDistance, stopDistance);
        }
    }
    catch (std::exception const& e)
    {
        throw std::logic_error(str(boost::format(
            "[%1%] %2%") % speedLimitMapJsonFile.u8string() % e.what()));
    }

    return result;
}
