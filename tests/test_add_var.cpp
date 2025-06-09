// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/log.hpp"
#include "mathsexpr/expr.hpp"

#include "utils.hpp"

int main(int argc, char** argv)
{
    mathsexpr::set_log_level(mathsexpr::LogLevel::Debug);
    mathsexpr::log_info("Starting add_var test");

    const char* expression = "a + b";

    mathsexpr::Expr expr(expression);

    if(!expr.compile(mathsexpr::ExprPrintFlags_PrintAll))
    {
        mathsexpr::log_error("Error while compiling expression");
        return 1;
    }

    double a = 4.0;
    double b = 18.0;

    auto [success, res] = expr.evaluate(a, b);

    if(!success)
    {
        mathsexpr::log_error("Error during expression evaluation");
        return false;
    }

    mathsexpr::log_info("expr \"{}\" evaluated: ({}, {}) = {}", 
                        expression,
                        a,
                        b,
                        res);

    if(!DOUBLE_EQ(22.0, res))
    {
        return 1;
    }

    mathsexpr::log_info("Finished add_var test");

    return 0;
}
