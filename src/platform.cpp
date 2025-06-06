// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/platform.hpp"

MATHSEXPR_NAMESPACE_BEGIN

const char* platform_as_string(uint32_t platform) noexcept
{
    switch(platform)
    {
        case Platform_Linux:
            return "Linux";
        case Platform_Windows:
            return "Windows";
        default:
            return "Unknown platform";
    }
}

const char* isa_as_string(uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
            return "x86_64";
        default:
            return "Unknown ISA";
    }
}

const char* gp_register_x86_64_as_string(uint32_t reg) noexcept
{
    switch(reg)
    {
        case GpRegisters_x86_64_RAX: 
            return "rax";
        case GpRegisters_x86_64_RBX: 
            return "rbx";
        case GpRegisters_x86_64_RCX: 
            return "rcx";
        case GpRegisters_x86_64_RDX: 
            return "rdx";
        case GpRegisters_x86_64_RSI: 
            return "rsi";
        case GpRegisters_x86_64_RDI: 
            return "rdi";
        case GpRegisters_x86_64_RBP: 
            return "rbp";
        case GpRegisters_x86_64_RSP: 
            return "rsp";
        case GpRegisters_x86_64_R8: 
            return "r8";
        case GpRegisters_x86_64_R9: 
            return "r9";
        case GpRegisters_x86_64_R10: 
            return "r10";
        case GpRegisters_x86_64_R11: 
            return "r11";
        case GpRegisters_x86_64_R12: 
            return "r12";
        case GpRegisters_x86_64_R13: 
            return "r13";
        case GpRegisters_x86_64_R14: 
            return "r14";
        case GpRegisters_x86_64_R15: 
            return "r15";
        default:
            return "???";
    }
}

uint32_t get_base_ptr_variable_register(uint32_t platform, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                {
                    return GpRegisters_x86_64_RDI;
                }
                case Platform_Windows:
                {
                    return GpRegisters_x86_64_RCX;
                }

                default:
                    return INVALID_GP_REGISTER;
            }

            break;
        }

        default:
            return INVALID_GP_REGISTER;
    }
}

uint32_t get_base_ptr_literal_register(uint32_t platform, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                {
                    return GpRegisters_x86_64_RSI;
                }
                case Platform_Windows:
                {
                    return GpRegisters_x86_64_RDX;
                }

                default:
                    return INVALID_GP_REGISTER;
            }

            break;
        }

        default:
            return INVALID_GP_REGISTER;
    }
}

MATHSEXPR_NAMESPACE_END