#include "CPU.h"
#include "Instructions.h"
#include "Graphics.h"
#include "Sound.h"
#include <iostream>
#include <iomanip>
#include <cstring>
#include <random>

// Constructor
CPU::CPU(Memory& mem) : memory(mem), graphics(nullptr), sound(nullptr), running(false), cycles(0) {
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
        // 0x - Misc/Video/Audio
        
        case OP_NOP:  // 0x00 - No operation
            break;
        
        case OP_CLS:  // 0x01 - Clear screen
            if (graphics) {
                graphics->clearForeground();
            }
            break;
        
        case OP_VBLNK:  // 0x02 - Wait for vertical blank
            if (graphics) {
                if (!graphics->getVBlank()) {
                    regs.PC -= 4;  // Repeat until VBLANK
                }
            }
            break;
        
        case OP_BGC: {  // 0x03 - Set background color
            if (graphics) {
                uint8_t colorIndex = inst.yx & 0x0F;
                graphics->setBackgroundColor(colorIndex);
            }
            break;
        }
        
        case OP_SPR: {  // 0x04 - Set sprite width/height
            if (graphics) {
                uint8_t width = inst.hhll & 0xFF;
                uint8_t height = (inst.hhll >> 8) & 0xFF;
                graphics->setSpriteSize(width, height);
            }
            break;
        }
        
        case OP_DRW_I: {  // 0x05 - Draw sprite at (RX, RY) from address HHLL
            if (graphics) {
                uint8_t x = inst.getX();
                uint8_t y = inst.getY();
                int16_t screenX = regs.R[x];
                int16_t screenY = regs.R[y];
                uint8_t* spriteData = memory.getPointer(inst.hhll);
                bool collision = graphics->drawSprite(screenX, screenY, spriteData, nullptr);
                setFlag(FLAG_C, collision);
            }
            break;
        }
        
        case OP_DRW_R: {  // 0x06 - Draw sprite at (RX, RY) from address in RZ
            if (graphics) {
                uint8_t x = inst.getX();
                uint8_t y = inst.getY();
                uint8_t z = inst.hhll & 0x0F;
                int16_t screenX = regs.R[x];
                int16_t screenY = regs.R[y];
                uint8_t* spriteData = memory.getPointer((uint16_t)regs.R[z]);
                bool collision = graphics->drawSprite(screenX, screenY, spriteData, nullptr);
                setFlag(FLAG_C, collision);
            }
            break;
        }
        
        case OP_RND: {  // 0x07 - Random number 0 to HHLL
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
        
        case OP_FLIP: {  // 0x08 - Set flip flags
            if (graphics) {
                bool hflip = (inst.hhll & 0x02) != 0;
                bool vflip = (inst.hhll & 0x01) != 0;
                graphics->setFlipFlags(hflip, vflip);
            }
            break;
        }
        
        case OP_SND0:  // 0x09 - Stop sound
            if (sound) {
                sound->stop();
            }
            break;
        
        case OP_SND1:  // 0x0A - Play 500Hz for HHLL ms
            if (sound) {
                sound->playTone(500.0f, (float)inst.hhll);
            }
            break;
        
        case OP_SND2:  // 0x0B - Play 1000Hz for HHLL ms
            if (sound) {
                sound->playTone(1000.0f, (float)inst.hhll);
            }
            break;
        
        case OP_SND3:  // 0x0C - Play 1500Hz for HHLL ms
            if (sound) {
                sound->playTone(1500.0f, (float)inst.hhll);
            }
            break;
        
        case OP_SNP: {  // 0x0D - Play tone at RX Hz for HHLL ms
            if (sound) {
                uint8_t x = inst.getX();
                float freq = (float)(uint16_t)regs.R[x];
                sound->playTone(freq, (float)inst.hhll);
            }
            break;
        }
        
        case OP_SNG: {  // 0x0E - Set ADSR envelope parameters
            if (sound) {
                // Byte 1 (yx): AD (Attack in high nibble, Decay in low)
                // Byte 2-3 (hhll): SRVT (Sustain, Release, Volume, waveType)
                int attack = (inst.yx >> 4) & 0x0F;
                int decay = inst.yx & 0x0F;
                int sustain = (inst.hhll >> 12) & 0x0F;
                int release = (inst.hhll >> 8) & 0x0F;
                int volume = (inst.hhll >> 4) & 0x0F;
                int waveType = inst.hhll & 0x03;
                sound->setEnvelope(attack, decay, sustain, release, volume, waveType);
            }
            break;
        }
        
        //1x - Jumps
        
        case OP_JMP:  // 0x10 - Jump to address
            regs.PC = inst.hhll - 4;
            break;
        
        case OP_JMC: {  // 0x11 - Jump if carry flag set
            if (getFlag(FLAG_C)) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        case OP_JX: {  // 0x12 - Conditional jump based on x nibble
            uint8_t cond = inst.getX();
            bool shouldJump = false;
            switch (cond) {
                case 0x0: shouldJump = getFlag(FLAG_Z); break;           // Z - Zero
                case 0x1: shouldJump = !getFlag(FLAG_Z); break;          // NZ - Not Zero
                case 0x2: shouldJump = getFlag(FLAG_N); break;           // N - Negative
                case 0x3: shouldJump = !getFlag(FLAG_N); break;          // NN - Not Negative
                case 0x4: shouldJump = !getFlag(FLAG_N) && !getFlag(FLAG_Z); break;  // P - Positive
                case 0x5: shouldJump = getFlag(FLAG_O); break;           // O - Overflow
                case 0x6: shouldJump = !getFlag(FLAG_O); break;          // NO - No Overflow
                case 0x7: shouldJump = !getFlag(FLAG_C) && !getFlag(FLAG_Z); break;  // A - Above
                case 0x8: shouldJump = !getFlag(FLAG_C); break;          // AE - Above or Equal
                case 0x9: shouldJump = getFlag(FLAG_C); break;           // B - Below
                case 0xA: shouldJump = getFlag(FLAG_C) || getFlag(FLAG_Z); break;    // BE - Below or Equal
                case 0xB: shouldJump = (getFlag(FLAG_O) == getFlag(FLAG_N)) && !getFlag(FLAG_Z); break;  // G
                case 0xC: shouldJump = (getFlag(FLAG_O) == getFlag(FLAG_N)); break;  // GE
                case 0xD: shouldJump = (getFlag(FLAG_O) != getFlag(FLAG_N)); break;  // L
                case 0xE: shouldJump = (getFlag(FLAG_O) != getFlag(FLAG_N)) || getFlag(FLAG_Z); break;  // LE
            }
            if (shouldJump) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        case OP_JME: {  // 0x13 - Jump if RX == RY
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            if (regs.R[x] == regs.R[y]) {
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        case OP_CALL:  // 0x14 - Call subroutine
            regs.SP -= 2;
            memory.writeWord(regs.SP, regs.PC + 4);
            regs.PC = inst.hhll - 4;
            break;
        
        case OP_RET:  // 0x15 - Return from subroutine
            regs.PC = memory.readWord(regs.SP) - 4;
            regs.SP += 2;
            break;
        
        case OP_JMP_R: {  // 0x16 - Jump to address in register
            uint8_t x = inst.getX();
            regs.PC = (uint16_t)regs.R[x] - 4;
            break;
        }
        
        case OP_CX: {  // 0x17 - Conditional call based on x nibble
            uint8_t cond = inst.getX();
            bool shouldCall = false;
            switch (cond) {
                case 0x0: shouldCall = getFlag(FLAG_Z); break;
                case 0x1: shouldCall = !getFlag(FLAG_Z); break;
                case 0x2: shouldCall = getFlag(FLAG_N); break;
                case 0x3: shouldCall = !getFlag(FLAG_N); break;
                case 0x4: shouldCall = !getFlag(FLAG_N) && !getFlag(FLAG_Z); break;
                case 0x5: shouldCall = getFlag(FLAG_O); break;
                case 0x6: shouldCall = !getFlag(FLAG_O); break;
                case 0x7: shouldCall = !getFlag(FLAG_C) && !getFlag(FLAG_Z); break;
                case 0x8: shouldCall = !getFlag(FLAG_C); break;
                case 0x9: shouldCall = getFlag(FLAG_C); break;
                case 0xA: shouldCall = getFlag(FLAG_C) || getFlag(FLAG_Z); break;
                case 0xB: shouldCall = (getFlag(FLAG_O) == getFlag(FLAG_N)) && !getFlag(FLAG_Z); break;
                case 0xC: shouldCall = (getFlag(FLAG_O) == getFlag(FLAG_N)); break;
                case 0xD: shouldCall = (getFlag(FLAG_O) != getFlag(FLAG_N)); break;
                case 0xE: shouldCall = (getFlag(FLAG_O) != getFlag(FLAG_N)) || getFlag(FLAG_Z); break;
            }
            if (shouldCall) {
                regs.SP -= 2;
                memory.writeWord(regs.SP, regs.PC + 4);
                regs.PC = inst.hhll - 4;
            }
            break;
        }
        
        case OP_CALL_R: {  // 0x18 - Call address in register
            uint8_t x = inst.getX();
            regs.SP -= 2;
            memory.writeWord(regs.SP, regs.PC + 4);
            regs.PC = (uint16_t)regs.R[x] - 4;
            break;
        }
        
        //2x - Loads
        
        case OP_LDI_R: {  // 0x20 - Load immediate into register
            uint8_t x = inst.getX();
            regs.R[x] = (int16_t)inst.hhll;
            break;
        }
        
        case OP_LDI_SP:  // 0x21 - Load immediate into SP
            regs.SP = inst.hhll;
            break;
        
        case OP_LDM_I: {  // 0x22 - Load from memory (immediate address)
            uint8_t x = inst.getX();
            regs.R[x] = (int16_t)memory.readWord(inst.hhll);
            break;
        }
        
        case OP_LDM_R: {  // 0x23 - Load from memory (register address)
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            regs.R[x] = (int16_t)memory.readWord((uint16_t)regs.R[y]);
            break;
        }
        
        case OP_MOV: {  // 0x24 - Move register to register
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            regs.R[x] = regs.R[y];
            break;
        }
        
        //3x - Stores
        
        case OP_STM_I: {  // 0x30 - Store to memory (immediate address)
            uint8_t x = inst.getX();
            memory.writeWord(inst.hhll, (uint16_t)regs.R[x]);
            break;
        }
        
        case OP_STM_R: {  // 0x31 - Store to memory (register address)
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            memory.writeWord((uint16_t)regs.R[y], (uint16_t)regs.R[x]);
            break;
        }
        
        // 4x - Addition
        
        case OP_ADDI: {  // 0x40 - Add immediate
            uint8_t x = inst.getX();
            int16_t a = regs.R[x];
            int16_t b = (int16_t)inst.hhll;
            int32_t result = (int32_t)a + (int32_t)b;
            regs.R[x] = (int16_t)result;
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));
            setFlag(FLAG_O, ((a > 0 && b > 0 && regs.R[x] < 0) || 
                             (a < 0 && b < 0 && regs.R[x] > 0)));
            break;
        }
        
        case OP_ADD_R2: {  // 0x41 - Add two registers
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int16_t a = regs.R[x];
            int16_t b = regs.R[y];
            int32_t result = (int32_t)a + (int32_t)b;
            regs.R[x] = (int16_t)result;
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));
            setFlag(FLAG_O, ((a > 0 && b > 0 && regs.R[x] < 0) || 
                             (a < 0 && b < 0 && regs.R[x] > 0)));
            break;
        }
        
        case OP_ADD_R3: {  // 0x42 - Add two registers, store in third
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t z = inst.hhll & 0x0F;
            int16_t a = regs.R[x];
            int16_t b = regs.R[y];
            int32_t result = (int32_t)a + (int32_t)b;
            regs.R[z] = (int16_t)result;
            updateZeroNegativeFlags(regs.R[z]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));
            setFlag(FLAG_O, ((a > 0 && b > 0 && regs.R[z] < 0) || 
                             (a < 0 && b < 0 && regs.R[z] > 0)));
            break;
        }
        
        //5x - Subtraction
        
        case OP_SUBI: {  // 0x50 - Subtract immediate
            uint8_t x = inst.getX();
            int16_t a = regs.R[x];
            int16_t b = (int16_t)inst.hhll;
            int32_t result = (int32_t)a - (int32_t)b;
            regs.R[x] = (int16_t)result;
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (uint16_t)a < (uint16_t)b);
            setFlag(FLAG_O, ((a > 0 && b < 0 && regs.R[x] < 0) || 
                             (a < 0 && b > 0 && regs.R[x] > 0)));
            break;
        }
        
        case OP_SUB_R2: {  // 0x51 - Subtract two registers
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int16_t a = regs.R[x];
            int16_t b = regs.R[y];
            int32_t result = (int32_t)a - (int32_t)b;
            regs.R[x] = (int16_t)result;
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (uint16_t)a < (uint16_t)b);
            setFlag(FLAG_O, ((a > 0 && b < 0 && regs.R[x] < 0) || 
                             (a < 0 && b > 0 && regs.R[x] > 0)));
            break;
        }
        
        case OP_SUB_R3: {  // 0x52 - Subtract two registers, store in third
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t z = inst.hhll & 0x0F;
            int16_t a = regs.R[x];
            int16_t b = regs.R[y];
            int32_t result = (int32_t)a - (int32_t)b;
            regs.R[z] = (int16_t)result;
            updateZeroNegativeFlags(regs.R[z]);
            setFlag(FLAG_C, (uint16_t)a < (uint16_t)b);
            setFlag(FLAG_O, ((a > 0 && b < 0 && regs.R[z] < 0) || 
                             (a < 0 && b > 0 && regs.R[z] > 0)));
            break;
        }
        
        case OP_CMPI: {  // 0x53 - Compare with immediate
            uint8_t x = inst.getX();
            int16_t a = regs.R[x];
            int16_t b = (int16_t)inst.hhll;
            int32_t result = (int32_t)a - (int32_t)b;
            setFlag(FLAG_Z, result == 0);
            setFlag(FLAG_N, (int16_t)result < 0);
            setFlag(FLAG_C, (uint16_t)a < (uint16_t)b);
            setFlag(FLAG_O, ((a ^ b) & (a ^ (int16_t)result)) & 0x8000);
            break;
        }
        
        case OP_CMP: {  // 0x54 - Compare two registers
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int16_t a = regs.R[x];
            int16_t b = regs.R[y];
            int32_t result = (int32_t)a - (int32_t)b;
            setFlag(FLAG_Z, result == 0);
            setFlag(FLAG_N, (int16_t)result < 0);
            setFlag(FLAG_C, (uint16_t)a < (uint16_t)b);
            setFlag(FLAG_O, ((a ^ b) & (a ^ (int16_t)result)) & 0x8000);
            break;
        }
        
        //6x - Bitwise AND
        
        case OP_ANDI: {  // 0x60 - AND with immediate
            uint8_t x = inst.getX();
            regs.R[x] = regs.R[x] & (int16_t)inst.hhll;
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_AND_R2: {  // 0x61 - AND two registers
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            regs.R[x] = regs.R[x] & regs.R[y];
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_AND_R3: {  // 0x62 - AND two registers, store in third
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t z = inst.hhll & 0x0F;
            regs.R[z] = regs.R[x] & regs.R[y];
            updateZeroNegativeFlags(regs.R[z]);
            break;
        }
        
        case OP_TSTI: {  // 0x63 - Test with immediate
            uint8_t x = inst.getX();
            int16_t result = regs.R[x] & (int16_t)inst.hhll;
            setFlag(FLAG_Z, result == 0);
            setFlag(FLAG_N, result < 0);
            break;
        }
        
        case OP_TST: {  // 0x64 - Test two registers
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int16_t result = regs.R[x] & regs.R[y];
            setFlag(FLAG_Z, result == 0);
            setFlag(FLAG_N, result < 0);
            break;
        }
        
        //7x - Bitwise OR
        
        case OP_ORI: {  // 0x70 - OR with immediate
            uint8_t x = inst.getX();
            regs.R[x] = regs.R[x] | (int16_t)inst.hhll;
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_OR_R2: {  // 0x71 - OR two registers
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            regs.R[x] = regs.R[x] | regs.R[y];
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_OR_R3: {  // 0x72 - OR two registers, store in third
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t z = inst.hhll & 0x0F;
            regs.R[z] = regs.R[x] | regs.R[y];
            updateZeroNegativeFlags(regs.R[z]);
            break;
        }
        
        //8x - Bitwise XOR
        
        case OP_XORI: {  // 0x80 - XOR with immediate
            uint8_t x = inst.getX();
            regs.R[x] = regs.R[x] ^ (int16_t)inst.hhll;
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_XOR_R2: {  // 0x81 - XOR two registers
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            regs.R[x] = regs.R[x] ^ regs.R[y];
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_XOR_R3: {  // 0x82 - XOR two registers, store in third
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t z = inst.hhll & 0x0F;
            regs.R[z] = regs.R[x] ^ regs.R[y];
            updateZeroNegativeFlags(regs.R[z]);
            break;
        }
        
        //9x - Multiplication
        
        case OP_MULI: {  // 0x90 - Multiply with immediate
            uint8_t x = inst.getX();
            int32_t result = (int32_t)regs.R[x] * (int32_t)(int16_t)inst.hhll;
            regs.R[x] = (int16_t)result;
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));
            break;
        }
        
        case OP_MUL_R2: {  // 0x91 - Multiply two registers
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int32_t result = (int32_t)regs.R[x] * (int32_t)regs.R[y];
            regs.R[x] = (int16_t)result;
            updateZeroNegativeFlags(regs.R[x]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));
            break;
        }
        
        case OP_MUL_R3: {  // 0x92 - Multiply two registers, store in third
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t z = inst.hhll & 0x0F;
            int32_t result = (int32_t)regs.R[x] * (int32_t)regs.R[y];
            regs.R[z] = (int16_t)result;
            updateZeroNegativeFlags(regs.R[z]);
            setFlag(FLAG_C, (result > 0x7FFF || result < -0x8000));
            break;
        }
        
        //Ax - Division
        
        case OP_DIVI: {  // 0xA0 - Divide by immediate
            uint8_t x = inst.getX();
            int16_t divisor = (int16_t)inst.hhll;
            if (divisor == 0) {
                setFlag(FLAG_C, true);
                regs.R[x] = 0;
            } else {
                regs.R[x] = regs.R[x] / divisor;
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_DIV_R2: {  // 0xA1 - Divide two registers
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            if (regs.R[y] == 0) {
                setFlag(FLAG_C, true);
                regs.R[x] = 0;
            } else {
                regs.R[x] = regs.R[x] / regs.R[y];
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_DIV_R3: {  // 0xA2 - Divide two registers, store in third
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t z = inst.hhll & 0x0F;
            if (regs.R[y] == 0) {
                setFlag(FLAG_C, true);
                regs.R[z] = 0;
            } else {
                regs.R[z] = regs.R[x] / regs.R[y];
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[z]);
            break;
        }
        
        case OP_MODI: {  // 0xA3 - Modulo with immediate
            uint8_t x = inst.getX();
            int16_t divisor = (int16_t)inst.hhll;
            if (divisor == 0) {
                setFlag(FLAG_C, true);
                regs.R[x] = 0;
            } else {
                regs.R[x] = regs.R[x] % divisor;
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_MOD_R2: {  // 0xA4 - Modulo two registers
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            if (regs.R[y] == 0) {
                setFlag(FLAG_C, true);
                regs.R[x] = 0;
            } else {
                regs.R[x] = regs.R[x] % regs.R[y];
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_MOD_R3: {  // 0xA5 - Modulo two registers, store in third
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t z = inst.hhll & 0x0F;
            if (regs.R[y] == 0) {
                setFlag(FLAG_C, true);
                regs.R[z] = 0;
            } else {
                regs.R[z] = regs.R[x] % regs.R[y];
                setFlag(FLAG_C, false);
            }
            updateZeroNegativeFlags(regs.R[z]);
            break;
        }
        
        case OP_REMI:   // 0xA6 - Remainder with immediate
        case OP_REM_R2: // 0xA7 - Remainder two registers
        case OP_REM_R3: // 0xA8 - Remainder two registers, store in third
            // REM is same as MOD for signed integers in C++
            // TODO: Verify behavior matches spec
            break;
        
        //Bx - Shifts
        
        case OP_SHL_N: {  // 0xB0 - Shift left by immediate
            uint8_t x = inst.getX();
            uint8_t n = inst.hhll & 0x0F;
            if (n > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (16 - n))) != 0);
                regs.R[x] = regs.R[x] << n;
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_SHR_N: {  // 0xB1 - Shift right by immediate (logical)
            uint8_t x = inst.getX();
            uint8_t n = inst.hhll & 0x0F;
            if (n > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (n - 1))) != 0);
                regs.R[x] = (uint16_t)regs.R[x] >> n;
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_SAR_N: {  // 0xB2 - Shift arithmetic right by immediate
            uint8_t x = inst.getX();
            uint8_t n = inst.hhll & 0x0F;
            if (n > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (n - 1))) != 0);
                regs.R[x] = regs.R[x] >> n;  // Arithmetic shift preserves sign
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_SHL_R: {  // 0xB3 - Shift left by register
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t n = regs.R[y] & 0x0F;
            if (n > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (16 - n))) != 0);
                regs.R[x] = regs.R[x] << n;
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_SHR_R: {  // 0xB4 - Shift right by register (logical)
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t n = regs.R[y] & 0x0F;
            if (n > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (n - 1))) != 0);
                regs.R[x] = (uint16_t)regs.R[x] >> n;
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_SAR_R: {  // 0xB5 - Shift arithmetic right by register
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            uint8_t n = regs.R[y] & 0x0F;
            if (n > 0) {
                setFlag(FLAG_C, (regs.R[x] & (1 << (n - 1))) != 0);
                regs.R[x] = regs.R[x] >> n;
            }
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        // Cx - Push/Pop
        
        case OP_PUSH: {  // 0xC0 - Push register
            uint8_t x = inst.getX();
            regs.SP -= 2;
            memory.writeWord(regs.SP, (uint16_t)regs.R[x]);
            break;
        }
        
        case OP_POP: {  // 0xC1 - Pop to register
            uint8_t x = inst.getX();
            regs.R[x] = (int16_t)memory.readWord(regs.SP);
            regs.SP += 2;
            break;
        }
        
        case OP_PUSHALL:  // 0xC2 - Push all registers
            for (int i = 0; i < 16; i++) {
                regs.SP -= 2;
                memory.writeWord(regs.SP, (uint16_t)regs.R[i]);
            }
            break;
        
        case OP_POPALL:  // 0xC3 - Pop all registers
            for (int i = 15; i >= 0; i--) {
                regs.R[i] = (int16_t)memory.readWord(regs.SP);
                regs.SP += 2;
            }
            break;
        
        case OP_PUSHF:  // 0xC4 - Push flags
            regs.SP -= 2;
            memory.writeWord(regs.SP, (uint16_t)regs.FLAGS);
            break;
        
        case OP_POPF:  // 0xC5 - Pop flags
            regs.FLAGS = (uint8_t)memory.readWord(regs.SP);
            regs.SP += 2;
            break;
        
        // Dx - Palette
        
        case OP_PAL_I: {  // 0xD0 - Load palette (immediate address)
            if (graphics) {
                uint8_t* paletteData = memory.getPointer(inst.hhll);
                graphics->loadCustomPalette(paletteData);
            }
            break;
        }
        
        case OP_PAL_R: {  // 0xD1 - Load palette (register address)
            uint8_t x = inst.getX();
            if (graphics) {
                uint8_t* paletteData = memory.getPointer((uint16_t)regs.R[x]);
                graphics->loadCustomPalette(paletteData);
            }
            break;
        }
        
        // === Ex - Not/Neg ===
        
        case OP_NOTI: {  // 0xE0 - NOT immediate (store in RX)
            uint8_t x = inst.getX();
            regs.R[x] = ~(int16_t)inst.hhll;
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_NOT_R: {  // 0xE1 - NOT register
            uint8_t x = inst.getX();
            regs.R[x] = ~regs.R[x];
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_NOT_R2: {  // 0xE2 - NOT register to register
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            regs.R[x] = ~regs.R[y];
            updateZeroNegativeFlags(regs.R[x]);
            break;
        }
        
        case OP_NEGI: {  // 0xE3 - NEG immediate (store in RX)
            uint8_t x = inst.getX();
            int16_t val = (int16_t)inst.hhll;
            regs.R[x] = -val;
            setFlag(FLAG_Z, regs.R[x] == 0);
            setFlag(FLAG_N, regs.R[x] < 0);
            setFlag(FLAG_O, val == -32768);
            break;
        }
        
        case OP_NEG_R: {  // 0xE4 - NEG register
            uint8_t x = inst.getX();
            int16_t original = regs.R[x];
            regs.R[x] = -original;
            setFlag(FLAG_Z, regs.R[x] == 0);
            setFlag(FLAG_N, regs.R[x] < 0);
            setFlag(FLAG_O, original == -32768);
            break;
        }
        
        case OP_NEG_R2: {  // 0xE5 - NEG register to register
            uint8_t x = inst.getX();
            uint8_t y = inst.getY();
            int16_t original = regs.R[y];
            regs.R[x] = -original;
            setFlag(FLAG_Z, regs.R[x] == 0);
            setFlag(FLAG_N, regs.R[x] < 0);
            setFlag(FLAG_O, original == -32768);
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
