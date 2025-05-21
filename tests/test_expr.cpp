// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/log.h"
#include "mathsexpr/expr.h"

int main(int argc, char** argv)
{
    mathsexpr::log_info("Starting Expr test");

    const char* expression = "log(2) * 1.0 / x + 4 * y - long_var_name * pow(x, 4)";

    mathsexpr::Expr expr(expression);

    if(!expr.compile())
    {
        mathsexpr::log_error("Error while compiling expression");
        return 1;
    }

    mathsexpr::log_info("Finished Expr test");

    return 0;
}
