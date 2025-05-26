// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_SSA)
#define __MATHSEXPR_SSA

#include "mathsexpr/ast.h"

MATHSEXPR_NAMESPACE_BEGIN

enum SSAStmtTypeId : int
{
    SSAStmtTypeId_Variable = 1,
    SSAStmtTypeId_Literal = 2,
    SSAStmtTypeId_UnOp = 3,
    SSAStmtTypeId_BinOp = 4,
    SSAStmtTypeId_FuncOp = 5,
};

static constexpr char VERSION_CHAR = 't';

static constexpr uint64_t INVALID_VERSION = std::numeric_limits<uint64_t>::max();

class MATHSEXPR_API SSAStmt
{
    uint64_t _version;

public:
    SSAStmt(uint64_t version = INVALID_VERSION) : _version(version) {}

    virtual ~SSAStmt() = default;

    virtual void print(std::ostream_iterator<char>& out) const noexcept = 0;

    virtual uint64_t canonicalize() const noexcept = 0;

    virtual int type_id() const noexcept = 0;

    uint64_t get_version() const noexcept { return this->_version; }
};

using SSAStmtPtr = std::shared_ptr<SSAStmt>;

class MATHSEXPR_API SSAStmtVariable : public SSAStmt
{
    std::string_view _name;

public:
    SSAStmtVariable(std::string_view name, 
                    uint64_t version = INVALID_VERSION) : SSAStmt(version), 
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
                   uint64_t version = INVALID_VERSION) : SSAStmt(version), 
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
                uint64_t version = INVALID_VERSION) : SSAStmt(version), 
                                                      _operand(operand),
                                                      _op(op) {}

    virtual ~SSAStmtUnOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 3; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }
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
                 uint64_t version = INVALID_VERSION) : SSAStmt(version),
                                                       _left(left),
                                                       _right(right),
                                                       _op(op) {}

    virtual ~SSAStmtBinOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 4; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }
};

class MATHSEXPR_API SSAStmtFunctionOp : public SSAStmt
{
    std::vector<SSAStmtPtr> _arguments;

    std::string_view _name;

public:
    SSAStmtFunctionOp(std::string_view name,
                        std::vector<SSAStmtPtr> arguments,
                        uint64_t version = INVALID_VERSION) : SSAStmt(version),
                                                              _name(name),
                                                              _arguments(std::move(arguments)) {}

    virtual ~SSAStmtFunctionOp() override {}

    virtual void print(std::ostream_iterator<char>& out) const noexcept override;

    virtual uint64_t canonicalize() const noexcept override;

    static constexpr int static_type_id() { return 5; }

    virtual int type_id() const noexcept override { return this->static_type_id(); }

    std::string_view get_name() const noexcept { return this->_name; }
};

template<typename T>
const T* statement_cast(const SSAStmt* stmt) noexcept
{
    if(stmt != nullptr && stmt->type_id() == T::static_type_id())
    {
        return static_cast<const T*>(stmt);
    }

    return nullptr;
}

class MATHSEXPR_API SSA
{
    std::vector<SSAStmtPtr> _statements;

public:
    SSA() {}

    void print() const noexcept;

    bool build_from_ast(const AST& ast) noexcept;

    const std::vector<SSAStmtPtr>& get_statements() const noexcept { return this->_statements; }
};

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_SSA) */