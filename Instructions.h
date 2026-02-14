#pragma once

#include <cstdint>

// Chip16 Instruction Opcodes

// Special
#define OP_NOP  0x00  // No operation
#define OP_HLT  0x02  // Halt execution

// Arithmetic
#define OP_ADD_R_R    0x01  // ADD RX, RY
#define OP_ADD_R_IMM  0x03  // ADD RX, HHLL
#define OP_SUB_R_R    0x04  // SUB RX, RY
#define OP_SUB_R_IMM  0x05  // SUB RX, HHLL
#define OP_MUL_R_R    0x10  // MUL RX, RY
#define OP_MUL_R_IMM  0x11  // MUL RX, HHLL
#define OP_DIV_R_R    0x13  // DIV RX, RY
#define OP_DIV_R_IMM  0x14  // DIV RX, HHLL
#define OP_MOD_R_R    0x16  // MOD RX, RY
#define OP_MOD_R_IMM  0x17  // MOD RX, HHLL

// Logical
#define OP_AND_R_R    0x07  // AND RX, RY
#define OP_AND_R_IMM  0x08  // AND RX, HHLL
#define OP_OR_R_R     0x0A  // OR RX, RY
#define OP_OR_R_IMM   0x0B  // OR RX, HHLL
#define OP_XOR_R_R    0x0D  // XOR RX, RY
#define OP_XOR_R_IMM  0x0E  // XOR RX, HHLL
#define OP_NOT_R      0x19  // NOT RX

// Comparison
#define OP_CMPI       0x1A  // CMPI RX, HHLL (compare with immediate)
#define OP_CMP        0x1B  // CMP RX, RY (compare registers)
#define OP_TST        0x1C  // TST RX, RY (test bits)
#define OP_NEG        0x1D  // NEG RX (negate - two's complement)
#define OP_RND        0x1E  // RND RX, HHLL (random number 0 to HHLL-1)

// Shifts
#define OP_SHL_R_R    0x30  // SHL RX, RY
#define OP_SHL_R_IMM  0x31  // SHL RX, N
#define OP_SHR_R_R    0x33  // SHR RX, RY
#define OP_SHR_R_IMM  0x34  // SHR RX, N
#define OP_SAR_R_R    0x36  // SAR RX, RY
#define OP_SAR_R_IMM  0x37  // SAR RX, N

// Register/Memory
#define OP_MOV        0x20  // MOV RX, RY
#define OP_LDI        0x40  // LDI RX, HHLL
#define OP_LDM_ADDR   0x41  // LDM RX, HHLL
#define OP_LDM_R      0x42  // LDM RX, RY
#define OP_STM_ADDR   0x43  // STM HHLL, RX
#define OP_STM_R      0x44  // STM RX, RY

// Stack
#define OP_PUSH       0x28  // PUSH RX
#define OP_POP        0x29  // POP RX
#define OP_PUSHALL    0x2A  // PUSHALL
#define OP_POPALL     0x2B  // POPALL
#define OP_PUSHF      0x2C  // PUSHF
#define OP_POPF       0x2D  // POPF

// Jumps (Unconditional)
#define OP_JMP        0x50  // JMP HHLL
#define OP_JMC        0x51  // JMC HHLL (jump relative)
#define OP_JX         0x52  // Jx (jump to RX)

// Jumps (Conditional)
#define OP_JZ         0x53  // JZ HHLL (jump if zero)
#define OP_JC         0x54  // JC HHLL (jump if carry)
#define OP_JN         0x55  // JN HHLL (jump if negative)
#define OP_JNZ        0x56  // JNZ HHLL (jump if not zero)
#define OP_JNC        0x57  // JNC HHLL (jump if not carry)
#define OP_JNN        0x58  // JNN HHLL (jump if not negative)
#define OP_JO         0x59  // JO HHLL (jump if overflow)
#define OP_JNO        0x5A  // JNO HHLL (jump if not overflow)

// Extended Conditional Jumps (comparison-based)
#define OP_JME        0x5B  // JME HHLL (jump if equal, after CMP)
#define OP_JNME       0x5C  // JNME HHLL (jump if not equal)
#define OP_JG         0x5D  // JG HHLL (jump if greater, signed)
#define OP_JGE        0x5E  // JGE HHLL (jump if greater or equal)
#define OP_JL         0x5F  // JL HHLL (jump if less, signed)
#define OP_JLE        0x64  // JLE HHLL (jump if less or equal)

// Subroutines
#define OP_CALL       0x60  // CALL HHLL
#define OP_RET        0x61  // RET
#define OP_CL         0x62  // CL RX (call address in RX)
#define OP_CLC        0x63  // CLC HHLL (conditional call if carry)
