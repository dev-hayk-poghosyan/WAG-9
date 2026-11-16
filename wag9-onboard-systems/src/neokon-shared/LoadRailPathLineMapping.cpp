#include "LoadRailPathLineMapping.h"

#include "JsonStreamToProtoMessage.h"
#include "RouteSet.pb.h"
#include "UuidFromString.h"

#include <boost/format.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <fstream>

std::unordered_map<boost::uuids::uuid, unsigned> loadRailPathLineMapping(
    std::filesystem::path const& routeSetFilePath, const RailwayLines& railwayLines)
{
    std::unordered_map<boost::uuids::uuid, unsigned> result;

    std::ifstream strm(routeSetFilePath);
    if (!strm)
    {
        return result;
    }

    boost::property_tree::ptree mappingTree;
    boost::property_tree::read_json(strm, mappingTree);

    const auto& lines = railwayLines.lines();
    const std::unordered_map<std::string, unsigned> lineIndexById =
        railwayLines.createIndexByIdRepository();

    const auto isNumber = [](const std::string & s)
    {
        return !s.empty() && std::find_if(s.begin(), s.end(),
            [](unsigned char c) { return !std::isdigit(c); }) == s.end();
    };

    for (auto const& value : mappingTree.get_child("railPathUuidToLineIndex"))
    {
        const auto& lineId = value.second.get_value<std::string>();
        if (isNumber(lineId))
        {
            const auto index = std::atoi(lineId.c_str());
            if (index >= railwayLines.lines().size())
            {
                throw std::runtime_error("Line index exceeds maximum possible index " +
                    lineId + ":" + value.first);
            }

            result.emplace(uuidFromString(value.first), index);
        }
        else
        {
            auto it = lineIndexById.find(lineId);
            if (it != lineIndexById.end())
            {
                result.emplace(uuidFromString(value.first), it->second);
            }
            else
            {
                throw std::runtime_error("Unknown line Id " + lineId + ":" + value.first);
            }
        }
    }

    return result;
}
