// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_SYMTABLE)
#define __MATHEXPR_SYMTABLE

#include "mathexpr/ast.hpp"

#include <string_view>
#include <map>
#include <format>

MATHEXPR_NAMESPACE_BEGIN

static constexpr size_t INVALID_SYMBOL_ID = std::numeric_limits<size_t>::max();

static constexpr size_t VALUE_OFFSET = sizeof(double);

static constexpr size_t INVALID_OFFSET = std::numeric_limits<size_t>::max();

class MATHEXPR_API Symbol 
{
    std::string_view _name;
    size_t _id;

public:
    Symbol(std::string_view name, size_t id) : _name(std::move(name)), _id(id) {}

    virtual ~Symbol() {}

    std::string_view get_name() const noexcept { return this->_name; }

    size_t get_id() const noexcept { return this->_id; }

    size_t get_offset() const noexcept 
    { 
        if(!this->valid())
        {
            return INVALID_OFFSET;
        }

        return this->_id * VALUE_OFFSET;
    }

    bool valid() const noexcept { return this->_id != INVALID_SYMBOL_ID; }
};

class MATHEXPR_API SymbolVariable : public Symbol
{
public:
    SymbolVariable() : Symbol("", INVALID_SYMBOL_ID) {}

    SymbolVariable(std::string_view name, size_t id) : Symbol(name, id) {}
};

class MATHEXPR_API SymbolLiteral : public Symbol
{
    double _value;

public:
    SymbolLiteral() : Symbol("", INVALID_SYMBOL_ID) {}

    SymbolLiteral(double value, std::string_view name, size_t id) : Symbol(name, id), _value(value) {}

    double get_value() const noexcept { return this->_value; }
};

class MATHEXPR_API SymbolTable
{
    std::map<std::string_view, SymbolVariable> _variables;

    std::map<std::string_view, SymbolLiteral> _literals;

    std::map<std::string_view, std::vector<const ASTNodeFunctionOp*>> _functions;

public:
    SymbolTable() {}

    ~SymbolTable() {}

    void print() const noexcept;

    void clear() noexcept;

    void collect(const AST& ast) noexcept;

    size_t get_variable_offset(std::string_view variable_name) const noexcept;

    size_t get_literal_offset(std::string_view literal_name) const noexcept;

    const std::map<std::string_view, SymbolVariable>& get_variables() const noexcept
    {
        return this->_variables;
    }

    const std::map<std::string_view, SymbolLiteral>& get_literals() const noexcept
    {
        return this->_literals;
    }
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_SYMTABLE) */
