// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_BYTECODE)
#define __MATHSEXPR_BYTECODE

#include "mathsexpr/mathsexpr.hpp"

#include <vector>

MATHSEXPR_NAMESPACE_BEGIN

using ByteCode = std::vector<std::byte>;

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_BYTECODE) */