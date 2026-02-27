// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_OP)
#define __MATHEXPR_OP

#include "mathexpr/common.hpp"

#include <string_view>

MATHEXPR_NAMESPACE_BEGIN

enum UnaryOpType : uint32_t
{
    UnaryOpType_Unknown,
    UnaryOpType_Neg,
};

MATHEXPR_API const char* op_unary_to_string(const uint32_t type) noexcept;

MATHEXPR_API uint32_t op_unary_from_string(const std::string_view& data) noexcept;

enum BinaryOpType : uint32_t 
{
    BinaryOpType_Unknown,
    BinaryOpType_Add,
    BinaryOpType_Sub,
    BinaryOpType_Mul,
    BinaryOpType_Div,
};

MATHEXPR_API const char* op_binary_to_string(const uint32_t type) noexcept;

MATHEXPR_API uint32_t op_binary_from_string(const std::string_view& data) noexcept;

MATHEXPR_API bool op_binary_is_commutative(const uint32_t type) noexcept;

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_OP) */