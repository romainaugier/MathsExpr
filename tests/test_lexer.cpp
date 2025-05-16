// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/log.h"
#include "mathsexpr/lexer.h"

int main(int argc, char** argv)
{
    mathsexpr::log_info("Starting Lexer test");

    const char* expression = "log(2) * 1.0 / x + 4 * y - long_var_name";

    auto [success, tokens] = mathsexpr::lexer_lex_expression(expression);

    if(!success)
    {
        mathsexpr::log_error("Error while lexing expression: {}", expression);
        mathsexpr::log_error("Check the log for more information", expression);
        return 1;
    }

    mathsexpr::log_info("Tokens for expression: {}", expression);
    mathsexpr::lexer_print_tokens(tokens);

    mathsexpr::log_info("Finished Lexer test");

    return 0;
}
