// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_OP)
#define __MATHSEXPR_OP

#include "mathsexpr/mathsexpr.h"

#include <string_view>

MATHSEXPR_NAMESPACE_BEGIN

enum UnaryOpType : uint32_t
{
    UnaryOpType_Unknown,
    UnaryOpType_Neg,
};

MATHSEXPR_API const char* op_unary_to_string(const uint32_t type) noexcept;

MATHSEXPR_API uint32_t op_unary_from_string(const std::string_view& data) noexcept;

enum BinaryOpType : uint32_t 
{
    BinaryOpType_Unknown,
    BinaryOpType_Add,
    BinaryOpType_Sub,
    BinaryOpType_Mul,
    BinaryOpType_Div,
};

MATHSEXPR_API const char* op_binary_to_string(const uint32_t type) noexcept;

MATHSEXPR_API uint32_t op_binary_from_string(const std::string_view& data) noexcept;

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_OP) */