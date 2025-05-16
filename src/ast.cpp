// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/ast.h"

#include <format>

MATHSEXPR_NAMESPACE_BEGIN

void ASTNodeVariable::print(std::ostream_iterator<char>& out, size_t indent) const noexcept
{
    std::format_to(out,
                   "{}VARIABLE: {}\n",
                   std::string(AST::PRINT_INDENT_SIZE * indent, ' '),
                   this->_name);
}

void ASTNodeLiteral::print(std::ostream_iterator<char>& out, size_t indent) const noexcept
{
    std::format_to(out,
                   "{}LITERAL: {}\n",
                   std::string(AST::PRINT_INDENT_SIZE * indent, ' '),
                   this->_value);
}

void ASTNodeFunctionCall::print(std::ostream_iterator<char>& out, size_t indent) const noexcept
{
    std::format_to(out, 
                   "{}FUNCTION CALL: {}({} arguments)\n", 
                   std::string(AST::PRINT_INDENT_SIZE * indent, ' '),
                   this->_name,
                   this->_arguments.size());

    for(const auto argument : this->_arguments)
    {
        argument->print(out, indent + 1);
    }
}

const char* ast_unary_op_type_to_string(const uint32_t type) noexcept
{
    switch(type)
    {
        case Neg:
            return "-";
        default:
            return "?";
    }
}

void ASTNodeUnaryOp::print(std::ostream_iterator<char>& out, size_t indent) const noexcept
{
    std::format_to(out,
                   "{}UNARY OP: {}\n",
                   std::string(AST::PRINT_INDENT_SIZE * indent, ' '),
                   ast_unary_op_type_to_string(this->_op));

    this->_operand->print(out, indent + 1);
}

const char* ast_binary_op_type_to_string(const uint32_t type) noexcept
{
    switch(type)
    {
        case Add: 
            return "+";
        case Sub: 
            return "-";
        case Mul: 
            return "*";
        case Div: 
            return "/";
        case Mod: 
            return "%";
        case Pow: 
            return "^";
        default:
            return "?";
    }
}

void ASTNodeBinaryOp::print(std::ostream_iterator<char>& out, size_t indent) const noexcept
{
    std::format_to(out,
                   "{}BINARY OP: {}\n",
                   std::string(AST::PRINT_INDENT_SIZE * indent, ' '),
                   ast_binary_op_type_to_string(this->_op));

    this->_left->print(out, indent + 1);
    this->_right->print(out, indent + 1);
}

void AST::print() const noexcept
{
    static std::ostream_iterator<char> out(std::cout);

    if(this->_root)
    {
        this->_root->print(out, 0);
    }
}

bool AST::build_from_tokens(const LexerTokens& tokens) noexcept
{
    this->clear();

    return true;
}

MATHSEXPR_NAMESPACE_END
