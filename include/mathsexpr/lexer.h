// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_PARSER)
#define __MATHSEXPR_PARSER

#include "mathsexpr/mathsexpr.h"

#include <string_view>
#include <vector>

MATHSEXPR_NAMESPACE_BEGIN

enum MELexerTokenType : uint32_t 
{
    MESymbol,
    MELiteral,
    MEOperator,
    MELParen,
    MERParen,
};

using MELexerToken = std::pair<std::string_view, uint32_t>;
using MELexerTokens = std::vector<MELexerToken>;

static constexpr uint32_t LEXER_ERROR = std::numeric_limits<uint32_t>::max();

MATHSEXPR_API std::tuple<bool, MELexerTokens> lexer_lex_expression(std::string_view expression) noexcept;

MATHSEXPR_API uint32_t lexer_get_operator_precedence(char op) noexcept;

MATHSEXPR_API bool lexer_is_operator_right_associative(char op) noexcept;

MATHSEXPR_API void lexer_print_tokens(const MELexerTokens& tokens) noexcept;

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_PARSER) */