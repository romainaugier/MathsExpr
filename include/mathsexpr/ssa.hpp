// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_SSA)
#define __MATHSEXPR_SSA

#include "mathsexpr/ast.hpp"
#include "mathsexpr/platform.hpp"

MATHSEXPR_NAMESPACE_BEGIN

enum SSAStmtTypeId : int
{
    SSAStmtTypeId_Variable = 1,
    SSAStmtTypeId_Literal = 2,
    SSAStmtTypeId_UnOp = 3,
    SSAStmtTypeId_BinOp = 4,
    SSAStmtTypeId_FuncOp = 5,
    SSAStmtTypeId_AllocateStackOp = 6,
    SSAStmtTypeId_SpillOp = 7,
    SSAStmtTypeId_LoadOp = 8,
};

static constexpr char VERSION_CHAR = 't';

static constexpr uint64_t INVALID_STMT_VERSION = std::numeric_limits<uint64_t>::max();
static constexpr uint64_t INVALID_STMT_REGISTER = std::numeric_limits<uint64_t>::max();

struct LiveRange {
    uint64_t start;
    uint64_t end;

    LiveRange(uint64_t start, uint64_t end) : start(start), end(end) {}
};

class MATHSEXPR_API SSAStmt
{
    uint64_t _version;
    uint64_t _register;

    LiveRange _range;

    uint64_t _frequency;

public:
    SSAStmt(uint64_t version = INVALID_STMT_VERSION, 
            uint64_t live_range_start = 0) : _version(version), 
                                             _range(live_range_start, 
                                                    live_range_start),
                                             _register(INVALID_STMT_REGISTER),
                                             _frequency(0) {}

    virtual ~SSAStmt() = default;

    virtual void print(std::ostream_iterator<char>& out) const noexcept = 0;

    virtual uint64_t canonicalize() const noexcept = 0;

    virtual int type_id() const noexcept = 0;

    uint64_t get_version() const noexcept { return this->_version; }

    void set_version(uint64_t version) noexcept { this->_version = version; }

    uint64_t get_register() const noexcept { return this->_register; }

    void set_register(uint64_t reg) noexcept { this->_register = reg; }

    LiveRange& get_live_range() noexcept { return this->_range; }

    const LiveRange& get_live_range() const noexcept { return this->_range; }

    void increment_frequency() noexcept { this->_frequency++; }

    uint64_t get_frequency() const noexcept { return this->_frequency; }
};

using SSAStmtPtr = std::shared_ptr<SSAStmt>;

class MATHSEXPR_API SSAStmtVariable : public SSAStmt
{
    std::string_view _name;

public:
    SSAStmtVariable(std::string_view name, 
                    uint64_t version = INVALID_STMT_VERSION,
                    uint64_t live_range_start = 0) : SSAStmt(version, live_range_start), 
                                                     _name(name) {}

    virtual ~SSAStmtVariable() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 1; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    std::string_view get_name() const noexcept { return this->_name; }
};

class MATHSEXPR_API SSAStmtLiteral : public SSAStmt
{
    std::string_view _name;

public:
    SSAStmtLiteral(std::string_view name,
                   uint64_t version = INVALID_STMT_VERSION,
                   uint64_t live_range_start = 0) : SSAStmt(version, live_range_start), 
                                                    _name(name) {}

    virtual ~SSAStmtLiteral() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 2; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    std::string_view get_name() const noexcept { return this->_name; }
};

class MATHSEXPR_API SSAStmtUnOp : public SSAStmt
{
    SSAStmtPtr _operand;

    uint32_t _op;

public:
    SSAStmtUnOp(SSAStmtPtr operand,
                uint32_t op,
                uint64_t version = INVALID_STMT_VERSION,
                uint64_t live_range_start = 0) : SSAStmt(version, live_range_start), 
                                                 _operand(operand),
                                                 _op(op) {}

    virtual ~SSAStmtUnOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 3; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    SSAStmtPtr& get_operand() noexcept { return this->_operand; }

    const SSAStmtPtr& get_operand() const noexcept { return this->_operand; }
};

class MATHSEXPR_API SSAStmtBinOp : public SSAStmt
{
    SSAStmtPtr _left;
    SSAStmtPtr _right;

    uint32_t _op;

public:
    SSAStmtBinOp(SSAStmtPtr left,
                 SSAStmtPtr right,
                 uint32_t op,
                 uint64_t version = INVALID_STMT_VERSION,
                 uint64_t live_range_start = 0) : SSAStmt(version, live_range_start),
                                                  _left(left),
                                                  _right(right),
                                                  _op(op) {}

    virtual ~SSAStmtBinOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 4; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    SSAStmtPtr& get_left() noexcept { return this->_left; }

    const SSAStmtPtr& get_left() const noexcept { return this->_left; }

    void set_left(SSAStmtPtr& left) noexcept { this->_left = left; }

    SSAStmtPtr& get_right() noexcept { return this->_right; }

    const SSAStmtPtr& get_right() const noexcept { return this->_right; }

    void set_right(SSAStmtPtr& right) noexcept { this->_right = right; }

    void swap_operands() noexcept { std::swap(this->_left, this->_right); }

    uint32_t get_op() const noexcept { return this->_op; }
};

class MATHSEXPR_API SSAStmtFunctionOp : public SSAStmt
{
    std::vector<SSAStmtPtr> _arguments;

    std::string_view _name;

public:
    SSAStmtFunctionOp(std::string_view name,
                      std::vector<SSAStmtPtr> arguments,
                      uint64_t version = INVALID_STMT_VERSION,
                      uint64_t live_range_start = 0) : SSAStmt(version, live_range_start),
                                                       _name(name),
                                                       _arguments(std::move(arguments)) {}

    virtual ~SSAStmtFunctionOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 5; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    std::string_view get_name() const noexcept { return this->_name; }

    std::vector<SSAStmtPtr>& get_arguments() noexcept { return this->_arguments; }
    
    const std::vector<SSAStmtPtr>& get_arguments() const noexcept { return this->_arguments; }
};

class MATHSEXPR_API SSAStmtAllocateStackOp : public SSAStmt
{
    uint64_t _size;

public:
    SSAStmtAllocateStackOp(uint64_t stack_size,
                           uint64_t version = INVALID_STMT_VERSION,
                           uint64_t live_range_start = 0) : SSAStmt(version),
                                                            _size(stack_size) {}

    virtual ~SSAStmtAllocateStackOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 6; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }
};

class MATHSEXPR_API SSAStmtSpillOp : public SSAStmt
{
    SSAStmtPtr _operand;

public:
    SSAStmtSpillOp(SSAStmtPtr operand,
                   uint64_t version = INVALID_STMT_VERSION,
                   uint64_t live_range_start = 0) : SSAStmt(version),
                                                    _operand(operand) {}

    virtual ~SSAStmtSpillOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 7; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    SSAStmtPtr& get_operand() noexcept { return this->_operand; }

    const SSAStmtPtr& get_operand() const noexcept { return this->_operand; }
};

class MATHSEXPR_API SSAStmtLoadOp : public SSAStmt
{
    SSAStmtPtr _spill;

public:
    SSAStmtLoadOp(SSAStmtPtr spill,
                  uint64_t version = INVALID_STMT_VERSION,
                  uint64_t live_range_start = 0) : SSAStmt(version),
                                                   _spill(spill) {}

    virtual ~SSAStmtLoadOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 8; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    SSAStmtPtr& get_spill() noexcept { return this->_spill; }

    const SSAStmtPtr& get_spill() const noexcept { return this->_spill; }
};

template<typename T>
const T* statement_const_cast(const SSAStmt* stmt) noexcept
{
    if(stmt != nullptr && stmt->type_id() == T::static_type_id())
    {
        return static_cast<const T*>(stmt);
    }

    return nullptr;
}

template<typename T>
T* statement_cast(SSAStmt* stmt) noexcept
{
    if(stmt != nullptr && stmt->type_id() == T::static_type_id())
    {
        return static_cast<T*>(stmt);
    }

    return nullptr;
}

MATHSEXPR_API bool ssa_statement_needs_register(const SSAStmt* stmt) noexcept;

class MATHSEXPR_API SSA
{
    std::vector<SSAStmtPtr> _statements;

    uint64_t get_statement_number() const noexcept { return this->_statements.size(); }

public:
    SSA() {}

    void print() const noexcept;

    bool build_from_ast(const AST& ast) noexcept;

    const std::vector<SSAStmtPtr>& get_statements() const noexcept { return this->_statements; }

    std::vector<SSAStmtPtr>& get_statements() noexcept { return this->_statements; }
};

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_SSA) */