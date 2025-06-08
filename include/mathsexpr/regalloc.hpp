// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_REGALLOC)
#define __MATHSEXPR_REGALLOC

#include "mathsexpr/ssa.hpp"
#include "mathsexpr/bytecode_x86_64.hpp"
#include "mathsexpr/symtable.hpp"

#include <unordered_map>

MATHSEXPR_NAMESPACE_BEGIN

enum MemLocTypeId : uint32_t
{
    MemLocTypeId_Invalid,
    MemLocTypeId_Register,
    MemLocTypeId_Stack,
    MemLocTypeId_Memory,
};

enum MemLocRegister : uint32_t
{
    MemLocRegister_Variables,
    MemLocRegister_Literals,
};

using RmOffByte = std::pair<std::byte, std::byte>;

class MATHSEXPR_API MemLoc
{
public:
    virtual ~MemLoc() noexcept {}

    virtual void print() const noexcept = 0;

    virtual int type_id() const noexcept = 0;

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept = 0;

    virtual std::byte as_reg_byte(uint32_t isa, uint32_t platform) const noexcept = 0;

    virtual RmOffByte as_rm_off_byte(uint32_t isa, uint32_t platform) const noexcept = 0;
};

using MemLocPtr = std::shared_ptr<MemLoc>;

static constexpr uint64_t INVALID_REGISTER = std::numeric_limits<uint64_t>::max();

class MATHSEXPR_API MemLocInvalid : public MemLoc
{
public:
    virtual void print() const noexcept override {}

    static constexpr int static_type_id() noexcept { return MemLocTypeId_Invalid; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;

    virtual std::byte as_reg_byte(uint32_t isa, uint32_t platform) const noexcept override;

    virtual RmOffByte as_rm_off_byte(uint32_t isa, uint32_t platform) const noexcept override;
};

class MATHSEXPR_API Register : public MemLoc
{
    uint64_t _id;

public:
    Register(uint64_t id) : _id(id) {}

    virtual void print() const noexcept override {}

    static constexpr int static_type_id() noexcept { return MemLocTypeId_Register; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;

    virtual std::byte as_reg_byte(uint32_t isa, uint32_t platform) const noexcept override;

    virtual RmOffByte as_rm_off_byte(uint32_t isa, uint32_t platform) const noexcept override;

    uint64_t get_id() const noexcept { return this->_id; }
};

class MATHSEXPR_API Stack : public MemLoc 
{
    uint64_t _offset;

public:
    Stack(uint64_t offset) : _offset(offset) {}

    virtual void print() const noexcept override {}

    static constexpr int static_type_id() noexcept { return MemLocTypeId_Stack; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;

    virtual std::byte as_reg_byte(uint32_t isa, uint32_t platform) const noexcept override;

    virtual RmOffByte as_rm_off_byte(uint32_t isa, uint32_t platform) const noexcept override;

    uint64_t get_offset() const noexcept { return this->_offset; }
};

class MATHSEXPR_API Memory : public MemLoc
{
    uint64_t _base_ptr; /* GP Register where the base ptr is located */
    uint64_t _offset;

public:
    Memory(uint64_t base_ptr, uint64_t offset) : _base_ptr(base_ptr), _offset(offset) {}

    virtual void print() const noexcept override {}

    static constexpr int static_type_id() noexcept { return MemLocTypeId_Memory; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;

    virtual std::byte as_reg_byte(uint32_t isa, uint32_t platform) const noexcept override;

    virtual RmOffByte as_rm_off_byte(uint32_t isa, uint32_t platform) const noexcept override;
};

template<typename T>
const T* memloc_cast(const MemLoc* loc) noexcept
{
    if(loc != nullptr && loc->type_id() == T::static_type_id())
    {
        return static_cast<const T*>(loc);
    }

    return nullptr;
}

class MATHSEXPR_API RegisterAllocator
{
    std::unordered_map<SSAStmtPtr, MemLocPtr> _mapping;

    Platform _platform;
    ISA _isa;

    uint64_t _max_registers;

    static uint64_t get_max_available_registers(Platform platform, 
                                                ISA isa) noexcept;

    static uint32_t get_fp_call_return_value_register(Platform platform,
                                                      ISA isa) noexcept;

    static uint64_t get_fp_call_max_args_register(Platform platform,
                                                  ISA isa) noexcept;

    static bool prepass_commutative_operand_swap(SSA& ssa) noexcept;

    const MemLocPtr get_reusable_register(const SSAStmtPtr& statement) const noexcept;

public:
    RegisterAllocator(Platform platform, 
                      ISA isa) : _platform(platform),
                                 _isa(isa),
                                 _max_registers(RegisterAllocator::get_max_available_registers(platform, 
                                                                                               isa))
                                 {}

    bool allocate(SSA& ssa, const SymbolTable& symtable) noexcept;

    MemLocPtr get_memloc(SSAStmtPtr& stmt) const noexcept
    {
        static MemLocPtr invalid = std::make_shared<MemLocInvalid>();

        auto it = this->_mapping.find(stmt);

        return it != this->_mapping.end() ? it->second : invalid;
    }
};

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_REGALLOC) */