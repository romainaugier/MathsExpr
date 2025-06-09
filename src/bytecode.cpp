// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/bytecode.hpp"

#include <ranges>

MATHSEXPR_NAMESPACE_BEGIN

void bytecode_as_hex_string(const ByteCode& bytecode,
                            std::string& out,
                            const std::unordered_set<std::byte>& prefixes) noexcept
{
    out.clear();
    out.reserve(bytecode.size() * 2);

    for(const auto [i, byte] : std::ranges::enumerate_view(bytecode))
    {
        if(i > 0 && prefixes.contains(byte))
        {
            std::format_to(std::back_inserter(out), "\n");
        }

        std::format_to(std::back_inserter(out), "{}", byte);
    }
}

MATHSEXPR_NAMESPACE_END