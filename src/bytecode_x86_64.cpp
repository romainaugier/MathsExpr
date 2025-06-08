// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/bytecode_x86_64.hpp"

MATHSEXPR_NAMESPACE_BEGIN

X86_64_NAMESPACE_BEGIN

std::byte encode_platform_gp_register(uint32_t platform_register) noexcept
{
    switch(platform_register)
    {
        case GpRegisters_x86_64_RAX: 
            return RAX;
        case GpRegisters_x86_64_RBX: 
            return RBX;
        case GpRegisters_x86_64_RCX: 
            return RCX;
        case GpRegisters_x86_64_RDX: 
            return RDX;
        case GpRegisters_x86_64_RSI: 
            return RSI;
        case GpRegisters_x86_64_RDI: 
            return RDI;
        case GpRegisters_x86_64_RBP: 
            return RBP;
        case GpRegisters_x86_64_RSP: 
            return RSP;
        case GpRegisters_x86_64_R8: 
            return R8;
        case GpRegisters_x86_64_R9: 
            return R9;
        case GpRegisters_x86_64_R10: 
            return R10;
        case GpRegisters_x86_64_R11: 
            return R11;
        case GpRegisters_x86_64_R12: 
            return R12;
        case GpRegisters_x86_64_R13: 
            return R13;
        case GpRegisters_x86_64_R14: 
            return R14;
        case GpRegisters_x86_64_R15: 
            return R15;
    }

    return BYTE(0);
}

std::byte encode_platform_fp_register(uint32_t platform_register) noexcept
{
    switch(platform_register)
    {
        case FpRegisters_x86_64_Xmm0:
            return XMM0;
        case FpRegisters_x86_64_Xmm1:
            return XMM1;
        case FpRegisters_x86_64_Xmm2:
            return XMM2;
        case FpRegisters_x86_64_Xmm3:
            return XMM3;
        case FpRegisters_x86_64_Xmm4:
            return XMM4;
        case FpRegisters_x86_64_Xmm5:
            return XMM5;
        case FpRegisters_x86_64_Xmm6:
            return XMM6;
        case FpRegisters_x86_64_Xmm7:
            return XMM7;
    }

    return BYTE(0);
}

X86_64_NAMESPACE_END

MATHSEXPR_NAMESPACE_END