// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_PARSER)
#define __MATHSEXPR_PARSER

#include "mathsexpr/mathsexpr.h"

#include "libromano/vector.h"

MATHSEXPR_CPP_ENTER

typedef enum {
    ParserTokenType_Literal,
    ParserTokenType_Variable,
    ParserTokenType_BinOperator,
    ParserTokenType_UnOperator,
    ParserTokenType_LParen,
    ParserTokenType_RParen,
    ParserTokenType_Function,
} ParserTokenType_;

typedef struct {
    char* start;
    uint32_t size;
    uint32_t token_type;
} ParserToken;

#define VARIABLE_LENGTH_MAX 8
#define FUNCTION_LENGTH_MAX 8

MATHSEXPR_API uint32_t mathsexpr_parser_parse(const char* expression,
                                              uint32_t expression_size,
                                              Vector* tokens);

MATHSEXPR_API uint32_t mathsexpr_parser_get_operator_precedence(char operator);

MATHSEXPR_API bool mathsexpr_is_operator_right_associative(char operator);

MATHSEXPR_API bool mathsexpr_parser_infix_to_postfix(Vector* infix_tokens,
                                                     Vector* postfix_tokens);

MATHSEXPR_API void mathsexpr_parser_debug_tokens(Vector* tokens);

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_PARSER) */