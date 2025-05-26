// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_CODEGEN)
#define __MATHSEXPR_CODEGEN

#include "mathsexpr/ssa.h"
#include "mathsexpr/symtable.h"

#include <variant>

MATHSEXPR_NAMESPACE_BEGIN

enum CodeGenPlatform : uint32_t
{
    CodeGenPlatform_Windows,
    CodeGenPlatform_Linux,
};

enum CodeGenISA : uint32_t
{
    CodeGenISA_x86_64,
};

enum RegisterKind : uint32_t
{
    RegisterKind_Virtual,
    RegisterKind_Physical,
};

static constexpr uint32_t INVALID_REGISTER_ID = std::numeric_limits<uint32_t>::max();

class MATHSEXPR_API Register
{
    uint32_t _id;
    uint32_t _kind;

public:
    Register(uint32_t id = INVALID_REGISTER_ID) : _id(id), _kind(RegisterKind_Virtual) {}

    bool is_virtual() const noexcept { return this->_kind == RegisterKind_Virtual; }
    bool is_physical() const noexcept { return this->_kind == RegisterKind_Physical; }

    void set_kind(uint32_t kind) noexcept { this->_kind = kind; }

    uint32_t get_id() const noexcept { return this->_id; }

    void set_id(uint32_t id) noexcept { this->_id = id; }

    void to_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept;
};

class MATHSEXPR_API MemoryAddress
{
    uint32_t _id;
    uint32_t _offset;

public:
    MemoryAddress(uint32_t id, uint32_t offset) : _id(id), _offset(offset) {}

    uint32_t get_id() const noexcept { return this->_id; }
    void set_id(uint32_t id) noexcept { this->_id = id; }

    uint32_t get_offset() const noexcept { return this->_offset; }
    void set_offset(uint32_t offset) noexcept { this->_offset = offset; }
};

using StackOffset = int64_t;

using OperandVariant = std::variant<Register,
                                    StackOffset,
                                    MemoryAddress>;

class MATHSEXPR_API Operand
{
public:
    OperandVariant data;

    static Operand from_register(Register r) noexcept
    {
        return Operand{Register{r}};
    }

    static Operand from_stack_offset(StackOffset offset) noexcept
    {
        return Operand{StackOffset{offset}};
    }

    static Operand from_memory_address(MemoryAddress address) noexcept
    {
        return Operand{MemoryAddress{address}};
    }

    void to_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept;
};

using ByteCode = std::vector<std::byte>;

class MATHSEXPR_API Instruction
{
public:
    virtual ~Instruction() = default;

    virtual std::vector<Operand*> get_used_operands() noexcept = 0;
    virtual std::vector<Operand*> get_mutable_operands() noexcept = 0;

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept = 0;
    virtual void as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept = 0;
};

class MATHSEXPR_API InstructionMov : public Instruction
{
    Operand _src;
    Operand _dst;

public:
    InstructionMov(Operand src, Operand dst) : _src(src), _dst(dst) {}

    virtual std::vector<Operand*> get_used_operands() noexcept override { return std::vector({ &this->_src }); }
    virtual std::vector<Operand*> get_mutable_operands() noexcept override { return std::vector({ &this->_dst }); };

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;
    virtual void as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept override;
};

using InstructionPtr = std::shared_ptr<Instruction>;

class MATHSEXPR_API CodeGenerator
{
    std::vector<InstructionPtr> _instructions;

public:
    CodeGenerator() {}

    bool build(const SSA& ssa, SymbolTable& symtable) noexcept;

    void print(uint32_t isa, uint32_t platform) const noexcept;

    std::tuple<bool, ByteCode> as_bytecode(uint32_t isa, uint32_t platform) const noexcept;

    std::tuple<bool, std::string> as_string(uint32_t isa, uint32_t platform) const noexcept;
};

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_CODEGEN) */