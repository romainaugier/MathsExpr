// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHSEXPR_PLATFORM)
#define __MATHSEXPR_PLATFORM

#include "mathsexpr/mathsexpr.hpp"

#include <limits>

MATHSEXPR_NAMESPACE_BEGIN

enum Platform : uint32_t
{
    Platform_Windows,
    Platform_Linux,
};

enum ISA : uint32_t
{
    ISA_x86_64,
};

static constexpr uint32_t INVALID_GP_REGISTER = std::numeric_limits<uint32_t>::max();

/* General Purpose Registers */
enum GpRegisters_x86_64 : uint32_t
{
    GpRegisters_x86_64_RAX,
    GpRegisters_x86_64_RBX,
    GpRegisters_x86_64_RCX,
    GpRegisters_x86_64_RDX,
    GpRegisters_x86_64_RSI,
    GpRegisters_x86_64_RDI,
    GpRegisters_x86_64_RBP,
    GpRegisters_x86_64_RSP,
    GpRegisters_x86_64_R8,
    GpRegisters_x86_64_R9,
    GpRegisters_x86_64_R10,
    GpRegisters_x86_64_R11,
    GpRegisters_x86_64_R12,
    GpRegisters_x86_64_R13,
    GpRegisters_x86_64_R14,
    GpRegisters_x86_64_R15,
};

MATHSEXPR_API const char* gp_register_x86_64_as_string(uint32_t reg) noexcept;

/* Floating Point Registers (omitting the upper 8-15 since not supported on abis) */
enum FpRegisters_x86_64 : uint32_t
{
    FpRegisters_x86_64_Xmm0,
    FpRegisters_x86_64_Xmm1,
    FpRegisters_x86_64_Xmm2,
    FpRegisters_x86_64_Xmm3,
    FpRegisters_x86_64_Xmm4,
    FpRegisters_x86_64_Xmm5,
    FpRegisters_x86_64_Xmm6,
    FpRegisters_x86_64_Xmm7,
};

/* base ptr for the variables values is passed as the first parameter */
MATHSEXPR_API uint32_t get_base_ptr_variable_register(uint32_t platform, uint32_t isa) noexcept;

/* base ptr for the variables values is passed as the second parameter */
MATHSEXPR_API uint32_t get_base_ptr_literal_register(uint32_t platform, uint32_t isa) noexcept;

MATHSEXPR_NAMESPACE_END

#endif /* !defined(__MATHSEXPR_PLATFORM) */