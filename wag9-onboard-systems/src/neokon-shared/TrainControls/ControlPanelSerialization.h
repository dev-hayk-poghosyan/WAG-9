#pragma once

#include <cstdint>
#include <vector>
#include <string_view>

namespace packet_io
{
using Packet = std::vector<uint8_t>;

void writeU8(uint8_t* data, size_t offset, int value);
void writeU16BE(uint8_t* data, size_t offset, int value);
void writeU24BE(uint8_t* data, size_t offset, int value);
void writeU32BE(uint8_t* data, size_t offset, uint32_t value);
void writeU64BE(uint8_t* data, size_t offset, uint64_t value);
void writeFloatBE(uint8_t* data, size_t offset, float value);
void writeChar8String(uint8_t* data, size_t offset, std::string_view value);
void writeChar16String(uint8_t* data, size_t offset, std::u16string_view value);

uint8_t parseU8(uint8_t const* data, size_t offset);
uint16_t parseU16BE(uint8_t const* data, size_t offset);
}
