// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_BYTECODE)
#define __MATHSEXPR_BYTECODE

#include "mathsexpr/mathsexpr.hpp"

#include <vector>
#include <string>
#include <format>

template<>
struct std::formatter<std::byte> {
    constexpr auto parse(std::format_parse_context& ctx) 
    {
        return ctx.begin();
    }
    
    auto format(std::byte b, std::format_context& ctx) const 
    {
        return std::format_to(ctx.out(), "{:02X}", static_cast<unsigned int>(b));
    }
};

MATHSEXPR_NAMESPACE_BEGIN

MATHSEXPR_FORCE_INLINE constexpr std::byte BYTE(auto b) noexcept { return static_cast<std::byte>(b); }

using ByteCode = std::vector<std::byte>;

void bytecode_as_hex_string(const ByteCode& bytecode, std::string& out) noexcept;

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_BYTECODE) */