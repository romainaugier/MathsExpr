// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_SIMDTYPES)
#define __MATHSEXPR_SIMDTYPES

#include "mathsexpr/mathsexpr.hpp"

#include <immintrin.h>

MATHSEXPR_NAMESPACE_BEGIN

using double2 = __m128d;
using double4 = __m256d;

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_SIMDTYPES) */