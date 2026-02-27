// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_X86_64)
#define __MATHEXPR_X86_64

#include "mathexpr/codegen.hpp"

#define X86_64_NAMESPACE_BEGIN namespace x86_64 {
#define X86_64_NAMESPACE_END }

MATHEXPR_NAMESPACE_BEGIN

X86_64_NAMESPACE_BEGIN

/* Helpers notes */

/*
    https://www.cs.uaf.edu/2002/fall/cs301/Encoding%20instructions.htm
*/

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

// SIB Byte (scale-index-base) if R/M == 100
// Needed if base == RSP | R12 or using scaled index
// SIB = (scale << 6) | (index << 3) | base
// scale: 00=1, 01=2, 10=4, 11=8

// Prefixes for pretty-printing of bytecode
static const std::unordered_set<std::byte> prefixes = {
    BYTE(0xF2), /* fp64 ops */
    BYTE(0xC3), /* ret */
    BYTE(0xC9), /* leave */
    BYTE(0x55), /* push rbp */
    BYTE(0x48), /* mov */
};

/* Mem related-instructions */

class MATHEXPR_API InstrMov : public Instr
{
    MemLocPtr _mem_loc_from;
    MemLocPtr _mem_loc_to;

public:
    InstrMov(MemLocPtr& from, MemLocPtr& to) : _mem_loc_from(from),
                                               _mem_loc_to(to) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

class MATHEXPR_API InstrPrologue : public Instr
{
    uint64_t _stack_size;

public:
    InstrPrologue(uint64_t stack_size) : _stack_size(stack_size) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

class MATHEXPR_API InstrEpilogue : public Instr
{
    uint64_t _stack_size;

public:
    InstrEpilogue(uint64_t stack_size) : _stack_size(stack_size) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

/* Unary ops instructions */

class MATHEXPR_API InstrNeg : public Instr
{
    MemLocPtr _operand;

public:
    InstrNeg(MemLocPtr& operand) : _operand(operand) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

/* Binary ops instructions */

class MATHEXPR_API InstrAdd : public Instr
{
    MemLocPtr _left;
    MemLocPtr _right;

public:
    InstrAdd(MemLocPtr& left, MemLocPtr& right) : _left(left),
                                                  _right(right) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

class MATHEXPR_API InstrSub : public Instr
{
    MemLocPtr _left;
    MemLocPtr _right;

public:
    InstrSub(MemLocPtr& left, MemLocPtr& right) : _left(left),
                                                  _right(right) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

class MATHEXPR_API InstrMul : public Instr
{
    MemLocPtr _left;
    MemLocPtr _right;

public:
    InstrMul(MemLocPtr& left, MemLocPtr& right) : _left(left),
                                                  _right(right) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

class MATHEXPR_API InstrDiv : public Instr
{
    MemLocPtr _left;
    MemLocPtr _right;

public:
    InstrDiv(MemLocPtr& left, MemLocPtr& right) : _left(left),
                                                  _right(right) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

/* Func ops instructions */

class MATHEXPR_API InstrCall : public Instr
{
    std::string_view _call_name;

public:
    InstrCall(std::string_view call_name) : _call_name(call_name) {}

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;

    virtual bool needs_linking() const noexcept override { return true; }

    virtual RelocInfo get_link_info(std::size_t bytecode_start) const noexcept override;
};

/* Terminator instructions */

class MATHEXPR_API InstrRet : public Instr
{
public:

    virtual void as_string(std::string& out) const noexcept override;
    virtual void as_bytecode(ByteCode& out) const noexcept override;
};

X86_64_NAMESPACE_END

class MATHEXPR_API X86_64_CodeGenerator : public TargetCodeGenerator
{
public:
    X86_64_CodeGenerator(PlatformABIPtr platform_abi) : TargetCodeGenerator(platform_abi) {}

    virtual ~X86_64_CodeGenerator() override = default;

    virtual bool is_valid() const noexcept override { return get_current_isa() == ISA_x86_64; }

    virtual InstrPtr create_mov(MemLocPtr& from, MemLocPtr& to) override;
    virtual InstrPtr create_prologue(uint64_t stack_size) override;
    virtual InstrPtr create_epilogue(uint64_t stack_size) override;
    virtual InstrPtr create_neg(MemLocPtr& operand) override;
    virtual InstrPtr create_add(MemLocPtr& left, MemLocPtr& right) override;
    virtual InstrPtr create_sub(MemLocPtr& left, MemLocPtr& right) override;
    virtual InstrPtr create_mul(MemLocPtr& left, MemLocPtr& right) override;
    virtual InstrPtr create_div(MemLocPtr& left, MemLocPtr& right) override;
    virtual InstrPtr create_call(std::string_view call_name) override;
    virtual InstrPtr create_ret() override;

    virtual void optimize_instr_sequence(std::vector<InstrPtr>& instructions) noexcept override;
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_X86_64) */
