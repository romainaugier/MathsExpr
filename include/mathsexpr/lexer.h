// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_LEXER)
#define __MATHSEXPRLEXER_

#include "mathsexpr/mathsexpr.h"

#include <string_view>
#include <vector>
#include <limits>

MATHSEXPR_NAMESPACE_BEGIN

enum LexerTokenType : uint32_t 
{
    Symbol,
    Literal,
    Operator,
    LParen,
    RParen,
    Comma,
    EndOfFile,
    Empty,
};

struct LexerToken
{
    std::string_view data;
    uint32_t type;

    bool operator==(const LexerToken& other) const noexcept
    {
        return this->type == other.type && this->data == other.data;
    }

    bool empty() const noexcept
    {
        return this->type == LexerTokenType::Empty;
    }
};

using LexerTokens = std::vector<LexerToken>;

static const LexerToken EMPTY_TOKEN = { "", LexerTokenType::Empty };

static constexpr uint32_t LEXER_ERROR = std::numeric_limits<std::uint32_t>::max();

MATHSEXPR_API std::tuple<bool, LexerTokens> lexer_lex_expression(std::string_view expression) noexcept;

MATHSEXPR_API uint32_t lexer_get_operator_precedence(char op) noexcept;

MATHSEXPR_API bool lexer_is_operator_right_associative(char op) noexcept;

MATHSEXPR_API const char* lexer_token_type_to_string(const uint32_t type) noexcept;

MATHSEXPR_API void lexer_print_tokens(const LexerTokens& tokens) noexcept;

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_LEXER) */