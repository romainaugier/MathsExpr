// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/log.hpp"
#include "mathsexpr/expr.hpp"

int main(int argc, char** argv)
{
    mathsexpr::set_log_level(mathsexpr::LogLevel::Debug);
    mathsexpr::log_info("Starting add test");

    const char* expression = "a + b";

    mathsexpr::Expr expr(expression);

    if(!expr.compile(mathsexpr::ExprPrintFlags_PrintAll))
    {
        mathsexpr::log_error("Error while compiling expression");
        return 1;
    }

    mathsexpr::log_info("Finished Expr test");

    return 0;
}
