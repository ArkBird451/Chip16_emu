#include "Memory.h"
#include <cstring> 

Memory::Memory() {
    reset();
}

// Read a single byte from memory
uint8_t Memory::readByte(uint16_t address) const {
    return ram[address];
}

// Read a 16-bit word from memory
uint16_t Memory::readWord(uint16_t address) const {
    uint8_t low = ram[address];
    uint8_t high = ram[address + 1];
    return (high << 8) | low;
}

// Write a single byte to memory
void Memory::writeByte(uint16_t address, uint8_t value) {
    ram[address] = value;
}

// Write a 16-bit word to memory
void Memory::writeWord(uint16_t address, uint16_t value) {
    ram[address] = value & 0xFF;          // Low byte 
    ram[address + 1] = (value >> 8) & 0xFF; // High byte
}

// Load ROM data into memory starting at specified address
void Memory::loadROM(const uint8_t* data, size_t size, uint16_t startAddr) {
    // Ensure we don't write past the end of memory
    if (startAddr + size > MEMORY_SIZE) {
        size = MEMORY_SIZE - startAddr;
    }
    
    // Copy ROM data into RAM
    memcpy(&ram[startAddr], data, size);
}

// Reset memory
void Memory::reset() {
    memset(ram, 0, MEMORY_SIZE);
}
