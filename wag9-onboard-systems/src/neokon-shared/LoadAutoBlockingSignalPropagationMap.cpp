#include "LoadAutoBlockingSignalPropagationMap.h"

#include "JsonStreamToProtoMessage.h"
#include "AutoBlockingSignalPropagationMap.pb.h"
#include "UuidFromString.h"

#include <boost/format.hpp>

#include <fstream>

std::unordered_map<boost::uuids::uuid, IntervalPropertyMap<AutoBlockingSignalPropagation>>
loadAutoBlockingSignalPropagationMap(std::filesystem::path const& propagationMapJsonFile)
{
    std::unordered_map<boost::uuids::uuid,
        IntervalPropertyMap<AutoBlockingSignalPropagation>> result;

    std::ifstream strm(propagationMapJsonFile);
    if (!strm)
        return result;

    proto::AutoBlockingSignalPropagationMap propagationMap;
    jsonStreamToProtoMessage(strm, propagationMap);

    result.reserve(propagationMap.rail_paths_size());

    try
    {
        for (auto const& railPathIntervals : propagationMap.rail_paths())
        {
            IntervalPropertyMap<AutoBlockingSignalPropagation> intervalPropertyMap;

            for (auto const& interval : railPathIntervals.intervals())
            {
                if (interval.start_distance() >= interval.stop_distance())
                    continue;

                intervalPropertyMap.setPropertyOnInterval(
                    autoBlockingSignalPropagationFromProto(interval.propagation()),
                    interval.start_distance(), interval.stop_distance());
            }

            result.emplace(uuidFromString(railPathIntervals.uuid_bytes()), intervalPropertyMap);
        }
    }
    catch (std::exception const& e)
    {
        throw std::logic_error(str(boost::format(
            "[%1%] %2%") % propagationMapJsonFile.u8string() % e.what()));
    }

    return result;
}
