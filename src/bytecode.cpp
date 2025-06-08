// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/bytecode.hpp"

MATHSEXPR_NAMESPACE_BEGIN

void bytecode_as_hex_string(const ByteCode& bytecode, std::string& out) noexcept
{
    out.clear();
    out.reserve(bytecode.size() * 2);

    for(const auto& byte : bytecode)
    {
        std::format_to(std::back_inserter(out), "{}", byte);
    }
}

MATHSEXPR_NAMESPACE_END