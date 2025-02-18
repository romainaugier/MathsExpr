// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/parser.h"

#include "libromano/logger.h"

#include <string.h>

#define NUM_EXPRS 7

int main(void)
{
    logger_init();

    const char* const exprs[NUM_EXPRS] = {
        "a * 3 + 57 * sqrt(b + 34)",
        "2 * x - 7",
        "4 * x ^ 2 + 7 * x + 2",
        "9 + 24 / (cos(7 - 3))",
        "(0.5 + 18 * x * sin(-34 - 4 * x)) ^ 0.5",
        "-3 + 6",
        "cos(tan(18 * x)) * a + 5.0 - b / (18 * x)"
    };

    for(uint32_t i = 0; i < NUM_EXPRS; i++)
    {
        if(i > 0)
        {
            printf("****************************************\n");
        }

        const size_t expr_size = strlen(exprs[i]);

        Vector* expr_tokens = vector_new(128, sizeof(ParserToken));

        if(mathsexpr_parser_parse(exprs[i], expr_size, expr_tokens) != 0)
        {
            logger_log_error("Error while parsing expression: %s", exprs[i]);

            vector_free(expr_tokens);
            logger_release();
            return 1;
        }

        printf("Expr%d: %s\n", i + 1, exprs[i]);
        printf("Expr%d: tokens\n", i + 1);
        mathsexpr_parser_debug_tokens(expr_tokens);

        vector_free(expr_tokens);
    }

    logger_release();

    return 0;
}