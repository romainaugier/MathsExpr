// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_EXPR)
#define __MATHSEXPR_EXPR

#include "mathsexpr/ast.h"

MATHSEXPR_NAMESPACE_BEGIN

class MATHSEXPR_API Expr
{
    std::string _expr;

    size_t _num_variables;
    

public:
    Expr(std::string expr) : _expr(std::move(expr)) {}

    bool compile() noexcept;

    template<typename... Args>
    std::tuple<bool, double> evaluate(Args&&... args) noexcept;
};

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_EXPR) */