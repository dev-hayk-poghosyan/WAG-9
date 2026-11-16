#include "UuidFromBytes.h"

#include <cstring>
#include <stdexcept>

boost::uuids::uuid uuidFromBytes(std::string const& bytes)
{
    if (bytes.size() != boost::uuids::uuid::static_size())
    {
        throw std::length_error("Trying to initialize a UUID with the wrong number of bytes");
    }

    boost::uuids::uuid uuid;
    memcpy(uuid.data, bytes.data(), boost::uuids::uuid::static_size());

    return uuid;
}
