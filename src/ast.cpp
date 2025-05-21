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

    for(const auto& argument : this->_arguments)
    {
        argument->print(out, indent + 1);
    }
}

const char* ast_unary_op_type_to_string(const uint32_t type) noexcept
{
    switch(type)
    {
        case UnaryOpType_Neg:
            return "-";
        default:
            return "?";
    }
}

uint32_t ast_unary_op_string_to_type(const std::string_view& data) noexcept
{
    if(data == "-")
    {
        return UnaryOpType_Neg;
    }

    return UnaryOpType_Unknown;
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
        case BinaryOpType_Add: 
            return "+";
        case BinaryOpType_Sub: 
            return "-";
        case BinaryOpType_Mul: 
            return "*";
        case BinaryOpType_Div: 
            return "/";
        default:
            return "?";
    }
}

uint32_t ast_binary_op_string_to_type(const std::string_view& data) noexcept
{
    if(data == "+") 
    {
        return BinaryOpType_Add;
    }
            
    if(data == "-") 
    {
        return BinaryOpType_Sub;
    }
            
    if(data == "*") 
    {
        return BinaryOpType_Mul;
    }
            
    if(data == "/") 
    {
        return BinaryOpType_Div;
    }

    return static_cast<uint32_t>(BinaryOpType_Unknown);
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

/*
    expression = term { ("+" | "-" , term };
    term = factor { ("*" | "/" | "%" , factor };
    factor = literal | symbol | function "(" expression ")" | "(" expression ")" | "-" factor;
*/

class Parser
{
    const LexerTokens& _tokens;

    std::string _error;

    size_t _index;

public:
    Parser(const LexerTokens& tokens) : _tokens(tokens), _index(0) {}

    MATHSEXPR_FORCE_INLINE void advance() noexcept { this->_index++; }

    MATHSEXPR_FORCE_INLINE bool is_at_end() const noexcept { return this->_index >= this->_tokens.size(); }

    MATHSEXPR_FORCE_INLINE const LexerToken& current() const noexcept
    {
        if(this->is_at_end())
        {
            return EMPTY_TOKEN;
        }

        return this->_tokens[this->_index];
    }

    MATHSEXPR_FORCE_INLINE const LexerToken& peek() const noexcept
    {
        if(this->_index >= (this->_tokens.size() - 2))
        {
            return EMPTY_TOKEN;
        }

        return this->_tokens[this->_index + 1];
    }

    std::shared_ptr<ASTNode> parse_factor() noexcept
    {
        switch(this->current().type)
        {
            case LexerTokenType::Literal:
            {
                const std::string_view& lit = this->current().data;
                double result;

                auto [ptr, ec] = std::from_chars(lit.data(), lit.data() + lit.size(), result);

                if(ec != std::errc())
                {
                    std::format_to(std::back_inserter(this->_error), "Unknown error caught while parsing a literal");
                    return nullptr;
                }

                this->advance();

                return std::make_shared<ASTNodeLiteral>(result, lit);
            }
            case LexerTokenType::Symbol:
            {
                const std::string_view name = this->current().data;

                if(this->peek().type == LexerTokenType::LParen)
                {
                    this->advance();

                    std::vector<std::shared_ptr<ASTNode>> arguments;

                    if(this->current().type != LexerTokenType::RParen)
                    {
                        arguments.push_back(std::move(this->parse_expression()));

                        while(this->current().type == LexerTokenType::Comma)
                        {
                            this->advance();
                            arguments.emplace_back(std::move(this->parse_expression()));
                        }
                    }

                    this->advance();

                    return std::make_shared<ASTNodeFunctionCall>(name, std::move(arguments));
                }
                else
                {
                    return std::make_shared<ASTNodeVariable>(name);
                }
            }
            case LexerTokenType::LParen:
            {
                this->advance();

                std::shared_ptr<ASTNode> expr = this->parse_expression();
                
                this->advance();

                return expr;
            }
            case LexerTokenType::Operator:
            {
                if(this->current().data != "-")
                {
                    std::format_to(std::back_inserter(this->_error),
                                   "Unexpected operator \"{}\" found when parsing unary op",
                                   this->current().data);

                    return nullptr;
                }

                return std::make_shared<ASTNodeUnaryOp>(this->parse_factor(), ast_unary_op_string_to_type(this->current().data));
            }
            default:
                std::format_to(std::back_inserter(this->_error),
                               "Unexpected token \"{}\" found when parsing factor",
                               lexer_token_type_to_string(this->current().type));

                return nullptr;
        }
    }

    std::shared_ptr<ASTNode> parse_term() noexcept
    {
        std::shared_ptr<ASTNode> left = this->parse_factor();

        while(this->current().type == LexerTokenType::Operator)
        {
            const uint32_t op = ast_binary_op_string_to_type(this->current().data);

            if(op != BinaryOpType_Mul && 
               op != BinaryOpType_Div)
            {
                break;
            }

            std::shared_ptr<ASTNode> right = this->parse_factor();

            left = std::make_shared<ASTNodeBinaryOp>(std::move(left), std::move(right), op);
        }

        return left;
    }

    std::shared_ptr<ASTNode> parse_expression() noexcept
    {
        std::shared_ptr<ASTNode> left = this->parse_term();

        while(this->current().type == LexerTokenType::Operator)
        {
            const uint32_t op = ast_binary_op_string_to_type(this->current().data);

            if(op != BinaryOpType_Add && 
               op != BinaryOpType_Sub)
            {
                break;
            }

            std::shared_ptr<ASTNode> right = this->parse_term();
            
            left = std::make_shared<ASTNodeBinaryOp>(std::move(left), std::move(right), op);
        }

        return left;
    }
};

bool AST::build_from_tokens(const LexerTokens& tokens) noexcept
{
    this->clear();

    Parser parser(tokens);

    this->_root = parser.parse_expression();

    if(this->_root == nullptr)
    {
        return false;
    }

    return true;
}

MATHSEXPR_NAMESPACE_END