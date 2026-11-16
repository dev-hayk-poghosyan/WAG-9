#include "neokon-shared/TrainControls/ControlPanelSerialization.h"

#include <algorithm>

namespace packet_io
{

void
writeU8(uint8_t* data, size_t offset, int value)
{
    int const clampedVal = std::clamp(value, 0, 0xff);

    data[offset] = static_cast<uint8_t>(clampedVal);
}

void
writeU16BE(uint8_t* data, size_t offset, int value)
{
    int const clampedVal = std::clamp(value, 0, 0xffff);

    data[offset] = static_cast<uint8_t>(clampedVal >> 8);
    data[offset + 1] = static_cast<uint8_t>(clampedVal);
}

void
writeU24BE(uint8_t* data, size_t offset, int value)
{
    int const clampedVal = std::clamp(value, 0, 0xffffff);

    data[offset] = static_cast<uint8_t>(clampedVal >> 16);
    data[offset + 1] = static_cast<uint8_t>(clampedVal >> 8);
    data[offset + 2] = static_cast<uint8_t>(clampedVal);
}

void
writeU32BE(uint8_t* data, size_t offset, uint32_t value)
{
    for (int i = 0; i < 4; ++i)
    {
        data[offset + i] = static_cast<uint8_t>(value >> (24 - i * 8));
    }
}

void
writeU64BE(uint8_t* data, size_t offset, uint64_t value)
{
    for (int i = 0; i < 8; ++i)
    {
        data[offset + i] = static_cast<uint8_t>(value >> (56 - i * 8));
    }
}

void
writeFloatBE(uint8_t* data, size_t offset, float value)
{
    union
    {
        float fval;
        uint32_t uval;
    };

    fval = value;
    writeU32BE(data, offset, uval);
}

void
writeChar8String(uint8_t* data, size_t offset, std::string_view value)
{
    for (size_t i = 0; i < value.size(); ++i)
    {
        writeU8(data, offset + i, static_cast<int>(value[i]));
    }
}

void
writeChar16String(uint8_t* data, size_t offset, std::u16string_view value)
{
    for (size_t i = 0; i < value.size(); ++i)
    {
        writeU16BE(data, offset + i * 2, static_cast<int>(value[i]));
    }
}

uint8_t
parseU8(uint8_t const* data, size_t offset)
{
    return data[offset];
}

uint16_t
parseU16BE(uint8_t const* data, size_t offset)
{
    return (uint16_t(data[offset]) << 8) | data[offset + 1];
}

}
