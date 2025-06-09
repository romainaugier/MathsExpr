// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_EXPR)
#define __MATHSEXPR_EXPR

#include "mathsexpr/constants.hpp"
#include "mathsexpr/execmem.hpp"
#include "mathsexpr/string_hash.hpp"

#include <string>
#include <unordered_map>
#include <set>
#include <vector>

MATHSEXPR_NAMESPACE_BEGIN

enum ExprPrintFlags : uint64_t
{
    ExprPrintFlags_PrintAST = 0x1,
    ExprPrintFlags_PrintSymTable = 0x2,
    ExprPrintFlags_PrintSSA = 0x4,
    ExprPrintFlags_PrintSSAOptimized = 0x8,
    ExprPrintFlags_PrintSSAOptimizationSteps = 0x10,
    ExprPrintFlags_PrintSSARegisterAlloc = 0x20,
    ExprPrintFlags_PrintCodeGeneratorAsString = 0x40,
    ExprPrintFlags_PrintCodeGeneratorByteCodeAsHexCode = 0x80,
    ExprPrintFlags_PrintAll = UINT64_T_MAX,
};

using Variables = std::unordered_map<std::string, double, string_hash, std::equal_to<>>;

class MATHSEXPR_API Expr
{
    std::string _expr;

    ExecMem _exec_mem;

    std::set<std::string_view> _variables;
    std::vector<double> _literals;

    std::tuple<bool, double> _evaluate_internal(const double* values) const noexcept;

public:
    Expr(std::string expr) : _expr(std::move(expr)) {}

    bool compile(uint64_t debug_flags = 0) noexcept;

    template<typename... Args>
        requires (std::same_as<std::remove_cvref_t<Args>, double> && ...)
    std::tuple<bool, double> evaluate(Args&&... args) const noexcept
    {
        if(sizeof...(Args) != this->_variables.size())
        {
            log_error("You passed {} arguments but the expression needs {}", 
                      sizeof...(Args),
                      this->_variables.size());

            return std::make_tuple(false, 0.0);
        }

        std::array<double, sizeof...(Args)> values;

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                ((values[Is] = static_cast<double>(args)), ...);
            }(std::make_index_sequence<sizeof...(Args)>{});

        return this->_evaluate_internal(values.data());
    }

    std::tuple<bool, double> evaluate(const Variables& variables) const noexcept
    {
        std::vector<double> values;
        values.reserve(this->_variables.size());

        for(const std::string_view& variable : this->_variables)
        {
            const auto it = variables.find(variable);

            if(it == variables.end())
            {
                return std::make_tuple(false, 0.0);
            }

            values.push_back(it->second);
        }

        return this->_evaluate_internal(values.data());
    }
};

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_EXPR) */