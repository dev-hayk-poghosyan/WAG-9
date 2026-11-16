#include "neokon-shared/RadioCommunicationEndpoint.h"
#include <cassert>

const char*
radioCommunicationEndpointToString(RadioCommunicationEndpoint endpoint)
{
    if (endpoint == RadioCommunicationEndpoint::In)
        return "in";

    if (endpoint == RadioCommunicationEndpoint::Out)
        return "out";

    assert(false && "Unknown RadioCommunicationEndpoint");
    return nullptr;
}
