#include "RadioCommunicationLine.h"
#include <cassert>

const char*
radioCommunicationLineToString(RadioCommunicationLine line)
{
    if (line == RadioCommunicationLine::Gsmr)
        return "gsmr";
    if (line == RadioCommunicationLine::Train)
        return "train";
    if (line == RadioCommunicationLine::Direct)
        return "direct";

    assert(false && "Unknown RadioCommunicationLine");
    return nullptr;
}
