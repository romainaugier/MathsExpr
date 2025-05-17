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

/* Parsing */

class Parser
{
    const LexerTokens& _tokens;

    std::string _error;

    size_t _index;

public:
    Parser(const LexerTokens& tokens) : _tokens(tokens), _index(0) {}

    MATHSEXPR_FORCE_INLINE void advance() noexcept { this->_index++; }

    MATHSEXPR_FORCE_INLINE bool is_at_end() const noexcept { this->_index >= this->_tokens.size(); }

    MATHSEXPR_FORCE_INLINE std::optional<const LexerToken&> current() const noexcept
    {
        if(this->is_at_end())
        {
            return std::nullopt;
        }

        return this->_tokens[this->_index];
    }

    MATHSEXPR_FORCE_INLINE std::optional<const LexerToken&> peek() const noexcept
    {
        if(this->_index >= (this->_tokens.size() - 2))
        {
            return std::nullopt;
        }

        return this->_tokens[this->_index + 1];
    }

    std::unique_ptr<ASTNode> parse_expression() noexcept
    {
        std::unique_ptr<ASTNode> left;

        if(this->current().has_value() && this->current().value().second == LexerTokenType::Symbol)
        {
            if(this->peek().has_value() && this->peek().value().second == LexerTokenType::LParen)
            {
                left = this->parse_function_call();
            }
            else
            {
                left = std::make_unique<ASTNodeVariable>(this->current().value().first);
            }
        }
        else if(this->current().has_value() && this->current().value().second == LexerTokenType::Literal)
        {
            const std::string_view& lit = this->current().value().first;
            double result;

            auto [ptr, ec] = std::from_chars(lit.data(), lit.data() + lit.size(), result);

            if(ec != std::errc())
            {
                std::format_to(std::back_inserter(this->_error), "Unknown error caught while parsing a literal");
                return nullptr;
            }

            left = std::make_unique<ASTNodeLiteral>(result, lit);
        }
    }

    std::unique_ptr<ASTNode> parse_function_call() noexcept
    {
        if(this->current().has_value() && this->current().value().second != LexerTokenType::Symbol)
        {
            return nullptr;
        }

        std::string_view name = this->current().value().first;

        std::vector<ASTNode*> arguments;

        this->advance();

        while(!this->is_at_end() && this->current().value().second != LexerTokenType::RParen)
        {
            std::unique_ptr<ASTNode> expr = this->parse_expression();

            if(expr == nullptr)
            {
                return nullptr;
            }
        }
    }
};

bool AST::build_from_tokens(const LexerTokens& tokens) noexcept
{
    this->clear();

    Parser parser(tokens);

    this->_root = parser.parse_expression();

    return true;
}

MATHSEXPR_NAMESPACE_END
