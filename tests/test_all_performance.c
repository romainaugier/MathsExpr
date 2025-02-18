// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/parser.h"
#include "mathsexpr/ssa_optimize.h"

#include "libromano/logger.h"

#define ROMANO_ENABLE_PROFILING
#include "libromano/profiling.h"

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

        printf("Expr%d: %s\n", i + 1, exprs[i]);

        const size_t expr_size = strlen(exprs[i]);

        SCOPED_PROFILE_US_START(all_pipeline);

        SCOPED_PROFILE_US_START(parser);
        Vector* expr_tokens = vector_new(128, sizeof(ParserToken));

        if(mathsexpr_parser_parse(exprs[i], expr_size, expr_tokens) != 0)
        {
            logger_log_error("Error while parsing expression: %s", exprs[i]);

            vector_free(expr_tokens);
            logger_release();
            return 1;
        }
        SCOPED_PROFILE_US_END(parser);

        SCOPED_PROFILE_US_START(ast);
        AST* expr_ast = mathsexpr_ast_new();

        if(!mathsexpr_ast_from_infix_parser_tokens(expr_ast, expr_tokens))
        {
            logger_log_error("Error while building AST for expression: %s", exprs[i]);

            mathsexpr_ast_destroy(expr_ast);
            vector_free(expr_tokens);
            logger_release();
            return 1;
        }
        SCOPED_PROFILE_US_END(ast);

        SCOPED_PROFILE_US_START(ssa);
        SSA* expr_ssa = mathsexpr_ssa_new();

        if(!mathsexpr_ssa_from_ast(expr_ssa, expr_ast))
        {
            logger_log_error("Error while building SSA for expression: %s", exprs[i]);

            mathsexpr_ssa_destroy(expr_ssa);
            mathsexpr_ast_destroy(expr_ast);
            vector_free(expr_tokens);
            logger_release();

            return 1;
        }
        SCOPED_PROFILE_US_END(ssa);

        SCOPED_PROFILE_US_START(ssa_optimizations);
        mathsexpr_ssa_optimize(expr_ssa, SSAOptimizationFlags_All);
        SCOPED_PROFILE_US_END(ssa_optimizations);

        mathsexpr_ssa_destroy(expr_ssa);
        mathsexpr_ast_destroy(expr_ast);
        vector_free(expr_tokens);

        SCOPED_PROFILE_US_END(all_pipeline);
    }

    logger_release();

    return 0;
}