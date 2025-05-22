// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/op.h"

MATHSEXPR_NAMESPACE_BEGIN

const char* op_unary_to_string(const uint32_t type) noexcept
{
    switch(type)
    {
        case UnaryOpType_Neg:
            return "-";
        default:
            return "?";
    }
}

uint32_t op_unary_from_string(const std::string_view& data) noexcept
{
    if(data == "-")
    {
        return UnaryOpType_Neg;
    }

    return UnaryOpType_Unknown;
}

const char* op_binary_to_string(const uint32_t type) noexcept
{
    switch(type)
    {
        case BinaryOpType_Add: 
            return "+";
        case BinaryOpType_Sub: 
            return "-";
        case BinaryOpType_Mul: 
            return "*";
        case BinaryOpType_Div: 
            return "/";
        default:
            return "?";
    }
}

uint32_t op_binary_from_string(const std::string_view& data) noexcept
{
    if(data == "+") 
    {
        return BinaryOpType_Add;
    }
            
    if(data == "-") 
    {
        return BinaryOpType_Sub;
    }
            
    if(data == "*") 
    {
        return BinaryOpType_Mul;
    }
            
    if(data == "/") 
    {
        return BinaryOpType_Div;
    }

    return static_cast<uint32_t>(BinaryOpType_Unknown);
}

MATHSEXPR_NAMESPACE_END