// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR)
#define __MATHSEXPR

#if defined(_MSC_VER)
#define MATHSEXPR_MSVC
#pragma warning(disable : 4711) /* function selected for automatic inline expansion */
#define _SILENCE_ALL_MS_EXT_DEPRECATION_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#elif defined(__GNUC__)
#define MATHSEXPR_GCC
#elif defined(__clang__)
#define MATHSEXPR_CLANG
#endif /* defined(_MSC_VER) */

#define MATHSEXPR_STRIFY(x) #x
#define MATHSEXPR_STRIFY_MACRO(m) MATHSEXPR_STRIFY(m)

#if !defined(MATHSEXPR_VERSION_MAJOR)
#define MATHSEXPR_VERSION_MAJOR 0
#endif /* !defined(MATHSEXPR_VERSION_MAJOR) */

#if !defined(MATHSEXPR_VERSION_MINOR)
#define MATHSEXPR_VERSION_MINOR 0
#endif /* !defined(MATHSEXPR_VERSION_MINOR) */

#if !defined(MATHSEXPR_VERSION_PATCH)
#define MATHSEXPR_VERSION_PATCH 0
#endif /* !defined(MATHSEXPR_VERSION_PATCH) */

#if !defined(MATHSEXPR_VERSION_REVISION)
#define MATHSEXPR_VERSION_REVISION 0
#endif /* !defined(MATHSEXPR_VERSION_REVISION) */

#define MATHSEXPR_VERSION_STR                                                                      \
    MATHSEXPR_STRIFY_MACRO(MATHSEXPR_VERSION_MAJOR)                                                \
    "." MATHSEXPR_STRIFY_MACRO(MATHSEXPR_VERSION_MINOR) "." MATHSEXPR_STRIFY_MACRO(                \
        MATHSEXPR_VERSION_PATCH) "." MATHSEXPR_STRIFY_MACRO(MATHSEXPR_VERSION_REVISION)

#include <cassert>
#include <cstddef>
#include <cstdint>

#if INTPTR_MAX == INT64_MAX || defined(__x86_64__)
#define MATHSEXPR_X64
#define MATHSEXPR_SIZEOF_PTR 8
#elif INTPTR_MAX == INT32_MAX
#define MATHSEXPR_X86
#define MATHSEXPR_SIZEOF_PTR 4
#endif /* INTPTR_MAX == INT64_MAX || defined(__x86_64__) */

#if defined(_WIN32)
#define MATHSEXPR_WIN
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif /* !defined(WIN32_LEAN_AND_MEAN) */
#if defined(MATHSEXPR_X64)
#define MATHSEXPR_PLATFORM_STR "WIN64"
#else
#define MATHSEXPR_PLATFORM_STR "WIN32"
#endif /* defined(MATHSEXPR_x64) */
#elif defined(__linux__)
#define MATHSEXPR_LINUX
#if defined(MATHSEXPR_X64)
#define MATHSEXPR_PLATFORM_STR "LINUX64"
#else
#define MATHSEXPR_PLATFORM_STR "LINUX32"
#endif /* defined(MATHSEXPR_X64) */
#endif /* defined(_WIN32) */

#if defined(MATHSEXPR_WIN)
#if defined(MATHSEXPR_MSVC)
#define MATHSEXPR_EXPORT __declspec(dllexport)
#define MATHSEXPR_IMPORT __declspec(dllimport)
#elif defined(MATHSEXPR_GCC) || defined(MATHSEXPR_CLANG)
#define MATHSEXPR_EXPORT __attribute__((dllexport))
#define MATHSEXPR_IMPORT __attribute__((dllimport))
#endif /* defined(MATHSEXPR_MSVC) */
#elif defined(MATHSEXPR_LINUX)
#define MATHSEXPR_EXPORT __attribute__((visibility("default")))
#define MATHSEXPR_IMPORT
#endif /* defined(MATHSEXPR_WIN) */

#if defined(MATHSEXPR_MSVC)
#define MATHSEXPR_FORCE_INLINE __forceinline
#define MATHSEXPR_LIB_ENTRY
#define MATHSEXPR_LIB_EXIT
#elif defined(MATHSEXPR_GCC)
#define MATHSEXPR_FORCE_INLINE inline __attribute__((always_inline))
#define MATHSEXPR_LIB_ENTRY __attribute__((constructor))
#define MATHSEXPR_LIB_EXIT __attribute__((destructor))
#elif defined(MATHSEXPR_CLANG)
#define MATHSEXPR_FORCE_INLINE __attribute__((always_inline))
#define MATHSEXPR_LIB_ENTRY __attribute__((constructor))
#define MATHSEXPR_LIB_EXIT __attribute__((destructor))
#endif /* defined(MATHSEXPR_MSVC) */

#if __cplusplus > 201703
#define MATHSEXPR_MAYBE_UNUSED [[maybe_unused]]
#else
#define MATHSEXPR_MAYBE_UNUSED
#endif /* */

#if defined(MATHSEXPR_BUILD_SHARED)
#define MATHSEXPR_API MATHSEXPR_EXPORT
#else
#define MATHSEXPR_API MATHSEXPR_IMPORT
#endif /* defined(MATHSEXPR_BUILD_SHARED) */

#if defined __cplusplus
#define MATHSEXPR_CPP_ENTER                                                                        \
    extern "C"                                                                                     \
    {
#define MATHSEXPR_CPP_END }
#else
#define MATHSEXPR_CPP_ENTER
#define MATHSEXPR_CPP_END
#endif /* DEFINED __cplusplus */

#if !defined NULL
#define NULL (void*)0
#endif /* !defined NULL */

#if defined(MATHSEXPR_WIN)
#define MATHSEXPR_FUNCTION __FUNCTION__
#elif defined(MATHSEXPR_GCC) || defined(MATHSEXPR_CLANG)
#define MATHSEXPR_FUNCTION __PRETTY_FUNCTION__
#endif /* MATHSEXPR_WIN */

#define CONCAT_(prefix, suffix) prefix##suffix
#define CONCAT(prefix, suffix) CONCAT_(prefix, suffix)

#define MATHSEXPR_ASSERT(expr, message)                                                            \
    if(!(expr))                                                                                    \
    {                                                                                              \
        std::fprintf(stderr,                                                                       \
                     "Assertion failed in file %s at line %d: %s",                                 \
                     __FILE__,                                                                     \
                     __LINE__,                                                                     \
                     message);                                                                     \
        std::abort();                                                                              \
    }

#define MATHSEXPR_STATIC_ASSERT(expr, message) static_assert(expr, message)
#define MATHSEXPR_NOT_IMPLEMENTED                                                                  \
    std::fprintf(stderr,                                                                           \
                 "Called function %s that is not implemented (%s:%d)",                             \
                 ROMANORENDER_FUNCTION,                                                            \
                 __FILE__,                                                                         \
                 __LINE__);                                                                        \
    std::exit(1)

#define MATHSEXPR_NON_COPYABLE(__class__)                                                          \
    __class__(const __class__&) = delete;                                                          \
    __class__(__class__&&) = delete;                                                               \
    const __class__& operator=(const __class__&) = delete;                                         \
    void operator=(__class__&&) = delete;

#if defined(MATHSEXPR_MSVC)
#define MATHSEXPR_PACKED_STRUCT(__struct__) __pragma(pack(push, 1)) __struct__ __pragma(pack(pop))
#elif defined(MATHSEXPR_GCC) || defined(MATHSEXPR_CLANG)
#define MATHSEXPR_PACKED_STRUCT(__struct__) __struct__ __attribute__((__packed__))
#else
#define MATHSEXPR_PACKED_STRUCT(__struct__) __struct__
#endif /* defined(MATHSEXPR_MSVC) */

#if defined(MATHSEXPR_MSVC)
#define dump_struct(s)
#elif defined(MATHSEXPR_CLANG)
#define dump_struct(s) __builtin_dump_struct(s, printf)
#elif defined(MATHSEXPR_GCC)
#define dump_struct(s)
#endif /* defined(MATHSEXPR_MSVC) */

#if defined(DEBUG_BUILD)
#define MATHSEXPR_DEBUG 1
#else
#define MATHSEXPR_DEBUG 0
#endif /* defined(DEBUG_BUILD) */

#define MATHSEXPR_NAMESPACE_BEGIN                                                                  \
    namespace mathsexpr\
    {
#define MATHSEXPR_NAMESPACE_END }

#define MATHSEXPR_ATEXIT_REGISTER(func, do_exit)                                                   \
    int res_##func = std::atexit(func);                                                            \
    if(res_##func != 0)                                                                            \
    {                                                                                              \
        std::fprintf(stderr, "Cannot register function \"" #func "\" in atexit");                  \
        if(do_exit)                                                                                \
            std::exit(1);                                                                          \
    }

#endif /* !defined(__MATHSEXPR) */