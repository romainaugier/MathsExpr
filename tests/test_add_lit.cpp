// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathexpr/log.hpp"
#include "mathexpr/expr.hpp"

#include "utils.hpp"

int main(int argc, char** argv)
{
    mathexpr::set_log_level(mathexpr::LogLevel::Debug);
    mathexpr::log_info("Starting add_lit test");

    const char* expression = "a + 4.0";

    mathexpr::Expr expr(expression);

    if(!expr.compile(mathexpr::ExprPrintFlags_PrintAll))
    {
        mathexpr::log_error("Error while compiling expression");
        return 1;
    }

    double a = 4.0;

    auto [success, res] = expr.evaluate(a);

    if(!success)
    {
        mathexpr::log_error("Error during expression evaluation");
        return false;
    }

    mathexpr::log_info("expr \"{}\" evaluated: ({}) = {}", 
                        expression,
                        a,
                        res);

    if(!DOUBLE_EQ(8.0, res))
    {
        return 1;
    }

    mathexpr::log_info("Finished add_lit test");

    return 0;
}
