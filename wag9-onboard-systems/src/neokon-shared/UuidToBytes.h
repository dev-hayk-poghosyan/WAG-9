#pragma once

#include <boost/uuid/uuid.hpp>

#include <string>

inline std::string uuidToBytes(boost::uuids::uuid const& uuid)
{
    return std::string(uuid.begin(), uuid.end());
}
