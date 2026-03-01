// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_SIMDTYPES)
#define __MATHEXPR_SIMDTYPES

#include "mathexpr/common.hpp"

#include <immintrin.h>

MATHEXPR_NAMESPACE_BEGIN

using float4 = __m128;
using float8 = __m256;

using double2 = __m128d;
using double4 = __m256d;

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_SIMDTYPES) */
