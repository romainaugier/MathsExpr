// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_PLATFORM)
#define __MATHEXPR_PLATFORM

#include "mathexpr/common.hpp"

#include <limits>
#include <vector>

MATHEXPR_NAMESPACE_BEGIN

enum Platform : uint32_t
{
    Platform_Windows,
    Platform_Linux,
    Platform_MacOS,
    Platform_Invalid,
};

MATHEXPR_API uint32_t get_current_platform() noexcept;

MATHEXPR_API const char* platform_as_string(uint32_t platform) noexcept;

enum ISA : uint32_t
{
    ISA_x86_64,
    ISA_aarch64,
    ISA_Invalid,
};

MATHEXPR_API uint32_t get_current_isa() noexcept;

MATHEXPR_API const char* isa_as_string(uint32_t isa) noexcept;

using RegisterId = uint32_t;

static constexpr RegisterId INVALID_GP_REGISTER = std::numeric_limits<RegisterId>::max();
static constexpr RegisterId INVALID_FP_REGISTER = std::numeric_limits<RegisterId>::max();

/* x86_64 registers */

/* General Purpose Registers */
enum GpRegisters_x86_64 : RegisterId
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

/* Floating Point Registers (omitting the upper 8-15, not available on windows abi) */
enum FpRegisters_x86_64 : RegisterId
{
    FpRegisters_x86_64_Xmm0,
    FpRegisters_x86_64_Xmm1,
    FpRegisters_x86_64_Xmm2,
    FpRegisters_x86_64_Xmm3,
    FpRegisters_x86_64_Xmm4,
    FpRegisters_x86_64_Xmm5,
    FpRegisters_x86_64_Xmm6,
    FpRegisters_x86_64_Xmm7,
    FpRegisters_x86_64_Ymm0,
    FpRegisters_x86_64_Ymm1,
    FpRegisters_x86_64_Ymm2,
    FpRegisters_x86_64_Ymm3,
    FpRegisters_x86_64_Ymm4,
    FpRegisters_x86_64_Ymm5,
    FpRegisters_x86_64_Ymm6,
    FpRegisters_x86_64_Ymm7,
};

/* aarch64 registers */

enum GpRegisters_aarch64 : RegisterId
{

};

enum FpRegisters_aarch64 : RegisterId
{

};

/* registers as string, convenient for pretty printing */
MATHEXPR_API const char* gp_register_as_string(RegisterId reg, uint32_t isa) noexcept;
MATHEXPR_API const char* fp_register_as_string(RegisterId reg, uint32_t isa) noexcept;

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_PLATFORM) */
