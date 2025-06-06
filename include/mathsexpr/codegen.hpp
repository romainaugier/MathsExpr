// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_CODEGEN)
#define __MATHSEXPR_CODEGEN

#include "mathsexpr/regalloc.hpp"
#include "mathsexpr/symtable.hpp"

#include <variant>

MATHSEXPR_NAMESPACE_BEGIN

/* Base asm instruction */

class MATHSEXPR_API Instr
{
public:
    virtual ~Instr() = default;

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept = 0;
    virtual void as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept = 0;
};

/* Mem related-instructions */

class MATHSEXPR_API InstrMov : public Instr
{
    MemLocPtr _mem_loc_from;
    MemLocPtr _mem_loc_to;

public:
    InstrMov(MemLocPtr& from, MemLocPtr& to) : _mem_loc_from(from),
                                               _mem_loc_to(to) {}

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;
    virtual void as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept override;
};

/* Unary ops instructions */

class MATHSEXPR_API InstrNeg : public Instr
{

};

/* Binary ops instructions */

class MATHSEXPR_API InstrAdd : public Instr
{
    MemLocPtr _left;
    MemLocPtr _right;

public:
    InstrAdd(MemLocPtr& left, MemLocPtr& right) : _left(left),
                                                  _right(right) {}

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;
    virtual void as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept override;
};

class MATHSEXPR_API InstrSub : public Instr
{
    MemLocPtr _left;
    MemLocPtr _right;

public:
    InstrSub(MemLocPtr& left, MemLocPtr& right) : _left(left),
                                                  _right(right) {}

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;
    virtual void as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept override;
};

class MATHSEXPR_API InstrMul : public Instr
{
    MemLocPtr _left;
    MemLocPtr _right;

public:
    InstrMul(MemLocPtr& left, MemLocPtr& right) : _left(left),
                                                  _right(right) {}

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;
    virtual void as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept override;
};

class MATHSEXPR_API InstrDiv : public Instr
{
    MemLocPtr _left;
    MemLocPtr _right;

public:
    InstrDiv(MemLocPtr& left, MemLocPtr& right) : _left(left),
                                                  _right(right) {}

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;
    virtual void as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept override;
};

/* Func ops instructions */

class MATHSEXPR_API InstrCall : public Instr
{
    std::string_view _call_name;

public:
    InstrCall(std::string_view call_name) : _call_name(call_name) {}

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;
    virtual void as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept override;
};

/* Terminator instructions */

class MATHSEXPR_API InstrRet : public Instr
{
public:

    virtual void as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept override;
    virtual void as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept override;
};

using InstrPtr = std::shared_ptr<Instr>;

class MATHSEXPR_API CodeGenerator
{
    std::vector<InstrPtr> _instructions;

public:
    CodeGenerator() {}

    bool build(const SSA& ssa,
               const RegisterAllocator& regalloc,
               SymbolTable& symtable) noexcept;

    void print(uint32_t isa, uint32_t platform) const noexcept;

    std::tuple<bool, ByteCode> as_bytecode(uint32_t isa, uint32_t platform) const noexcept;

    std::tuple<bool, std::string> as_string(uint32_t isa, uint32_t platform) const noexcept;
};

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_CODEGEN) */