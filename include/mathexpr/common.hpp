// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR)
#define __MATHEXPR

#if defined(_MSC_VER)
#define MATHEXPR_MSVC
#pragma warning(disable : 4711) /* function selected for automatic inline expansion */
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#elif defined(__GNUC__)
#define MATHEXPR_GCC
#elif defined(__clang__)
#define MATHEXPR_CLANG
#endif /* defined(_MSC_VER) */

#define MATHEXPR_STRIFY(x) #x
#define MATHEXPR_STRIFY_MACRO(m) MATHEXPR_STRIFY(m)

#if !defined(MATHEXPR_VERSION_MAJOR)
#define MATHEXPR_VERSION_MAJOR 0
#endif /* !defined(MATHEXPR_VERSION_MAJOR) */

#if !defined(MATHEXPR_VERSION_MINOR)
#define MATHEXPR_VERSION_MINOR 0
#endif /* !defined(MATHEXPR_VERSION_MINOR) */

#if !defined(MATHEXPR_VERSION_PATCH)
#define MATHEXPR_VERSION_PATCH 0
#endif /* !defined(MATHEXPR_VERSION_PATCH) */

#if !defined(MATHEXPR_VERSION_REVISION)
#define MATHEXPR_VERSION_REVISION 0
#endif /* !defined(MATHEXPR_VERSION_REVISION) */

#define MATHEXPR_VERSION_STR                                                                      \
    MATHEXPR_STRIFY_MACRO(MATHEXPR_VERSION_MAJOR)                                                \
    "." MATHEXPR_STRIFY_MACRO(MATHEXPR_VERSION_MINOR) "." MATHEXPR_STRIFY_MACRO(                \
        MATHEXPR_VERSION_PATCH) "." MATHEXPR_STRIFY_MACRO(MATHEXPR_VERSION_REVISION)

#include <cassert>
#include <cstddef>
#include <cstdint>

#if INTPTR_MAX == INT64_MAX || defined(__x86_64__)
#define MATHEXPR_X64
#define MATHEXPR_SIZEOF_PTR 8
#elif INTPTR_MAX == INT32_MAX
#define MATHEXPR_X86
#define MATHEXPR_SIZEOF_PTR 4
#endif /* INTPTR_MAX == INT64_MAX || defined(__x86_64__) */

#if defined(__x86_64__)
#define MATHEXPR_X86_64
#elif defined(__aarch64__)
#define MATHEXPR_AARCH64
#endif /* defined(__x86_64__) */

#if defined(_WIN32)
#define MATHEXPR_WIN
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif /* !defined(WIN32_LEAN_AND_MEAN) */
#if !defined(NOMINMAX)
#define NOMINMAX
#endif /* !defined(NOMINMAX) */
#if defined(MATHEXPR_X64)
#define MATHEXPR_PLATFORM_STR "WIN64"
#else
#define MATHEXPR_PLATFORM_STR "WIN32"
#endif /* defined(MATHEXPR_x64) */
#elif defined(__linux__)
#define MATHEXPR_LINUX
#if defined(MATHEXPR_X64)
#define MATHEXPR_PLATFORM_STR "LINUX64"
#else
#define MATHEXPR_PLATFORM_STR "LINUX32"
#endif /* defined(MATHEXPR_X64) */
#endif /* defined(_WIN32) */

#if defined(MATHEXPR_WIN)
#if defined(MATHEXPR_MSVC)
#define MATHEXPR_EXPORT __declspec(dllexport)
#define MATHEXPR_IMPORT __declspec(dllimport)
#elif defined(MATHEXPR_GCC) || defined(MATHEXPR_CLANG)
#define MATHEXPR_EXPORT __attribute__((dllexport))
#define MATHEXPR_IMPORT __attribute__((dllimport))
#endif /* defined(MATHEXPR_MSVC) */
#elif defined(MATHEXPR_LINUX)
#define MATHEXPR_EXPORT __attribute__((visibility("default")))
#define MATHEXPR_IMPORT
#endif /* defined(MATHEXPR_WIN) */

#if defined(MATHEXPR_MSVC)
#define MATHEXPR_FORCE_INLINE __forceinline
#define MATHEXPR_LIB_ENTRY
#define MATHEXPR_LIB_EXIT
#elif defined(MATHEXPR_GCC)
#define MATHEXPR_FORCE_INLINE inline __attribute__((always_inline))
#define MATHEXPR_LIB_ENTRY __attribute__((constructor))
#define MATHEXPR_LIB_EXIT __attribute__((destructor))
#elif defined(MATHEXPR_CLANG)
#define MATHEXPR_FORCE_INLINE __attribute__((always_inline))
#define MATHEXPR_LIB_ENTRY __attribute__((constructor))
#define MATHEXPR_LIB_EXIT __attribute__((destructor))
#endif /* defined(MATHEXPR_MSVC) */

#if __cplusplus > 201703
#define MATHEXPR_MAYBE_UNUSED [[maybe_unused]]
#else
#define MATHEXPR_MAYBE_UNUSED
#endif /* */

#if defined(MATHEXPR_BUILD_SHARED)
#define MATHEXPR_API MATHEXPR_EXPORT
#else
#define MATHEXPR_API MATHEXPR_IMPORT
#endif /* defined(MATHEXPR_BUILD_SHARED) */

#if defined __cplusplus
#define MATHEXPR_CPP_ENTER                                                                        \
    extern "C"                                                                                     \
    {
#define MATHEXPR_CPP_END }
#else
#define MATHEXPR_CPP_ENTER
#define MATHEXPR_CPP_END
#endif /* DEFINED __cplusplus */

#if !defined NULL
#define NULL (void*)0
#endif /* !defined NULL */

#if defined(MATHEXPR_WIN)
#define MATHEXPR_FUNCTION __FUNCTION__
#elif defined(MATHEXPR_GCC) || defined(MATHEXPR_CLANG)
#define MATHEXPR_FUNCTION __PRETTY_FUNCTION__
#endif /* MATHEXPR_WIN */

#define CONCAT_(prefix, suffix) prefix##suffix
#define CONCAT(prefix, suffix) CONCAT_(prefix, suffix)

#define MATHEXPR_ASSERT(expr, message)                                                            \
    if(!(expr))                                                                                    \
    {                                                                                              \
        std::fprintf(stderr,                                                                       \
                     "Assertion failed in file %s at line %d: %s",                                 \
                     __FILE__,                                                                     \
                     __LINE__,                                                                     \
                     message);                                                                     \
        std::abort();                                                                              \
    }

#define MATHEXPR_STATIC_ASSERT(expr, message) static_assert(expr, message)
#define MATHEXPR_NOT_IMPLEMENTED                                                                  \
    std::fprintf(stderr,                                                                           \
                 "Called function %s that is not implemented (%s:%d)",                             \
                 MATHEXPR_FUNCTION,                                                               \
                 __FILE__,                                                                         \
                 __LINE__);                                                                        \
    std::exit(1)

#define MATHEXPR_NON_COPYABLE(__class__)                                                          \
    __class__(const __class__&) = delete;                                                          \
    __class__(__class__&&) = delete;                                                               \
    const __class__& operator=(const __class__&) = delete;                                         \
    void operator=(__class__&&) = delete;

#if defined(MATHEXPR_MSVC)
#define MATHEXPR_PACKED_STRUCT(__struct__) __pragma(pack(push, 1)) __struct__ __pragma(pack(pop))
#elif defined(MATHEXPR_GCC) || defined(MATHEXPR_CLANG)
#define MATHEXPR_PACKED_STRUCT(__struct__) __struct__ __attribute__((__packed__))
#else
#define MATHEXPR_PACKED_STRUCT(__struct__) __struct__
#endif /* defined(MATHEXPR_MSVC) */

#if defined(MATHEXPR_MSVC)
#define dump_struct(s)
#elif defined(MATHEXPR_CLANG)
#define dump_struct(s) __builtin_dump_struct(s, printf)
#elif defined(MATHEXPR_GCC)
#define dump_struct(s)
#endif /* defined(MATHEXPR_MSVC) */

#if defined(DEBUG_BUILD)
#define MATHEXPR_DEBUG 1
#else
#define MATHEXPR_DEBUG 0
#endif /* defined(DEBUG_BUILD) */

#define MATHEXPR_NAMESPACE_BEGIN                                                                  \
    namespace mathexpr\
    {
#define MATHEXPR_NAMESPACE_END }

#define MATHEXPR_ATEXIT_REGISTER(func, do_exit)                                                   \
    int res_##func = std::atexit(func);                                                            \
    if(res_##func != 0)                                                                            \
    {                                                                                              \
        std::fprintf(stderr, "Cannot register function \"" #func "\" in atexit");                  \
        if(do_exit)                                                                                \
            std::exit(1);                                                                          \
    }

#endif /* !defined(__MATHEXPR) */
