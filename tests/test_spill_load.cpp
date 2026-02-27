// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathexpr/log.hpp"
#include "mathexpr/expr.hpp"

#include "utils.hpp"

int main(int argc, char** argv)
{
    mathexpr::set_log_level(mathexpr::LogLevel::Debug);
    mathexpr::log_info("Starting spill/load test");

    const char* expression = "(d / f) / ((c - e) / ((b / f) / ((a / b) - (((a - ((b - e) / ((c / e) / (a - f)))) / ((d - e) - (f - (a / b)))) - "
                             "((c - (d / f)) / (((e / b) - (f / a)) / (b - (c - d))))) / (c - d))))";

    // const char* expression = "(d / f) / (c - e) / b * a";

    mathexpr::Expr expr(expression);

    if(!expr.compile(mathexpr::ExprPrintFlags_PrintAll))
    {
        mathexpr::log_error("Error while compiling expression");
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
        mathexpr::log_error("Error during expression evaluation");
        return false;
    }

    mathexpr::log_info("expr \"{}\" evaluated: ({}, {}, {}, {}, {}, {}) = {}", 
                        expression,
                        a, b, c, d, e, f,
                        res);

    if(!DOUBLE_EQ(0.003968773703576324, res))
    {
        return 1;
    }

    mathexpr::log_info("Finished spill/load test");

    return 0;
}
