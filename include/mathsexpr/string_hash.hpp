// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include <string>

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
