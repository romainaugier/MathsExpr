// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathexpr/log.hpp"
#include "mathexpr/expr.hpp"

#include "utils.hpp"

int main(int argc, char** argv)
{
    mathexpr::set_log_level(mathexpr::LogLevel::Debug);
    mathexpr::log_info("Starting sub_var test");

    const char* expression = "a - b";

    mathexpr::Expr expr(expression);

    if(!expr.compile(mathexpr::ExprPrintFlags_PrintAll))
    {
        mathexpr::log_error("Error while compiling expression");
        return 1;
    }

    double a = 4.0;
    double b = 18.0;

    auto [success, res] = expr.evaluate(a, b);

    if(!success)
    {
        mathexpr::log_error("Error during expression evaluation");
        return false;
    }

    mathexpr::log_info("expr \"{}\" evaluated: ({}, {}) = {}", 
                        expression,
                        a,
                        b,
                        res);

    if(!DOUBLE_EQ(res, -18.0))
    {
        return 1;
    }

    mathexpr::log_info("Finished sub_var test");

    return 0;
}
