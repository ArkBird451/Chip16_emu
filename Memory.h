#pragma once

#include <cstdint>
#include <cstddef>

// Memory constants
#define MEMORY_SIZE 0x10000      // 64KB total memory
#define STACK_START 0xFDF0       // Stack begins here (512 bytes)
#define STACK_END 0xFFEF         // Stack ends here
#define IO_PORT_START 0xFFF0     // I/O ports start (4 bytes)
#define IO_PORT_END 0xFFF3       // I/O ports end

class Memory {
private:
    uint8_t ram[MEMORY_SIZE];  // 64KB of RAM

public:
    Memory();
    
    /**
     * Read a single byte from memory
     */
    uint8_t readByte(uint16_t address) const;
    
    /**
     * Read a 16-bit word from memory
     */
    uint16_t readWord(uint16_t address) const;
    
    /**
     * Write a single byte to memory
     */
    void writeByte(uint16_t address, uint8_t value);
    
    /**
     * Write a 16-bit word to memory 
     */
    void writeWord(uint16_t address, uint16_t value);
    
    /**
     * Load ROM data into memory starting at specified address
     */
    void loadROM(const uint8_t* data, size_t size, uint16_t startAddr = 0x0000);
    
    /**
     * Reset memory - clear all RAM to zero
     */
    void reset();
    
    /**
     * Get direct pointer to RAM
     */
    const uint8_t* getRawMemory() const { return ram; }
};
