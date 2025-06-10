// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/platform.hpp"

MATHSEXPR_NAMESPACE_BEGIN

uint32_t get_current_platform() noexcept
{
#if defined(MATHSEXPR_WIN)
    return Platform_Windows;
#elif defined(MATHSEXPR_LINUX)
    return Platform_Linux;
#elif defined(MATHSEXPR_MACOS)
    return Platform_MacOS;
#endif /* defined(MATHSEXPR_WIN) */
    return Platform_Invalid;
}

const char* platform_as_string(uint32_t platform) noexcept
{
    switch(platform)
    {
        case Platform_Linux:
            return "Linux";
        case Platform_Windows:
            return "Windows";
        case Platform_MacOS:
            return "MacOS";
        default:
            return "Unknown platform";
    }
}

uint32_t get_current_isa() noexcept
{
#if defined(MATHSEXPR_X64)
    return ISA_x86_64;
#elif defined(MATHSEXPR_AARCH64)
    return ISA_aarch64;
#endif /* defined(MATHSEXPR_X64) */
    return ISA_Invalid;
}

const char* isa_as_string(uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
            return "x86_64";
        case ISA_aarch64:
            return "aarch64";
        default:
            return "Unknown ISA";
    }
}

/* 
    Information about registers

    https://www.thejat.in/learn/system-v-amd64-calling-convention
    https://learn.microsoft.com/en-us/cpp/build/x64-software-conventions?view=msvc-170#x64-register-usage
*/

const char* gp_register_as_string(uint32_t reg, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
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

            break;
        }

        case ISA_aarch64:
        {
            switch(reg)
            {
                default:
                    return "???";
            }

            break;
        }

        default:
            return "???";
    }
}

const char* fp_register_as_string(uint32_t reg, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(reg)
            {
                case FpRegisters_x86_64_Xmm0: 
                    return "xmm0";
                case FpRegisters_x86_64_Xmm1: 
                    return "xmm1";
                case FpRegisters_x86_64_Xmm2: 
                    return "xmm2";
                case FpRegisters_x86_64_Xmm3: 
                    return "xmm3";
                case FpRegisters_x86_64_Xmm4: 
                    return "xmm4";
                case FpRegisters_x86_64_Xmm5: 
                    return "xmm5";
                case FpRegisters_x86_64_Xmm6: 
                    return "xmm6";
                case FpRegisters_x86_64_Xmm7: 
                    return "xmm7";
                case FpRegisters_x86_64_Ymm0: 
                    return "ymm0";
                case FpRegisters_x86_64_Ymm1: 
                    return "ymm1";
                case FpRegisters_x86_64_Ymm2: 
                    return "ymm2";
                case FpRegisters_x86_64_Ymm3: 
                    return "ymm3";
                case FpRegisters_x86_64_Ymm4: 
                    return "ymm4";
                case FpRegisters_x86_64_Ymm5: 
                    return "ymm5";
                case FpRegisters_x86_64_Ymm6: 
                    return "ymm6";
                case FpRegisters_x86_64_Ymm7: 
                    return "ymm7";
                default:
                    return "???";
            }
        }

        case ISA_aarch64:
        {
            switch(reg)
            {
                default:
                    return "???";
            }
        }

        default:
            return "???";
    }
}

RegisterId get_base_ptr_variable_register(uint32_t platform, uint32_t isa) noexcept
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

        /* TODO: implement aarch64 support */
        case ISA_aarch64:
        {
            return INVALID_GP_REGISTER;
        }

        default:
            return INVALID_GP_REGISTER;
    }
}

RegisterId get_base_ptr_literal_register(uint32_t platform, uint32_t isa) noexcept
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

        /* TODO: implement aarch64 support */
        case ISA_aarch64:
        {
            return INVALID_GP_REGISTER;
        }

        default:
            return INVALID_GP_REGISTER;
    }
}

uint64_t get_max_available_gp_registers(uint32_t platform, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                    return 4;

                case Platform_Linux:
                    return 6;

                default:
                    return 0;
            }
        }

        /* TODO: implement aarch64 support */
        case ISA_aarch64:
            return 0;

        default:
            return 0;
    }
}

uint64_t get_max_available_fp_registers(uint32_t platform, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                    return 6;
                case Platform_Linux:
                    return 8;
                default:
                    return 0;
            }
        }

        /* TODO: implement aarch64 support */
        case ISA_aarch64:
            return 0;

        default:
            return 0;
    }
}

RegisterId get_call_return_value_gp_register(uint32_t platform, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                    return GpRegisters_x86_64_RAX;
                case Platform_Linux:
                    return GpRegisters_x86_64_RAX;
                default:
                    return INVALID_GP_REGISTER;
            }
        }

        /* TODO: implement aarch64 support */
        case ISA_aarch64:
            return INVALID_GP_REGISTER;

        default:
            return INVALID_GP_REGISTER;
    }
}

RegisterId get_call_return_value_fp_register(uint32_t platform, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                    return FpRegisters_x86_64_Xmm0;
                case Platform_Linux:
                    return FpRegisters_x86_64_Xmm0;
                default:
                    return INVALID_FP_REGISTER;
            }
        }

        /* TODO: implement aarch64 support */
        case ISA_aarch64:
            return INVALID_FP_REGISTER;

        default:
            return INVALID_FP_REGISTER;
    }
}

/* Returns the number of registers we can use to store arguments for a function call */
uint64_t get_call_max_args_gp_registers(uint32_t platform, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                    return 4;
                case Platform_Linux:
                    return 6;
                default:
                    return 0;
            }
        }
        
        /* TODO: implement aarch64 support */
        case ISA_aarch64:
        {
            switch(platform)
            {
                default:
                    return 0;
            }
        }

        default:
            return 0;
    }
}

uint64_t get_call_max_args_fp_registers(uint32_t platform, uint32_t isa) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                    return 4;
                case Platform_Linux:
                    return 6;
                default:
                    return 0;
            }
        }
        
        /* TODO: implement aarch64 support */
        case ISA_aarch64:
        {
            switch(platform)
            {
                default:
                    return 0;
            }
        }

        default:
            return 0;
    }
}

/* 
    Returns the register order for arguments placement before a function call
*/
const std::vector<RegisterId>& get_call_args_gp_registers(uint32_t platform, 
                                                          uint32_t isa) noexcept
{
    static std::vector<RegisterId> invalid_regs({});

    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                {
                    static std::vector<RegisterId> regs({ GpRegisters_x86_64_RCX,
                                                          GpRegisters_x86_64_RDX,
                                                          GpRegisters_x86_64_R8,
                                                          GpRegisters_x86_64_R9 });
                    return regs;
                }

                case Platform_Linux:
                {
                    static std::vector<RegisterId> regs({ GpRegisters_x86_64_RDI,
                                                          GpRegisters_x86_64_RSI,
                                                          GpRegisters_x86_64_RDX,
                                                          GpRegisters_x86_64_RCX,
                                                          GpRegisters_x86_64_R8,
                                                          GpRegisters_x86_64_R9 });
                    return regs;
                }

                default:
                    return invalid_regs;
            }
        }

        /* TODO: implement aarch64 support */
        case ISA_aarch64:
        {
            switch(platform)
            {
                default:
                    return invalid_regs;
            }
        }

        default:
            return invalid_regs;
    }
}

const std::vector<RegisterId>& get_call_args_fp_registers(uint32_t platform, 
                                                          uint32_t isa) noexcept
{
    static std::vector<RegisterId> invalid_regs({});

    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                {
                    /* Needs __vectorcall to allow 6 fp registers, otherwise it's 4 */
                    static std::vector<RegisterId> regs({ FpRegisters_x86_64_Xmm0,
                                                          FpRegisters_x86_64_Xmm1,
                                                          FpRegisters_x86_64_Xmm2,
                                                          FpRegisters_x86_64_Xmm3,
                                                          FpRegisters_x86_64_Xmm4,
                                                          FpRegisters_x86_64_Xmm5 });
                    return regs;
                }

                case Platform_Linux:
                {
                    static std::vector<RegisterId> regs({ FpRegisters_x86_64_Xmm0,
                                                          FpRegisters_x86_64_Xmm1,
                                                          FpRegisters_x86_64_Xmm2,
                                                          FpRegisters_x86_64_Xmm3,
                                                          FpRegisters_x86_64_Xmm4,
                                                          FpRegisters_x86_64_Xmm5,
                                                          FpRegisters_x86_64_Xmm6,
                                                          FpRegisters_x86_64_Xmm7 });
                    return regs;
                }

                default:
                    return invalid_regs;
            }
        }

        /* TODO: implement aarch64 support */
        case ISA_aarch64:
        {
            switch(platform)
            {
                default:
                    return invalid_regs;
            }
        }

        default:
            return invalid_regs;
    }
}

MATHSEXPR_NAMESPACE_END