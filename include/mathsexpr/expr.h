// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_EXPR)
#define __MATHSEXPR_EXPR

#include "mathsexpr/mathsexpr.h"

#include <string>
#include <unordered_map>
#include <set>
#include <vector>

struct string_hash 
{
    using is_transparent = void;

    [[nodiscard]] size_t operator()(const char *txt) const 
    {
        return std::hash<std::string_view>{}(txt);
    }

    [[nodiscard]] size_t operator()(std::string_view txt) const 
    {
        return std::hash<std::string_view>{}(txt);
    }

    [[nodiscard]] size_t operator()(const std::string &txt) const 
    {
        return std::hash<std::string>{}(txt);
    }
};

MATHSEXPR_NAMESPACE_BEGIN

using Variables = std::unordered_map<std::string, double, string_hash, std::equal_to<>>;

class MATHSEXPR_API Expr
{
    std::string _expr;

    std::vector<std::byte> _byte_code;

    std::set<std::string_view> _variables;
    std::vector<double> _literals;

    std::tuple<bool, double> _evaluate_internal(const double* values) const noexcept;

public:
    Expr(std::string expr) : _expr(std::move(expr)) {}

    bool compile() noexcept;

    template<typename... Args>
        requires (std::same_as<std::remove_cvref_t<Args>, double> && ...)
    std::tuple<bool, double> evaluate(Args&&... args) const noexcept
    {
        if(sizeof...(Args) != this->_variables.size())
        {
            return std::make_tuple(false, 0.0);
        }

        std::array<double, sizeof...(Args)> values;

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            ((values[Is] = const_cast<double>(std::get<Is>(std::forward_as_tuple(std::forward<Args>(args))))), ...);
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