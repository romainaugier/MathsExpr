// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_SYMTABLE)
#define __MATHSEXPR_SYMTABLE

#include "mathsexpr/ast.h"

#include <string_view>
#include <unordered_map>
#include <format>

MATHSEXPR_NAMESPACE_BEGIN

class MATHSEXPR_API Symbol 
{
    std::string_view _name;
    size_t _id;

public:
    Symbol(std::string_view name, size_t id) : _name(std::move(name)), _id(id) {}

    virtual ~Symbol() {}

    std::string_view get_name() const noexcept { return this->_name; }

    size_t get_id() const noexcept { return this->_id; }
};

class MATHSEXPR_API SymbolVariable : public Symbol
{
public:
    SymbolVariable(std::string_view name, size_t id) : Symbol(name, id) {}
};

class MATHSEXPR_API SymbolLiteral : public Symbol
{
    double _value;

public:
    SymbolLiteral(double value, std::string_view name, size_t id) : Symbol(name, id), _value(value) {}

    double get_value() const noexcept { return this->_value; }
};

class MATHSEXPR_API SymbolTable
{
    std::unordered_map<std::string_view, SymbolVariable> _variables;

    std::unordered_map<std::string_view, SymbolLiteral> _literals;

    std::unordered_map<std::string_view, std::vector<const ASTNodeFunctionCall*>> _functions;

public:
    SymbolTable() {}

    ~SymbolTable() {}

    void print() const noexcept;

    void clear() noexcept;

    void collect(const AST& ast) noexcept;
};

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_SYMTABLE) */
