#include "LoadJsonFileToProtoMessage.h"

#include "JsonStreamToProtoMessage.h"

#include <boost/format.hpp>

#include <fstream>
#include <stdexcept>

bool
tryLoadJsonFileToProtoMessage(std::filesystem::path const& filePath, google::protobuf::Message& msg)
{
    std::ifstream strm(filePath, std::ios::binary);
    if (!strm)
    {
        return false;
    }

    try
    {
        jsonStreamToProtoMessage(strm, msg);
    }
    catch (std::exception const& e)
    {
        LOG_ERROR(
            str(boost::format("Error reading JSON file \"%1%\" to proto message: %2%") %
                filePath.u8string() % e.what()));
        return false;
    }
    return true;
}

void
loadJsonFileToProtoMessage(std::filesystem::path const& filePath, google::protobuf::Message& msg)
{
    bool res = tryLoadJsonFileToProtoMessage(filePath, msg);

    if (!res)
    {
        throw std::runtime_error(
            str(boost::format("Failed to load file \"%1%\"") % filePath.u8string()));
    }
}
