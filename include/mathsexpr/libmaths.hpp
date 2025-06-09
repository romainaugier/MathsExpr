// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_LIBMATHS)
#define __MATHSEXPR_LIBMATHS

#include "mathsexpr/simdtypes.hpp"

#include <unordered_map>
#include <string>

#define LIBMATHS_NAMESPACE_BEGIN namespace libmaths {
#define LIBMATHS_NAMESPACE_END }

MATHSEXPR_NAMESPACE_BEGIN

LIBMATHS_NAMESPACE_BEGIN

/* Core mathematical functions */
/* Absolute value */
MATHSEXPR_API double abs_d(const double x) noexcept;
MATHSEXPR_API double2 abs_d2(const double2 x) noexcept;
MATHSEXPR_API double4 abs_d4(const double4 x) noexcept;

/* Square root */
MATHSEXPR_API double sqrt_d(const double x) noexcept;
MATHSEXPR_API double2 sqrt_d2(const double2 x) noexcept;
MATHSEXPR_API double4 sqrt_d4(const double4 x) noexcept;

/* Cube root */
MATHSEXPR_API double cbrt_d(const double x) noexcept;
MATHSEXPR_API double2 cbrt_d2(const double2 x) noexcept;
MATHSEXPR_API double4 cbrt_d4(const double4 x) noexcept;

/* Power function */
MATHSEXPR_API double pow_d(const double x, const double y) noexcept;
MATHSEXPR_API double2 pow_d2(const double2 x, const double2 y) noexcept;
MATHSEXPR_API double4 pow_d4(const double4 x, const double4 y) noexcept;

/* Exponential function */
MATHSEXPR_API double exp_d(const double x) noexcept;
MATHSEXPR_API double2 exp_d2(const double2 x) noexcept;
MATHSEXPR_API double4 exp_d4(const double4 x) noexcept;

/* exp(x) - 1 */
MATHSEXPR_API double expm1_d(const double x) noexcept;
MATHSEXPR_API double2 expm1_d2(const double2 x) noexcept;
MATHSEXPR_API double4 expm1_d4(const double4 x) noexcept;

/* Natural logarithm */
MATHSEXPR_API double log_d(const double x) noexcept;
MATHSEXPR_API double2 log_d2(const double2 x) noexcept;
MATHSEXPR_API double4 log_d4(const double4 x) noexcept;

/* Base-10 logarithm */
MATHSEXPR_API double log10_d(const double x) noexcept;
MATHSEXPR_API double2 log10_d2(const double2 x) noexcept;
MATHSEXPR_API double4 log10_d4(const double4 x) noexcept;

/* Base-2 logarithm */
MATHSEXPR_API double log2_d(const double x) noexcept;
MATHSEXPR_API double2 log2_d2(const double2 x) noexcept;
MATHSEXPR_API double4 log2_d4(const double4 x) noexcept;

/* log(1 + x) */
MATHSEXPR_API double log1p_d(const double x) noexcept;
MATHSEXPR_API double2 log1p_d2(const double2 x) noexcept;
MATHSEXPR_API double4 log1p_d4(const double4 x) noexcept;

/* Trigonometric functions */
/* Sine */
MATHSEXPR_API double sin_d(const double x) noexcept;
MATHSEXPR_API double2 sin_d2(const double2 x) noexcept;
MATHSEXPR_API double4 sin_d4(const double4 x) noexcept;

/* Cosine */
MATHSEXPR_API double cos_d(const double x) noexcept;
MATHSEXPR_API double2 cos_d2(const double2 x) noexcept;
MATHSEXPR_API double4 cos_d4(const double4 x) noexcept;

/* Tangent */
MATHSEXPR_API double tan_d(const double x) noexcept;
MATHSEXPR_API double2 tan_d2(const double2 x) noexcept;
MATHSEXPR_API double4 tan_d4(const double4 x) noexcept;

/* Arcsine */
MATHSEXPR_API double asin_d(const double x) noexcept;
MATHSEXPR_API double2 asin_d2(const double2 x) noexcept;
MATHSEXPR_API double4 asin_d4(const double4 x) noexcept;

/* Arccosine */
MATHSEXPR_API double acos_d(const double x) noexcept;
MATHSEXPR_API double2 acos_d2(const double2 x) noexcept;
MATHSEXPR_API double4 acos_d4(const double4 x) noexcept;

/* Arctangent */
MATHSEXPR_API double atan_d(const double x) noexcept;
MATHSEXPR_API double2 atan_d2(const double2 x) noexcept;
MATHSEXPR_API double4 atan_d4(const double4 x) noexcept;

/* Arctangent with two arguments */
MATHSEXPR_API double atan2_d(const double y, const double x) noexcept;
MATHSEXPR_API double2 atan2_d2(const double2 y, const double2 x) noexcept;
MATHSEXPR_API double4 atan2_d4(const double4 y, const double4 x) noexcept;

/* Hyperbolic functions */
/* Hyperbolic sine */
MATHSEXPR_API double sinh_d(const double x) noexcept;
MATHSEXPR_API double2 sinh_d2(const double2 x) noexcept;
MATHSEXPR_API double4 sinh_d4(const double4 x) noexcept;

/* Hyperbolic cosine */
MATHSEXPR_API double cosh_d(const double x) noexcept;
MATHSEXPR_API double2 cosh_d2(const double2 x) noexcept;
MATHSEXPR_API double4 cosh_d4(const double4 x) noexcept;

/* Hyperbolic tangent */
MATHSEXPR_API double tanh_d(const double x) noexcept;
MATHSEXPR_API double2 tanh_d2(const double2 x) noexcept;
MATHSEXPR_API double4 tanh_d4(const double4 x) noexcept;

/* Inverse hyperbolic sine */
MATHSEXPR_API double asinh_d(const double x) noexcept;
MATHSEXPR_API double2 asinh_d2(const double2 x) noexcept;
MATHSEXPR_API double4 asinh_d4(const double4 x) noexcept;

/* Inverse hyperbolic cosine */
MATHSEXPR_API double acosh_d(const double x) noexcept;
MATHSEXPR_API double2 acosh_d2(const double2 x) noexcept;
MATHSEXPR_API double4 acosh_d4(const double4 x) noexcept;

/* Inverse hyperbolic tangent */
MATHSEXPR_API double atanh_d(const double x) noexcept;
MATHSEXPR_API double2 atanh_d2(const double2 x) noexcept;
MATHSEXPR_API double4 atanh_d4(const double4 x) noexcept;

/* Rounding and modulo */
/* Floor function */
MATHSEXPR_API double floor_d(const double x) noexcept;
MATHSEXPR_API double2 floor_d2(const double2 x) noexcept;
MATHSEXPR_API double4 floor_d4(const double4 x) noexcept;

/* Ceiling function */
MATHSEXPR_API double ceil_d(const double x) noexcept;
MATHSEXPR_API double2 ceil_d2(const double2 x) noexcept;
MATHSEXPR_API double4 ceil_d4(const double4 x) noexcept;

/* Truncate */
MATHSEXPR_API double trunc_d(const double x) noexcept;
MATHSEXPR_API double2 trunc_d2(const double2 x) noexcept;
MATHSEXPR_API double4 trunc_d4(const double4 x) noexcept;

/* Round to nearest */
MATHSEXPR_API double round_d(const double x) noexcept;
MATHSEXPR_API double2 round_d2(const double2 x) noexcept;
MATHSEXPR_API double4 round_d4(const double4 x) noexcept;

/* Floating-point remainder */
MATHSEXPR_API double fmod_d(const double x, const double y) noexcept;
MATHSEXPR_API double2 fmod_d2(const double2 x, const double2 y) noexcept;
MATHSEXPR_API double4 fmod_d4(const double4 x, const double4 y) noexcept;

/* IEEE remainder */
MATHSEXPR_API double remainder_d(const double x, const double y) noexcept;
MATHSEXPR_API double2 remainder_d2(const double2 x, const double2 y) noexcept;
MATHSEXPR_API double4 remainder_d4(const double4 x, const double4 y) noexcept;

/* Copy sign from y to x */
MATHSEXPR_API double copysign_d(const double x, const double y) noexcept;
MATHSEXPR_API double2 copysign_d2(const double2 x, const double2 y) noexcept;
MATHSEXPR_API double4 copysign_d4(const double4 x, const double4 y) noexcept;

/* Miscellaneous */
/* Hypotenuse sqrt(x*x + y*y) */
MATHSEXPR_API double hypot_d(const double x, const double y) noexcept;
MATHSEXPR_API double2 hypot_d2(const double2 x, const double2 y) noexcept;
MATHSEXPR_API double4 hypot_d4(const double4 x, const double4 y) noexcept;

/* Convert degrees to radians */
MATHSEXPR_API double radians_d(const double x) noexcept;
MATHSEXPR_API double2 radians_d2(const double2 x) noexcept;
MATHSEXPR_API double4 radians_d4(const double4 x) noexcept;

/* Convert radians to degrees */
MATHSEXPR_API double degrees_d(const double x) noexcept;
MATHSEXPR_API double2 degrees_d2(const double2 x) noexcept;
MATHSEXPR_API double4 degrees_d4(const double4 x) noexcept;

/* Functions table */

using Fn1_d = double (*)(double) noexcept;
using Fn2_d = double (*)(double, double) noexcept;
using Fn3_d = double (*)(double, double, double) noexcept;

using Fn1_d2 = double2 (*)(double2) noexcept;
using Fn2_d2 = double2 (*)(double2, double2) noexcept;
using Fn3_d2 = double2 (*)(double2, double2, double2) noexcept;

using Fn1_d4 = double4 (*)(double4) noexcept;
using Fn2_d4 = double4 (*)(double4, double4) noexcept;
using Fn3_d4 = double4 (*)(double4, double4, double4) noexcept;

struct FunctionEntry {
    void* scalar_ptr;
    void* vector2_ptr;
    void* vector4_ptr;
    size_t arity;
};

MATHSEXPR_API FunctionEntry* get_function_entry(const std::string& name) noexcept;

LIBMATHS_NAMESPACE_END

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_LIBMATHS) */