// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/log.hpp"
#include "mathsexpr/expr.hpp"

#include "utils.hpp"

int main(int argc, char** argv)
{
    mathsexpr::set_log_level(mathsexpr::LogLevel::Debug);
    mathsexpr::log_info("Starting spill/load test");

    const char* expression = "(d / f) / ((c - e) / ((b / f) / ((a / b) - (((a - ((b - e) / ((c / e) / (a - f)))) / ((d - e) - (f - (a / b)))) - "
                             "((c - (d / f)) / (((e / b) - (f / a)) / (b - (c - d))))) / (c - d))))";

    mathsexpr::Expr expr(expression);

    if(!expr.compile(mathsexpr::ExprPrintFlags_PrintAll))
    {
        mathsexpr::log_error("Error while compiling expression");
        return 1;
    }

    double a = 10.0;
    double b = 9.0;
    double c = 8.0;
    double d = 7.0;
    double e = 6.0;
    double f = 5.0;

    auto [success, res] = expr.evaluate(d, f, c, e, b, a);

    if(!success)
    {
        mathsexpr::log_error("Error during expression evaluation");
        return false;
    }

    mathsexpr::log_info("expr \"{}\" evaluated: ({}, {}, {}, {}, {}, {}) = {}", 
                        expression,
                        a, b, c, d, e, f,
                        res);

    if(!DOUBLE_EQ(0.003968773703576324, res))
    {
        return 1;
    }

    mathsexpr::log_info("Finished spill/load test");

    return 0;
}
