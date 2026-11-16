#include "Class158VarsToVirtualControlViewer.h"
#include <algorithm>

namespace Class158
{

packet_io::Packet
VirtualViewerIncomingVars::serialize() const
{
    using namespace packet_io;
    constexpr size_t packetSize = sizeof(VirtualViewerIncomingVars);

    Packet packet(packetSize);

    auto* const source = reinterpret_cast<const packet_io::Packet::value_type*>(this);
    packet.assign(source, source + packetSize);

    return packet;
}


bool
VirtualViewerIncomingVars::deserialize(const packet_io::Packet& packet)
{
    if (packet.size() < sizeof(VirtualViewerIncomingVars))
    {
        return false;
    }

    auto* const dest = reinterpret_cast<packet_io::Packet::value_type*>(this);
    auto const* const source = packet.data();

    std::copy(source, source + packet.size(), dest);
    return true;
}

std::vector<uint8_t> DataToDesk::serialize() const
{
    constexpr size_t packetSize = sizeof(DataToDesk);

    std::vector<uint8_t> packet{ packetSize };

    auto* const source = reinterpret_cast<const std::vector<uint8_t>::value_type*>(this);
    packet.assign(source, source + packetSize);

    return packet;
}

bool Controls::deserialize(const packet_io::Packet& packet)
{
    if (packet.size() < sizeof(Controls))
    {
        return false;
    }

    auto* const dest = reinterpret_cast<packet_io::Packet::value_type*>(this);
    auto const* const source = packet.data();

    std::copy(source, source + packet.size(), dest);
    return true;
}

} // namespace Class158
