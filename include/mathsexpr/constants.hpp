// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_CONSTANTS)
#define __MATHSEXPR_CONSTANTS

#include "mathsexpr/mathsexpr.hpp"

#include <limits>

MATHSEXPR_NAMESPACE_BEGIN

static constexpr uint64_t UINT64_T_MAX = std::numeric_limits<uint64_t>::max();

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_CONSTANTS) */