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
    EOF,
};

using LexerToken = std::pair<std::string_view, uint32_t>;
using LexerTokens = std::vector<LexerToken>;

static constexpr uint32_t LEXER_ERROR = std::numeric_limits<std::uint32_t>::max();

MATHSEXPR_API std::tuple<bool, LexerTokens> lexer_lex_expression(std::string_view expression) noexcept;

MATHSEXPR_API uint32_t lexer_get_operator_precedence(char op) noexcept;

MATHSEXPR_API bool lexer_is_operator_right_associative(char op) noexcept;

MATHSEXPR_API void lexer_print_tokens(const LexerTokens& tokens) noexcept;

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_LEXER) */