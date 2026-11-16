#include "LoadProtobufToMsg.h"

void loadProtobufToMsg(std::filesystem::path const& filePath, google::protobuf::Message& msg)
{
    std::ifstream fileStrm(filePath, std::ios_base::binary);
    if (!fileStrm)
    {
        throw std::logic_error(str(
            boost::format("Can not open \"%1%\" for reading") % filePath.u8string()));
    }
    if (!msg.ParseFromIstream(&fileStrm))
    {
        throw std::logic_error(str(
            boost::format("Couldn't read \"%1%\" file") % filePath.u8string()));
    }
}
