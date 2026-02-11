#include "CPU.h"
#include "Instructions.h"
#include <iostream>
#include <iomanip>
#include <cstring>

// Constructor
CPU::CPU(Memory& mem) : memory(mem), running(false), cycles(0) {
    reset();
}

// Reset CPU to initial state
void CPU::reset(uint16_t startAddress) {
    regs.PC = startAddress;
    regs.SP = STACK_START;  // 0xFDF0
    regs.FLAGS = 0;
    
    // Clear all general purpose registers
    for (int i = 0; i < 16; i++) {
        regs.R[i] = 0;
    }
    
    running = true;
    cycles = 0;
}

// Fetch instruction from memory at current PC
Instruction CPU::fetch() {
    Instruction inst;
    
    // Read 4 bytes from memory
    inst.opcode = memory.readByte(regs.PC);      // Byte 0: opcode
    inst.yx = memory.readByte(regs.PC + 1);      // Byte 1: yx
    inst.hhll = memory.readWord(regs.PC + 2);    // Bytes 2-3: hhll
    
    return inst;
}

// Set or clear a specific flag
void CPU::setFlag(uint8_t flag, bool value) {
    if (value) {
        regs.FLAGS |= flag;   // Set flag
    } else {
        regs.FLAGS &= ~flag;  // Clear flag
    }
}

// Get the state of a specific flag
bool CPU::getFlag(uint8_t flag) const {
    return (regs.FLAGS & flag) != 0;
}

// Update Zero and Negative flags based on result
void CPU::updateZeroNegativeFlags(int16_t result) {
    setFlag(FLAG_Z, result == 0);        // Zero flag
    setFlag(FLAG_N, result < 0);         // Negative flag
}

// Execute one instruction
void CPU::step() {
    if (!running) return;
    
    // Fetch instruction from memory at PC
    Instruction inst = fetch();
    
    // Execute the instruction
    execute(inst);
    
    // Increment PC by 4 (all instructions are 4 bytes)
    regs.PC += 4;
    
    // Increment cycle counter (all instructions take 1 cycle)
    cycles++;
}

// Run CPU for specified number of cycles (-1 = run until halt)
void CPU::run(int maxCycles) {
    if (maxCycles < 0) {
        // Run until halted
        while (running) {
            step();
        }
    } else {
        // Run for specified cycles
        for (int i = 0; i < maxCycles && running; i++) {
            step();
        }
    }
}

// Execute decoded instruction
void CPU::execute(const Instruction& inst) {
    switch (inst.opcode) {
        // NOP - No operation
        case OP_NOP:
            // Do nothing
            break;
        
        // HLT - Halt execution
        case OP_HLT:
            running = false;
            break;
        
        // MOV RX, RY - Copy register to register
        case OP_MOV: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            regs.R[x] = regs.R[y];
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // LDI RX, HHLL - Load immediate value into register
        case OP_LDI: {
            uint8_t x = inst.getX();
            regs.R[x] = (int16_t)inst.hhll;
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // ADD RX, RY - Add register to register
        case OP_ADD_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int16_t a = regs.R[x];
            int16_t b = regs.R[y];
            int32_t result = (int32_t)a + (int32_t)b;
            regs.R[x] = (int16_t)result;
            
            // Set flags
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));  // Carry
            setFlag(FLAG_O, ((a > 0 && b > 0 && regs.R[x] < 0) || 
                             (a < 0 && b < 0 && regs.R[x] > 0)));     // Overflow
            break;
        }
        
        // ADD RX, HHLL - Add immediate to register
        case OP_ADD_R_IMM: {
            uint8_t x = inst.getX();
            int16_t a = regs.R[x];
            int16_t b = (int16_t)inst.hhll;
            int32_t result = (int32_t)a + (int32_t)b;
            regs.R[x] = (int16_t)result;
            
            // Set flags
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));
            setFlag(FLAG_O, ((a > 0 && b > 0 && regs.R[x] < 0) || 
                             (a < 0 && b < 0 && regs.R[x] > 0)));
            break;
        }
        
        // SUB RX, RY - Subtract register from register
        case OP_SUB_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int16_t a = regs.R[x];
            int16_t b = regs.R[y];
            int32_t result = (int32_t)a - (int32_t)b;
            regs.R[x] = (int16_t)result;
            
            // Set flags
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));  // Borrow
            setFlag(FLAG_O, ((a > 0 && b < 0 && regs.R[x] < 0) || 
                             (a < 0 && b > 0 && regs.R[x] > 0)));     // Overflow
            break;
        }
        
        // SUB RX, HHLL - Subtract immediate from register
        case OP_SUB_R_IMM: {
            uint8_t x = inst.getX();
            int16_t a = regs.R[x];
            int16_t b = (int16_t)inst.hhll;
            int32_t result = (int32_t)a - (int32_t)b;
            regs.R[x] = (int16_t)result;
            
            // Set flags
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));
            setFlag(FLAG_O, ((a > 0 && b < 0 && regs.R[x] < 0) || 
                             (a < 0 && b > 0 && regs.R[x] > 0)));
            break;
        }
        
        // AND RX, RY - Bitwise AND register with register
        case OP_AND_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            regs.R[x] = regs.R[x] & regs.R[y];
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // AND RX, HHLL - Bitwise AND register with immediate
        case OP_AND_R_IMM: {
            uint8_t x = inst.getX();
            regs.R[x] = regs.R[x] & (int16_t)inst.hhll;
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // OR RX, RY - Bitwise OR register with register
        case OP_OR_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            regs.R[x] = regs.R[x] | regs.R[y];
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // OR RX, HHLL - Bitwise OR register with immediate
        case OP_OR_R_IMM: {
            uint8_t x = inst.getX();
            regs.R[x] = regs.R[x] | (int16_t)inst.hhll;
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // XOR RX, RY - Bitwise XOR register with register
        case OP_XOR_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            regs.R[x] = regs.R[x] ^ regs.R[y];
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // XOR RX, HHLL - Bitwise XOR register with immediate
        case OP_XOR_R_IMM: {
            uint8_t x = inst.getX();
            regs.R[x] = regs.R[x] ^ (int16_t)inst.hhll;
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // NOT RX - Bitwise NOT (invert all bits)
        case OP_NOT_R: {
            uint8_t x = inst.getX();
            regs.R[x] = ~regs.R[x];
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // MUL RX, RY - Multiply register by register
        case OP_MUL_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int32_t result = (int32_t)regs.R[x] * (int32_t)regs.R[y];
            regs.R[x] = (int16_t)result;
            
            // Set flags
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));
            break;
        }
        
        // MUL RX, HHLL - Multiply register by immediate
        case OP_MUL_R_IMM: {
            uint8_t x = inst.getX();
            int32_t result = (int32_t)regs.R[x] * (int32_t)(int16_t)inst.hhll;
            regs.R[x] = (int16_t)result;
            
            // Set flags
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));
            break;
        }
        
        // DIV RX, RY - Divide register by register
        case OP_DIV_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            
            if (regs.R[y] == 0) {
                // Division by zero
                setFlag(FLAG_C, true);
                regs.R[x] = 0;
            } else {
                regs.R[x] = regs.R[x] / regs.R[y];
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // DIV RX, HHLL - Divide register by immediate
        case OP_DIV_R_IMM: {
            uint8_t x = inst.getX();
            int16_t divisor = (int16_t)inst.hhll;
            
            if (divisor == 0) {
                // Division by zero
                setFlag(FLAG_C, true);
                regs.R[x] = 0;
            } else {
                regs.R[x] = regs.R[x] / divisor;
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // MOD RX, RY - Modulo register by register
        case OP_MOD_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            
            if (regs.R[y] == 0) {
                // Modulo by zero
                setFlag(FLAG_C, true);
                regs.R[x] = 0;
            } else {
                regs.R[x] = regs.R[x] % regs.R[y];
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // MOD RX, HHLL - Modulo register by immediate
        case OP_MOD_R_IMM: {
            uint8_t x = inst.getX();
            int16_t divisor = (int16_t)inst.hhll;
            
            if (divisor == 0) {
                // Modulo by zero
                setFlag(FLAG_C, true);
                regs.R[x] = 0;
            } else {
                regs.R[x] = regs.R[x] % divisor;
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // Unknown opcode
        default:
            std::cerr << "Unknown opcode: 0x" << std::hex << (int)inst.opcode << std::endl;
            running = false;
            break;
    }
}

// Dump CPU state for debugging
void CPU::dumpState() const {
    std::cout << "=== CPU State ===" << std::endl;
    std::cout << std::hex << std::uppercase;
    
    // Program Counter and Stack Pointer
    std::cout << "PC: 0x" << std::setw(4) << std::setfill('0') << regs.PC << "  ";
    std::cout << "SP: 0x" << std::setw(4) << std::setfill('0') << regs.SP << std::endl;
    
    // Flags
    std::cout << "FLAGS: 0x" << std::setw(2) << std::setfill('0') << (int)regs.FLAGS << " [";
    std::cout << (regs.FLAGS & FLAG_C ? "C" : "-");
    std::cout << (regs.FLAGS & FLAG_Z ? "Z" : "-");
    std::cout << (regs.FLAGS & FLAG_O ? "O" : "-");
    std::cout << (regs.FLAGS & FLAG_N ? "N" : "-");
    std::cout << "]" << std::endl;
    
    // General purpose registers (4 per line)
    for (int i = 0; i < 16; i++) {
        if (i % 4 == 0) std::cout << std::endl;
        std::cout << "R" << std::hex << i << ": 0x" 
                  << std::setw(4) << std::setfill('0') << (uint16_t)regs.R[i] << "  ";
    }
    
    // Cycle count
    std::cout << std::dec << std::endl << std::endl;
    std::cout << "Cycles: " << cycles << std::endl;
    std::cout << "Running: " << (running ? "Yes" : "No") << std::endl;
}
