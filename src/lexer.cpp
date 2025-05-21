// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/lexer.h"
#include "mathsexpr/log.h"

#include <cctype>
#include <format>
#include <iostream>

MATHSEXPR_NAMESPACE_BEGIN

/* Lexing utils */

MATHSEXPR_FORCE_INLINE bool is_operator(unsigned int c)
{
    return (c == '+') |
           (c == '-') |
           (c == '*') |
           (c == '/');
}

MATHSEXPR_FORCE_INLINE bool is_paren(unsigned int c)
{
    return (c == '(') | (c == ')');
}

MATHSEXPR_FORCE_INLINE bool is_comma(unsigned int c)
{
    return c == ',';
}

MATHSEXPR_FORCE_INLINE uint32_t consume_literal(std::string_view s)
{
    const std::string_view orig = s;

    uint32_t start = 0;
    bool found_dot = false;

    while(!s.empty() && (std::isdigit(static_cast<int>(s.front())) || s.front() == '.'))
    {
        if(s.front() == '.')
        {
            if(found_dot)
            {
                log_error("Ill-formed literal: {}", orig.substr(0, start + 1));
                return LEXER_ERROR;
            }
        
            found_dot = true;
        }

        start++;
        s.remove_prefix(1);
    }

    return start;
}

MATHSEXPR_FORCE_INLINE uint32_t consume_symbol(std::string_view s)
{
    uint32_t start = 0;

    while(!s.empty() && (std::isalnum(static_cast<int>(s.front())) || s.front() == '_'))
    {
        start++;
        s.remove_prefix(1);
    }

    return start;
}

std::tuple<bool, LexerTokens> lexer_lex_expression(std::string_view expression) noexcept
{
    LexerTokens tokens;

    while(!expression.empty())
    {
        /* Literal */
        if(std::isdigit(static_cast<int>(expression.front())))
        {
            const uint32_t lit_size = consume_literal(expression);
            
            if(lit_size == LEXER_ERROR)
            {
                return std::make_tuple(false, std::move(tokens));
            }

            tokens.emplace_back(expression.substr(0, lit_size), LexerTokenType::Literal);

            expression.remove_prefix(lit_size);
        }
        /* Symbol */
        else if(std::isalpha(static_cast<int>(expression.front())))
        {
            const uint32_t sym_size = consume_symbol(expression);

            if(sym_size == LEXER_ERROR)
            {
                return std::make_tuple(false, std::move(tokens));
            }

            tokens.emplace_back(expression.substr(0, sym_size), LexerTokenType::Symbol);

            expression.remove_prefix(sym_size);
        }
        /* Operator */
        else if(is_operator(static_cast<int>(expression.front())))
        {
            tokens.emplace_back(expression.substr(0, 1), LexerTokenType::Operator);

            expression.remove_prefix(1);
        }
        /* Paren */
        else if(is_paren(static_cast<int>(expression.front())))
        {
            tokens.emplace_back(expression.substr(0, 1), 
                                expression.front() == '(' ? LexerTokenType::LParen : 
                                                            LexerTokenType::RParen);

            expression.remove_prefix(1);
        }
        /* Comma (in function calls) */
        else if(is_comma(static_cast<int>(expression.front())))
        {
            tokens.emplace_back(expression.substr(0, 1), LexerTokenType::Comma);

            expression.remove_prefix(1);
        }
        else
        {
            expression.remove_prefix(1);
        }
    }

    return std::make_tuple(true, std::move(tokens));
}

uint32_t lexer_get_operator_precedence(char op) noexcept
{
    switch(op) 
    {
        case '*':
        case '/':
            return 3;
        case '+':
        case '-':         
            return 2;
        default:
            return 0;
    }
}

bool lexer_is_operator_right_associative(char op) noexcept
{
    return op == '^';
}

const char* lexer_token_type_to_string(const uint32_t type) noexcept
{
    switch(type)
    {
        case Symbol: 
            return "SYMBOL";
        case Literal: 
            return "LITERAL";
        case Operator: 
            return "OPERATOR";
        case LParen: 
            return "LPAREN";
        case RParen: 
            return "RPAREN";
        case EndOfFile:
            return "ENDOFFILE";
        case Comma:
            return "COMMA";
        case Empty:
            return "EMPTY";
        default:
            return "UNKNOWN";
    }
}

void lexer_print_tokens(const LexerTokens& tokens) noexcept
{
    static std::ostream_iterator<char> out(std::cout);

    for(const auto& [view, type] : tokens)
    {
        std::format_to(out, "{}: {}\n", lexer_token_type_to_string(type), view);
    }
}

MATHSEXPR_NAMESPACE_END