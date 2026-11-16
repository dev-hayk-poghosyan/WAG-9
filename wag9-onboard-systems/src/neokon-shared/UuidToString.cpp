#include "UuidToString.h"

#include <boost/uuid/uuid_io.hpp>

std::string 
uuidToString(boost::uuids::uuid const& uuid)
{
    return boost::uuids::to_string(uuid);
}
