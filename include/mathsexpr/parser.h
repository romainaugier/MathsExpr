// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_PARSER)
#define __MATHSEXPR_PARSER

#include "mathsexpr/mathsexpr.h"
#include "mathsexpr/ast.h"

#include "libromano/vector.h"

MATHSEXPR_CPP_ENTER

typedef enum {
    ParserTokenType_Literal,
    ParserTokenType_Variable,
    ParserTokenType_Operator,
    ParserTokenType_LParen,
    ParserTokenType_RParen,
} ParserTokenType_;

typedef struct {
    char* start;
    uint32_t size;
    uint32_t token_type;
} ParserToken;

MATHSEXPR_API uint32_t mathsexpr_parse(const char* expression,
                                       uint32_t expression_size);

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_PARSER) */