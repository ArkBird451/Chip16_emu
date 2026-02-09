#pragma once

#include <cstdint>
#include "Memory.h"

// Flags register bits
#define FLAG_C 0x01  // Carry
#define FLAG_Z 0x02  // Zero
#define FLAG_O 0x04  // Overflow
#define FLAG_N 0x08  // Negative

// CPU Registers: PC, SP (unsigned), R0-RF (signed), FLAGS
struct Registers {
    uint16_t PC;
    uint16_t SP;
    int16_t R[16];
    uint8_t FLAGS;
};

// Instruction format: 4 bytes [opcode][yx][hhll_low][hhll_high]
struct Instruction {
    uint8_t opcode;      // Byte 0
    uint8_t yx;          // Byte 1 (Y=high nibble, X=low nibble)
    uint16_t hhll;       // Bytes 2-3 (little-endian)

    // Get Y register (high nibble)
    uint8_t getY() const {
        return (yx >> 4) & 0x0F;
    }

    // Get X register (low nibble)
    uint8_t getX() const {
        return yx & 0x0F;
    }
};

// Chip16 CPU Emulator
class CPU {
private:
    Registers regs;      // CPU registers (PC, SP, R0-RF, FLAGS)
    Memory& memory;      // Reference to memory
    bool running;        // CPU running state
    uint64_t cycles;     // Cycle counter

public:
    // Constructor
    CPU(Memory& mem);
    
    // Core execution
    void reset(uint16_t startAddress = 0x0000);
    void step();                          // Execute one instruction
    void run(int maxCycles = -1);         // Run until halt or max cycles
    
    // Flag operations
    void setFlag(uint8_t flag, bool value);
    bool getFlag(uint8_t flag) const;
    void updateZeroNegativeFlags(int16_t result);
    
    // Instruction execution
    void execute(const Instruction& inst);
    
    // Debugging
    void dumpState() const;
};
