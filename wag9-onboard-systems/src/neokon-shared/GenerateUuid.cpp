#include "GenerateUuid.h"

#include <boost/uuid/uuid_generators.hpp>

boost::uuids::uuid generateUuid()
{
    thread_local boost::uuids::random_generator gen;
    return gen();
}
