// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/symtable.hpp"

#include <queue>

MATHSEXPR_NAMESPACE_BEGIN

void SymbolTable::print() const noexcept
{
    static std::ostream_iterator<char> out(std::cout);

    std::format_to(out, "SYMBOL TABLE\n");

    std::format_to(out, "VARIABLES ({}):\n", this->_variables.size());

    for(const auto& [name, variable] : this->_variables)
    {
        std::format_to(out, "    - {} (offset: {})\n", name, variable.get_id() * VALUE_OFFSET);
    }

    std::format_to(out, "LITERALS ({}):\n", this->_literals.size());

    for(const auto& [name, value] : this->_literals)
    {
        std::format_to(out,
                       "    - {} (={}, offset: {}))\n",
                       name,
                       value.get_value(),
                       value.get_id() * VALUE_OFFSET);
    }

    std::format_to(out, "FUNCTIONS ({}):\n", this->_functions.size());

    for(const auto& [name, nodes] : this->_functions)
    {
        std::format_to(out, "    - {} ({} calls)\n", name, nodes.size());
    }

    std::format_to(out, "\n");
}

void SymbolTable::clear() noexcept
{
    this->_variables.clear();
    this->_literals.clear();
}

void SymbolTable::collect(const AST& ast) noexcept
{
    size_t variable_id = 0;
    size_t literal_id = 0;

    auto pre_order_trav = [&](auto&& self, const ASTNode* current) -> void {
        if(current == nullptr)
        {
            return;
        }

        auto children = current->get_children();

        if(children.has_value())
        {
            for(auto& child : children.value())
            {
                self(self, child);
            }
        }

        if(auto current_variable = node_cast<ASTNodeVariable>(current))
        {
            if(!this->_variables.contains(current_variable->get_name()))
            {
                this->_variables[current_variable->get_name()] = SymbolVariable(current_variable->get_name(),
                                                                                variable_id++);
            }
        }
        else if(auto current_literal = node_cast<ASTNodeLiteral>(current))
        {
            if(!this->_literals.contains(current_literal->get_name()))
            {
                this->_literals[current_literal->get_name()] = SymbolLiteral(current_literal->get_value(),
                                                                             current_literal->get_name(),
                                                                             literal_id++);
            }
        }
        else if(auto current_function_call = node_cast<ASTNodeFunctionOp>(current))
        {
            this->_functions[current_function_call->get_function_name()].push_back(current_function_call);
        }
    };

    pre_order_trav(pre_order_trav, ast.get_root());
}

size_t SymbolTable::get_variable_offset(std::string_view variable_name) const noexcept
{
    auto it = this->_variables.find(variable_name);

    if(it == this->_variables.end())
    {
        return INVALID_OFFSET;
    }

    return it->second.get_offset();
}

size_t SymbolTable::get_literal_offset(std::string_view literal_name) const noexcept
{
    auto it = this->_literals.find(literal_name);

    if(it == this->_literals.end())
    {
        return INVALID_OFFSET;
    }

    return it->second.get_offset();
}

MATHSEXPR_NAMESPACE_END