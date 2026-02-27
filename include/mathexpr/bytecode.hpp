// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_BYTECODE)
#define __MATHEXPR_BYTECODE

#include "mathexpr/common.hpp"

#include <vector>
#include <string>
#include <format>
#include <unordered_set>

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

MATHEXPR_NAMESPACE_BEGIN

MATHEXPR_FORCE_INLINE constexpr std::byte BYTE(auto b) noexcept { return static_cast<std::byte>(b); }

using ByteCode = std::vector<std::byte>;

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_BYTECODE) */