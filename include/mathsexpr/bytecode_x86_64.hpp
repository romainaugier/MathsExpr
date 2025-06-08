// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_BYTECODE_X86_64)
#define __MATHSEXPR_BYTECODE_X86_64

#include "mathsexpr/bytecode.hpp"
#include "mathsexpr/platform.hpp"

MATHSEXPR_NAMESPACE_BEGIN

#define X86_64_NAMESPACE_BEGIN namespace x86_64 {
#define X86_64_NAMESPACE_END }

X86_64_NAMESPACE_BEGIN

/* Register Codes (ModR/M, SIB fields) */
static constexpr std::byte RAX = BYTE(0);  // 000
static constexpr std::byte RCX = BYTE(1);  // 001
static constexpr std::byte RDX = BYTE(2);  // 010
static constexpr std::byte RBX = BYTE(3);  // 011
static constexpr std::byte RSP = BYTE(4);  // 100 (implies SIB when used as base)
static constexpr std::byte RBP = BYTE(5);  // 101 (requires disp32 if Mod == 00)
static constexpr std::byte RSI = BYTE(6);  // 110
static constexpr std::byte RDI = BYTE(7);  // 111

static constexpr std::byte R8  = BYTE(0);  // Use REX.B/R/X = 1
static constexpr std::byte R9  = BYTE(1);
static constexpr std::byte R10 = BYTE(2);
static constexpr std::byte R11 = BYTE(3);
static constexpr std::byte R12 = BYTE(4);
static constexpr std::byte R13 = BYTE(5);
static constexpr std::byte R14 = BYTE(6);
static constexpr std::byte R15 = BYTE(7);

static constexpr std::byte XMM0 = BYTE(0);
static constexpr std::byte XMM1 = BYTE(1);
static constexpr std::byte XMM2 = BYTE(2);
static constexpr std::byte XMM3 = BYTE(3);
static constexpr std::byte XMM4 = BYTE(4);
static constexpr std::byte XMM5 = BYTE(5);
static constexpr std::byte XMM6 = BYTE(6);
static constexpr std::byte XMM7 = BYTE(7);

std::byte encode_platform_gp_register(uint32_t platform_register) noexcept;

std::byte encode_platform_fp_register(uint32_t platform_register) noexcept;

/* REX Prefix (binary: 0100WRXB) */
static constexpr std::byte REX_BASE = BYTE(0x40);
static constexpr std::byte REX_W    = BYTE(0x08);  // 64-bit operand
static constexpr std::byte REX_R    = BYTE(0x04);  // Extension of Reg field
static constexpr std::byte REX_X    = BYTE(0x02);  // Extension of Index field (SIB)
static constexpr std::byte REX_B    = BYTE(0x01);  // Extension of R/M field or base

/* 
    OPSD common opcodes (just as a reminder)

    movs
    MOVSD_LOAD  0xF2, 0x0F, 0x10   xmm, [mem]
    MOVSD_STORE 0xF2, 0x0F, 0x11   [mem], xmm

    binops
    ADDSD       0xF2, 0x0F, 0x58   xmm, xmm/mem
    SUBSD       0xF2, 0x0F, 0x5C
    MULSD       0xF2, 0x0F, 0x59
    DIVSD       0xF2, 0x0F, 0x5E

    unops
    SQRTSD      0xF2, 0x0F, 0x51

    terminators
    RET         0xC3               return
*/

/* 
    ModR/M Byte (binary: mmrrrmmm)
    mod:   2 bits - 00=no disp, 01=disp8, 10=disp32, 11=register
    reg:   3 bits - XMM or GPR (source or opcode extension)
    r/m:   3 bits - base register or destination
*/

// Mod values
static constexpr std::byte MOD_INDIRECT        = BYTE(0x00);  // [reg]
static constexpr std::byte MOD_INDIRECT_DISP8  = BYTE(0x40);  // [reg + imm8]
static constexpr std::byte MOD_INDIRECT_DISP32 = BYTE(0x80);  // [reg + imm32]
static constexpr std::byte MOD_DIRECT          = BYTE(0xC0);  // Register to register

// ======== SIB Byte (scale-index-base) if R/M == 100 ========
// Needed if base == RSP or using scaled index
// SIB = (scale << 6) | (index << 3) | base
// scale: 00=1, 01=2, 10=4, 11=8

X86_64_NAMESPACE_END

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_BYTECODE_X86_64) */