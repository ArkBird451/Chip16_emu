#include "CPU.h"
#include "Instructions.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <random>

// Constructor
CPU::CPU(Memory& mem) : memory(mem), running(false), cycles(0) {
    // Initialize random number generator with random seed
    std::random_device rd;
    rng.seed(rd());
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
        
        // CMPI RX, HHLL - Compare register with immediate
        case OP_CMPI: {
            uint8_t x = inst.getX();
            int16_t imm = (int16_t)inst.hhll;
            int32_t result = (int32_t)regs.R[x] - (int32_t)imm;
            
            // Set flags based on comparison
            setFlag(FLAG_Z, result == 0);                    // Equal
            setFlag(FLAG_N, (int16_t)result < 0);            // Negative
            setFlag(FLAG_C, (uint16_t)regs.R[x] < (uint16_t)imm);  // Unsigned borrow
            
            // Overflow: (a-b) overflows if signs differ and result sign != a sign
            bool overflow = ((regs.R[x] ^ imm) & (regs.R[x] ^ (int16_t)result)) & 0x8000;
            setFlag(FLAG_O, overflow);
            break;
        }
        
        // CMP RX, RY - Compare two registers
        case OP_CMP: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int32_t result = (int32_t)regs.R[x] - (int32_t)regs.R[y];
            
            // Set flags based on comparison
            setFlag(FLAG_Z, result == 0);                    // Equal
            setFlag(FLAG_N, (int16_t)result < 0);            // Negative
            setFlag(FLAG_C, (uint16_t)regs.R[x] < (uint16_t)regs.R[y]);  // Unsigned borrow
            
            // Overflow: (a-b) overflows if signs differ and result sign != a sign
            bool overflow = ((regs.R[x] ^ regs.R[y]) & (regs.R[x] ^ (int16_t)result)) & 0x8000;
            setFlag(FLAG_O, overflow);
            break;
        }
        
        // TST RX, RY - Test bits (bitwise AND without storing result)
        case OP_TST: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int16_t result = regs.R[x] & regs.R[y];
            
            // Set flags based on result (don't modify registers)
            setFlag(FLAG_Z, result == 0);
            setFlag(FLAG_N, result < 0);
            break;
        }
        
        // NEG RX - Negate (two's complement)
        case OP_NEG: {
            uint8_t x = inst.getX();
            int16_t original = regs.R[x];
            regs.R[x] = -original;
            
            // Set flags
            setFlag(FLAG_Z, regs.R[x] == 0);
            setFlag(FLAG_N, regs.R[x] < 0);
            // Overflow occurs when negating the most negative value (-32768)
            setFlag(FLAG_O, original == -32768);
            break;
        }
        
        // RND RX, HHLL - Random number from 0 to HHLL-1
        case OP_RND: {
            uint8_t x = inst.getX();
            uint16_t max = inst.hhll;
            
            if (max == 0) {
                regs.R[x] = 0;
            } else {
                std::uniform_int_distribution<int16_t> dist(0, max - 1);
                regs.R[x] = dist(rng);
            }
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
        
        // SHL RX, RY - Shift left by register value
        case OP_SHL_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t shift = regs.R[y] & 0x0F;  // Use only lower 4 bits
            
            if (shift > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (16 - shift))) != 0);
                regs.R[x] = regs.R[x] << shift;
            } else {
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // SHL RX, N - Shift left by immediate
        case OP_SHL_R_IMM: {
            uint8_t x = inst.getX();
            uint8_t shift = inst.hhll & 0x0F;  // Use only lower 4 bits
            
            if (shift > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (16 - shift))) != 0);
                regs.R[x] = regs.R[x] << shift;
            } else {
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // SHR RX, RY - Shift right by register value
        case OP_SHR_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t shift = regs.R[y] & 0x0F;
            
            if (shift > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (shift - 1))) != 0);
                regs.R[x] = (uint16_t)regs.R[x] >> shift;  // Logical shift
            } else {
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // SHR RX, N - Shift right by immediate
        case OP_SHR_R_IMM: {
            uint8_t x = inst.getX();
            uint8_t shift = inst.hhll & 0x0F;
            
            if (shift > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (shift - 1))) != 0);
                regs.R[x] = (uint16_t)regs.R[x] >> shift;  // Logical shift
            } else {
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // SAR RX, RY - Shift right by register value
        case OP_SAR_R_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t shift = regs.R[y] & 0x0F;
            
            if (shift > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (shift - 1))) != 0);
                regs.R[x] = regs.R[x] >> shift;  // Arithmetic shift
            } else {
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // SAR RX, N - Shift right by immediate
        case OP_SAR_R_IMM: {
            uint8_t x = inst.getX();
            uint8_t shift = inst.hhll & 0x0F;
            
            if (shift > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (shift - 1))) != 0);
                regs.R[x] = regs.R[x] >> shift;  // Arithmetic shift (sign-preserving)
            } else {
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // LDM RX, HHLL - Load word from memory address into register
        case OP_LDM_ADDR: {
            uint8_t x = inst.getX();
            uint16_t address = inst.hhll;
            regs.R[x] = (int16_t)memory.readWord(address);
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // LDM RX, RY - Load word from memory address in RY into register
        case OP_LDM_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint16_t address = (uint16_t)regs.R[y];
            regs.R[x] = (int16_t)memory.readWord(address);
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // STM HHLL, RX - Store register word to memory address
        case OP_STM_ADDR: {
            uint8_t x = inst.getX();
            uint16_t address = inst.hhll;
            memory.writeWord(address, (uint16_t)regs.R[x]);
            break;
        }
        
        // STM RX, RY - Store RY to memory address in RX
        case OP_STM_R: {
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint16_t address = (uint16_t)regs.R[x];
            memory.writeWord(address, (uint16_t)regs.R[y]);
            break;
        }
        
        // PUSH RX - Push register onto stack
        case OP_PUSH: {
            uint8_t x = inst.getX();
            regs.SP -= 2;  // Stack grows downward
            memory.writeWord(regs.SP, (uint16_t)regs.R[x]);
            break;
        }
        
        // POP RX - Pop from stack into register
        case OP_POP: {
            uint8_t x = inst.getX();
            regs.R[x] = (int16_t)memory.readWord(regs.SP);
            regs.SP += 2;  // Move stack pointer up
            break;
        }
        
        // PUSHALL - Push all 16 registers onto stack
        case OP_PUSHALL: {
            for (int i = 0; i < 16; i++) {
                regs.SP -= 2;
                memory.writeWord(regs.SP, (uint16_t)regs.R[i]);
            }
            break;
        }
        
        // POPALL - Pop all 16 registers from stack
        case OP_POPALL: {
            for (int i = 15; i >= 0; i--) {  // Pop in reverse order
                regs.R[i] = (int16_t)memory.readWord(regs.SP);
                regs.SP += 2;
            }
            break;
        }
        
        // PUSHF - Push FLAGS register onto stack
        case OP_PUSHF: {
            regs.SP -= 2;
            memory.writeWord(regs.SP, (uint16_t)regs.FLAGS);
            break;
        }
        
        // POPF - Pop FLAGS register from stack
        case OP_POPF: {
            regs.FLAGS = (uint8_t)memory.readWord(regs.SP);
            regs.SP += 2;
            break;
        }
        
        // JMP HHLL - Jump to address
        case OP_JMP: {
            regs.PC = inst.hhll - 4;  // -4 because step() adds 4
            break;
        }
        
        // JMC HHLL - Jump relative
        case OP_JMC: {
            int16_t offset = (int16_t)inst.hhll;
            regs.PC = regs.PC + offset;
            break;
        }
        
        // JX - Jump to address in register RX
        case OP_JX: {
            uint8_t x = inst.getX();
            regs.PC = (uint16_t)regs.R[x] - 4;
            break;
        }
        
        // JZ HHLL - Jump if zero flag is set
        case OP_JZ: {
            if (getFlag(FLAG_Z)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JC HHLL - Jump if carry flag is set
        case OP_JC: {
            if (getFlag(FLAG_C)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JN HHLL - Jump if negative flag is set
        case OP_JN: {
            if (getFlag(FLAG_N)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JNZ HHLL - Jump if zero flag is clear
        case OP_JNZ: {
            if (!getFlag(FLAG_Z)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JNC HHLL - Jump if carry flag is clear
        case OP_JNC: {
            if (!getFlag(FLAG_C)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JNN HHLL - Jump if negative flag is clear
        case OP_JNN: {
            if (!getFlag(FLAG_N)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JO HHLL - Jump if overflow flag is set
        case OP_JO: {
            if (getFlag(FLAG_O)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JNO HHLL - Jump if overflow flag is clear
        case OP_JNO: {
            if (!getFlag(FLAG_O)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JME HHLL - Jump if equal (Z flag set, after CMP)
        case OP_JME: {
            if (getFlag(FLAG_Z)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JNME HHLL - Jump if not equal (Z flag clear, after CMP)
        case OP_JNME: {
            if (!getFlag(FLAG_Z)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JG HHLL - Jump if greater (signed: Z clear AND N==O)
        case OP_JG: {
            bool z = getFlag(FLAG_Z);
            bool n = getFlag(FLAG_N);
            bool o = getFlag(FLAG_O);
            
            // Greater: not equal AND (N == O)
            if (!z && (n == o)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JGE HHLL - Jump if greater or equal (signed: N==O)
        case OP_JGE: {
            bool n = getFlag(FLAG_N);
            bool o = getFlag(FLAG_O);
            
            // Greater or equal: N == O
            if (n == o) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JL HHLL - Jump if less (signed: N!=O)
        case OP_JL: {
            bool n = getFlag(FLAG_N);
            bool o = getFlag(FLAG_O);
            
            // Less: N != O
            if (n != o) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // JLE HHLL - Jump if less or equal (signed: Z set OR N!=O)
        case OP_JLE: {
            bool z = getFlag(FLAG_Z);
            bool n = getFlag(FLAG_N);
            bool o = getFlag(FLAG_O);
            
            // Less or equal: equal OR (N != O)
            if (z || (n != o)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        // CALL HHLL - Call subroutine at address
        case OP_CALL: {
            regs.SP -= 2;
            memory.writeWord(regs.SP, regs.PC + 4);  // Push return address
            regs.PC = inst.hhll - 4;  // Jump to subroutine
            break;
        }
        
        // RET - Return from subroutine
        case OP_RET: {
            regs.PC = memory.readWord(regs.SP) - 4;  // Pop return address
            regs.SP += 2;
            break;
        }
        
        // CL RX - Call address in register
        case OP_CL: {
            uint8_t x = inst.getX();
            regs.SP -= 2;
            memory.writeWord(regs.SP, regs.PC + 4);  // Push return address
            regs.PC = (uint16_t)regs.R[x] - 4;  // Jump to address in register
            break;
        }
        
        // CLC HHLL - Conditional call if carry is set
        case OP_CLC: {
            if (getFlag(FLAG_C)) {
                regs.SP -= 2;
                memory.writeWord(regs.SP, regs.PC + 4);  // Push return address
                regs.PC = inst.hhll - 4;  // Jump to subroutine
            }
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
    std::cout << "CPU State" << std::endl;
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
