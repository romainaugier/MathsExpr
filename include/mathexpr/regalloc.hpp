// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_REGALLOC)
#define __MATHEXPR_REGALLOC

#include "mathexpr/ssa.hpp"
#include "mathexpr/symtable.hpp"
#include "mathexpr/abi.hpp"

#include <unordered_map>

MATHEXPR_NAMESPACE_BEGIN

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

class MATHEXPR_API MemLoc
{
public:
    virtual ~MemLoc() noexcept {}

    virtual void print() const noexcept = 0;

    virtual int type_id() const noexcept = 0;
};

using MemLocPtr = std::shared_ptr<MemLoc>;

class MATHEXPR_API MemLocInvalid : public MemLoc
{
public:
    virtual void print() const noexcept override {}

    static constexpr int static_type_id() noexcept { return MemLocTypeId_Invalid; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }
};

class MATHEXPR_API Register : public MemLoc
{
    uint32_t _id;

public:
    Register(uint32_t id) : _id(id) {}

    virtual void print() const noexcept override {}

    static constexpr int static_type_id() noexcept { return MemLocTypeId_Register; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    RegisterId get_id() const noexcept { return this->_id; }
};

class MATHEXPR_API Stack : public MemLoc 
{
    uint64_t _offset;

public:
    Stack(uint64_t offset) : _offset(offset) {}

    virtual void print() const noexcept override {}

    static constexpr int static_type_id() noexcept { return MemLocTypeId_Stack; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    uint64_t get_offset() const noexcept { return this->_offset; }

    int64_t get_signed_offset() const noexcept { return -static_cast<int64_t>(this->_offset); }
};

class MATHEXPR_API Memory : public MemLoc
{
    RegisterId _base_ptr; /* RegisterId value where the base ptr is located */
    uint64_t _offset;

public:
    Memory(uint64_t base_ptr, uint64_t offset) : _base_ptr(base_ptr), _offset(offset) {}

    virtual void print() const noexcept override {}

    static constexpr int static_type_id() noexcept { return MemLocTypeId_Memory; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    RegisterId get_base_ptr_register() const noexcept { return this->_base_ptr; }

    uint64_t get_offset() const noexcept { return this->_offset; }
};

template<typename T>
const T* memloc_const_cast(const MemLoc* loc) noexcept
{
    if(loc != nullptr && loc->type_id() == T::static_type_id())
    {
        return static_cast<const T*>(loc);
    }

    return nullptr;
}

template<typename T>
T* memloc_cast(MemLoc* loc) noexcept
{
    if(loc != nullptr && loc->type_id() == T::static_type_id())
    {
        return static_cast<T*>(loc);
    }

    return nullptr;
}

class MATHEXPR_API RegisterAllocator
{
    std::unordered_map<SSAStmtPtr, MemLocPtr> _mapping;
    PlatformABIPtr _platform_abi;

    static bool prepass_commutative_operand_swap(SSA& ssa) noexcept;

    const MemLocPtr get_reusable_register(const SSAStmtPtr& statement) const noexcept;

public:
    RegisterAllocator(PlatformABIPtr platform_abi) : _platform_abi(platform_abi) {}

    bool allocate(SSA& ssa, const SymbolTable& symtable) noexcept;

    MemLocPtr get_memloc(SSAStmtPtr& stmt) const noexcept
    {
        static MemLocPtr invalid = std::make_shared<MemLocInvalid>();

        auto it = this->_mapping.find(stmt);

        return it != this->_mapping.end() ? it->second : invalid;
    }
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_REGALLOC) */