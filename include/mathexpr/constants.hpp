// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_CONSTANTS)
#define __MATHEXPR_CONSTANTS

#include "mathexpr/common.hpp"

#include <limits>

MATHEXPR_NAMESPACE_BEGIN

static constexpr uint64_t UINT64_T_MAX = std::numeric_limits<uint64_t>::max();

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_CONSTANTS) */