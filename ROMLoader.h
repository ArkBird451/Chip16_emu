#pragma once

#include <cstdint>
#include <string>
#include "Memory.h"

// C16 Header structure (16 bytes)
#pragma pack(push, 1)
struct C16Header {
    char magic[4];        // 'CH16'
    uint8_t reserved;     // 0x00
    uint8_t specVersion;  // Version (0xHL format)
    uint32_t romSize;     // ROM size in bytes (little-endian)
    uint16_t startAddr;   // Start address (little-endian)
    uint32_t crc32;       // CRC32 checksum (not validated)
};
#pragma pack(pop)

// ROM Loader for Chip16
class ROMLoader {
public:
    // Load ROM from file (supports raw binary and C16 format)
    bool loadROM(const std::string& filepath, Memory& memory, uint16_t& startAddr);
};
