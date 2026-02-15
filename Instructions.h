#pragma once

#include <cstdint>

// 0x - Misc/Video/Audio
#define OP_NOP   0x00  // No operation
#define OP_CLS   0x01  // Clear screen
#define OP_VBLNK 0x02  // Wait for vertical blank
#define OP_BGC   0x03  // Set background color
#define OP_SPR   0x04  // Set sprite width/height
#define OP_DRW_I 0x05  // Draw sprite (immediate address)
#define OP_DRW_R 0x06  // Draw sprite (register address)
#define OP_RND   0x07  // Random number
#define OP_FLIP  0x08  // Set flip flags
#define OP_SND0  0x09  // Stop sound
#define OP_SND1  0x0A  // Play 500Hz tone
#define OP_SND2  0x0B  // Play 1000Hz tone
#define OP_SND3  0x0C  // Play 1500Hz tone
#define OP_SNP   0x0D  // Play sound from register
#define OP_SNG   0x0E  // Set sound generation parameters

// 1x - Jumps
#define OP_JMP   0x10  // Jump to address
#define OP_JMC   0x11  // Jump if carry
#define OP_JX    0x12  // Conditional jump
#define OP_JME   0x13  // Jump if equal (RX == RY)
#define OP_CALL  0x14  // Call subroutine
#define OP_RET   0x15  // Return from subroutine
#define OP_JMP_R 0x16  // Jump to address in register
#define OP_CX    0x17  // Conditional call
#define OP_CALL_R 0x18 // Call address in register

// 2x - Loads
#define OP_LDI_R  0x20  // Load immediate into register
#define OP_LDI_SP 0x21  // Load immediate into SP
#define OP_LDM_I  0x22  // Load from memory (immediate address)
#define OP_LDM_R  0x23  // Load from memory (register address)
#define OP_MOV    0x24  // Move register to register

// 3x - Stores
#define OP_STM_I  0x30  // Store to memory (immediate address)
#define OP_STM_R  0x31  // Store to memory (register address)

// 4x - Addition
#define OP_ADDI   0x40  // Add immediate
#define OP_ADD_R2 0x41  // Add two registers
#define OP_ADD_R3 0x42  // Add two registers, store in third

// 5x - Subtraction
#define OP_SUBI   0x50  // Subtract immediate
#define OP_SUB_R2 0x51  // Subtract two registers
#define OP_SUB_R3 0x52  // Subtract two registers, store in third
#define OP_CMPI   0x53  // Compare with immediate
#define OP_CMP    0x54  // Compare two registers

// 6x - Bitwise AND
#define OP_ANDI   0x60  // AND with immediate
#define OP_AND_R2 0x61  // AND two registers
#define OP_AND_R3 0x62  // AND two registers, store in third
#define OP_TSTI   0x63  // Test with immediate
#define OP_TST    0x64  // Test two registers

// 7x - Bitwise OR
#define OP_ORI    0x70  // OR with immediate
#define OP_OR_R2  0x71  // OR two registers
#define OP_OR_R3  0x72  // OR two registers, store in third

// 8x - Bitwise XOR
#define OP_XORI   0x80  // XOR with immediate
#define OP_XOR_R2 0x81  // XOR two registers
#define OP_XOR_R3 0x82  // XOR two registers, store in third

// 9x - Multiplication
#define OP_MULI   0x90  // Multiply with immediate
#define OP_MUL_R2 0x91  // Multiply two registers
#define OP_MUL_R3 0x92  // Multiply two registers, store in third

// Ax - Division
#define OP_DIVI   0xA0  // Divide by immediate
#define OP_DIV_R2 0xA1  // Divide two registers
#define OP_DIV_R3 0xA2  // Divide two registers, store in third
#define OP_MODI   0xA3  // Modulo with immediate
#define OP_MOD_R2 0xA4  // Modulo two registers
#define OP_MOD_R3 0xA5  // Modulo two registers, store in third
#define OP_REMI   0xA6  // Remainder with immediate
#define OP_REM_R2 0xA7  // Remainder two registers
#define OP_REM_R3 0xA8  // Remainder two registers, store in third

// Bx - Shifts
#define OP_SHL_N  0xB0  // Shift left by immediate
#define OP_SHR_N  0xB1  // Shift right by immediate
#define OP_SAR_N  0xB2  // Shift arithmetic right by immediate
#define OP_SHL_R  0xB3  // Shift left by register
#define OP_SHR_R  0xB4  // Shift right by register
#define OP_SAR_R  0xB5  // Shift arithmetic right by register

// Cx - Push/Pop
#define OP_PUSH     0xC0  // Push register
#define OP_POP      0xC1  // Pop to register
#define OP_PUSHALL  0xC2  // Push all registers
#define OP_POPALL   0xC3  // Pop all registers
#define OP_PUSHF    0xC4  // Push flags
#define OP_POPF     0xC5  // Pop flags

// Dx - Palette
#define OP_PAL_I    0xD0  // Load palette (immediate address)
#define OP_PAL_R    0xD1  // Load palette (register address)

// Ex - Not/Neg
#define OP_NOTI     0xE0  // NOT immediate
#define OP_NOT_R    0xE1  // NOT register
#define OP_NOT_R2   0xE2  // NOT register to register
#define OP_NEGI     0xE3  // NEG immediate
#define OP_NEG_R    0xE4  // NEG register
#define OP_NEG_R2   0xE5  // NEG register to register
