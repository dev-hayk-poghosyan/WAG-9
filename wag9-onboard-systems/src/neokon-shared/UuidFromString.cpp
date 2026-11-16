#include "UuidFromString.h"

#include <boost/uuid/string_generator.hpp>

#include <sstream>
#include <stdexcept>

boost::uuids::uuid
uuidFromString(std::string_view str)
{
    boost::uuids::uuid result;

    try
    {
        result = boost::uuids::string_generator()(str.begin(), str.end());
    }
    catch (std::exception const&)
    {
        // Boost just throws std::runtime_error("invalid uuid string"),
        // without specifiying which string was invalud.
        std::ostringstream strm;
        strm << "Invalid UUID: '" << str << '\'';
        throw std::runtime_error(strm.str());
    }

    return result;
}
