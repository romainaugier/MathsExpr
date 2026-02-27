// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/libmaths.hpp"
#include "mathexpr/string_hash.hpp"

#include <cmath>

MATHEXPR_NAMESPACE_BEGIN

LIBMATHS_NAMESPACE_BEGIN

/* Functions implementation */

/* Core mathematical functions */
/* Absolute value */
double abs_d(const double x) noexcept
{
    return ::fabs(x);
}

double2 abs_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 abs_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Square root */
double sqrt_d(const double x) noexcept
{
    return ::sqrt(x);
}

double2 sqrt_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 sqrt_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Cube root */
double cbrt_d(const double x) noexcept
{
    return ::cbrt(x);
}

double2 cbrt_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 cbrt_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Power function */
double pow_d(const double x, const double y) noexcept
{
    return ::pow(x, y);
}

double2 pow_d2(const double2 x, const double2 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 pow_d4(const double4 x, const double4 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Exponential function */
double exp_d(const double x) noexcept
{
    return ::exp(x);
}

double2 exp_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 exp_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* exp(x) - 1 */
double expm1_d(const double x) noexcept
{
    return ::expm1(x);
}

double2 expm1_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 expm1_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Natural logarithm */
double log_d(const double x) noexcept
{
    return ::log(x);
}

double2 log_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 log_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Base-10 logarithm */
double log10_d(const double x) noexcept
{
    return log10(x);
}

double2 log10_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 log10_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Base-2 logarithm */
double log2_d(const double x) noexcept
{
    return log2(x);
}

double2 log2_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 log2_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* log(1 + x) */
double log1p_d(const double x) noexcept
{
    return ::log1p(x);
}

double2 log1p_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 log1p_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Trigonometric functions */
/* Sine */
double sin_d(const double x) noexcept
{
    return ::sin(x);
}

double2 sin_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 sin_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Cosine */
double cos_d(const double x) noexcept
{
    return ::cos(x);
}

double2 cos_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 cos_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Tangent */
double tan_d(const double x) noexcept
{
    return ::tan(x);
}

double2 tan_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 tan_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Arcsine */
double asin_d(const double x) noexcept
{
    return ::asin(x);
}

double2 asin_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 asin_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Arccosine */
double acos_d(const double x) noexcept
{
    return ::acos(x);
}

double2 acos_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 acos_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Arctangent */
double atan_d(const double x) noexcept
{
    return ::atan(x);
}

double2 atan_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 atan_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Arctangent with two arguments */
double atan2_d(const double y, const double x) noexcept
{
    return ::atan2(y, x);
}

double2 atan2_d2(const double2 y, const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 atan2_d4(const double4 y, const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Hyperbolic functions */
/* Hyperbolic sine */
double sinh_d(const double x) noexcept
{
    return ::sinh(x);
}

double2 sinh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 sinh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Hyperbolic cosine */
double cosh_d(const double x) noexcept
{
    return ::cosh(x);
}

double2 cosh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 cosh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Hyperbolic tangent */
double tanh_d(const double x) noexcept
{
    return ::tanh(x);
}

double2 tanh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 tanh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Inverse hyperbolic sine */
double asinh_d(const double x) noexcept
{
    return asinh(x);
}

double2 asinh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 asinh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Inverse hyperbolic cosine */
double acosh_d(const double x) noexcept
{
    return ::acosh(x);
}

double2 acosh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 acosh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Inverse hyperbolic tangent */
double atanh_d(const double x) noexcept
{
    return ::atanh(x);
}

double2 atanh_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 atanh_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Rounding and modulo */
/* Floor function */
double floor_d(const double x) noexcept
{
    return ::floor(x);
}

double2 floor_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 floor_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Ceiling function */
double ceil_d(const double x) noexcept
{
    return ::ceil(x);
}

double2 ceil_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 ceil_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Truncate */
double trunc_d(const double x) noexcept
{
    return ::trunc(x);
}

double2 trunc_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 trunc_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Round to nearest */
double round_d(const double x) noexcept
{
    return ::round(x);
}

double2 round_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 round_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Floating-point remainder */
double fmod_d(const double x, const double y) noexcept
{
    return ::fmod(x, y);
}

double2 fmod_d2(const double2 x, const double2 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 fmod_d4(const double4 x, const double4 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* IEEE remainder */
double remainder_d(const double x, const double y) noexcept
{
    return ::remainder(x, y);
}

double2 remainder_d2(const double2 x, const double2 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 remainder_d4(const double4 x, const double4 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Copy sign from y to x */
double copysign_d(const double x, const double y) noexcept
{
    return copysign(x, y);
}

double2 copysign_d2(const double2 x, const double2 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 copysign_d4(const double4 x, const double4 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Miscellaneous */
/* Hypotenuse sqrt(x*x + y*y) */
double hypot_d(const double x, const double y) noexcept
{
    return hypot(x, y);
}

double2 hypot_d2(const double2 x, const double2 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 hypot_d4(const double4 x, const double4 y) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Convert degrees to radians */
double radians_d(const double x) noexcept
{
    return x;
}

double2 radians_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 radians_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Convert radians to degrees */
double degrees_d(const double x) noexcept
{
    return x;
}

double2 degrees_d2(const double2 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

double4 degrees_d4(const double4 x) noexcept
{
    MATHEXPR_NOT_IMPLEMENTED;
}

/* Function table */

#define REGISTER_FUNCTION(name, base, arity) \
    { name, { \
        reinterpret_cast<void*>(static_cast<Fn##arity##_d>(&base##_d)), \
        reinterpret_cast<void*>(static_cast<Fn##arity##_d2>(&base##_d2)), \
        reinterpret_cast<void*>(static_cast<Fn##arity##_d2>(&base##_d2)), \
        arity \
    } }

using FuncTable = std::unordered_map<std::string,
                                     FunctionEntry,
                                     string_hash,
                                     std::equal_to<>>;

static const FuncTable g_function_table = {
    REGISTER_FUNCTION("abs", abs, 1),
    REGISTER_FUNCTION("sqrt", sqrt, 1),
    REGISTER_FUNCTION("cbrt", cbrt, 1),
    REGISTER_FUNCTION("pow", pow, 2),
    REGISTER_FUNCTION("exp", exp, 1),
    REGISTER_FUNCTION("expm1", expm1, 1),
    REGISTER_FUNCTION("log", log, 1),
    REGISTER_FUNCTION("log10", log10, 1),
    REGISTER_FUNCTION("log2", log2, 1),
    REGISTER_FUNCTION("log1p", log1p, 1),

    REGISTER_FUNCTION("sin", sin, 1),
    REGISTER_FUNCTION("cos", cos, 1),
    REGISTER_FUNCTION("tan", tan, 1),
    REGISTER_FUNCTION("asin", asin, 1),
    REGISTER_FUNCTION("acos", acos, 1),
    REGISTER_FUNCTION("atan", atan, 1),
    REGISTER_FUNCTION("atan2", atan2, 2),

    REGISTER_FUNCTION("sinh", sinh, 1),
    REGISTER_FUNCTION("cosh", cosh, 1),
    REGISTER_FUNCTION("tanh", tanh, 1),
    REGISTER_FUNCTION("asinh", asinh, 1),
    REGISTER_FUNCTION("acosh", acosh, 1),
    REGISTER_FUNCTION("atanh", atanh, 1),

    REGISTER_FUNCTION("floor", floor, 1),
    REGISTER_FUNCTION("ceil", ceil, 1),
    REGISTER_FUNCTION("trunc", trunc, 1),
    REGISTER_FUNCTION("fmod", fmod, 2),
    REGISTER_FUNCTION("remainder", remainder, 2),
    REGISTER_FUNCTION("copysign", copysign, 2),

    REGISTER_FUNCTION("hypot", hypot, 2),
    REGISTER_FUNCTION("radians", radians, 1),
    REGISTER_FUNCTION("degrees", degrees, 1),
};

const FunctionEntry* get_function_entry(const std::string_view& name) noexcept
{
    auto entry = g_function_table.find(name);

    if(entry == g_function_table.end())
        return nullptr;

    return std::addressof(entry->second);
}

LIBMATHS_NAMESPACE_END

MATHEXPR_NAMESPACE_END
