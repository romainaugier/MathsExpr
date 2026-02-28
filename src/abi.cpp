// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/abi.hpp"

MATHEXPR_NAMESPACE_BEGIN

PlatformABIPtr get_current_platform_abi(uint32_t isa, uint32_t platform) noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Windows:
                    return std::make_shared<WindowsX64ABI>();

                case Platform_Linux:
                    return std::make_shared<LinuxX64ABI>();
            }
        }
    }

    return nullptr;
}

/* Windows x64 ABI */

RegisterId WindowsX64ABI::get_variable_base_ptr() const noexcept
{
    return GpRegisters_x86_64_RCX;
}

RegisterId WindowsX64ABI::get_literal_base_ptr() const noexcept
{
    return GpRegisters_x86_64_RDX;
}

uint64_t WindowsX64ABI::get_max_available_gp_registers() const noexcept
{
    return 4;
}

uint64_t WindowsX64ABI::get_max_available_fp_registers() const noexcept
{
    return 6;
}

RegisterId WindowsX64ABI::get_call_return_value_gp_register() const noexcept
{
    return GpRegisters_x86_64_RAX;
}

RegisterId WindowsX64ABI::get_call_return_value_fp_register() const noexcept
{
    return FpRegisters_x86_64_Xmm0;
}

uint64_t WindowsX64ABI::get_call_max_args_gp_registers() const noexcept
{
    return 4;
}

uint64_t WindowsX64ABI::get_call_max_args_fp_registers() const noexcept
{
    return 6;
}

const std::vector<RegisterId>& WindowsX64ABI::get_call_args_gp_registers() const noexcept
{
    static const std::vector<RegisterId> regs({ GpRegisters_x86_64_RCX,
                                                GpRegisters_x86_64_RDX,
                                                GpRegisters_x86_64_R8,
                                                GpRegisters_x86_64_R9 });

    return regs;
}

const std::vector<RegisterId>& WindowsX64ABI::get_call_args_fp_registers() const noexcept
{
    static const std::vector<RegisterId> regs({ FpRegisters_x86_64_Xmm0,
                                                FpRegisters_x86_64_Xmm1,
                                                FpRegisters_x86_64_Xmm2,
                                                FpRegisters_x86_64_Xmm3,
                                                FpRegisters_x86_64_Xmm4,
                                                FpRegisters_x86_64_Xmm5 });

    return regs;
}

/* Linux x64 ABI */

RegisterId LinuxX64ABI::get_variable_base_ptr() const noexcept
{
    return GpRegisters_x86_64_RDI;
}

RegisterId LinuxX64ABI::get_literal_base_ptr() const noexcept
{
    return GpRegisters_x86_64_RSI;
}

uint64_t LinuxX64ABI::get_max_available_gp_registers() const noexcept
{
    return 6;
}

uint64_t LinuxX64ABI::get_max_available_fp_registers() const noexcept
{
    return 8;
}

RegisterId LinuxX64ABI::get_call_return_value_gp_register() const noexcept
{
    return GpRegisters_x86_64_RAX;
}

RegisterId LinuxX64ABI::get_call_return_value_fp_register() const noexcept
{
    return FpRegisters_x86_64_Xmm0;
}

uint64_t LinuxX64ABI::get_call_max_args_gp_registers() const noexcept
{
    return 6;
}

uint64_t LinuxX64ABI::get_call_max_args_fp_registers() const noexcept
{
    return 8;
}

const std::vector<RegisterId>& LinuxX64ABI::get_call_args_gp_registers() const noexcept
{
    static const std::vector<RegisterId> regs({ GpRegisters_x86_64_RSI,
                                                GpRegisters_x86_64_RDI,
                                                GpRegisters_x86_64_RCX,
                                                GpRegisters_x86_64_RDX,
                                                GpRegisters_x86_64_R8,
                                                GpRegisters_x86_64_R9 });

    return regs;
}

const std::vector<RegisterId>& LinuxX64ABI::get_call_args_fp_registers() const noexcept
{
    static const std::vector<RegisterId> regs({ FpRegisters_x86_64_Xmm0,
                                                FpRegisters_x86_64_Xmm1,
                                                FpRegisters_x86_64_Xmm2,
                                                FpRegisters_x86_64_Xmm3,
                                                FpRegisters_x86_64_Xmm4,
                                                FpRegisters_x86_64_Xmm5,
                                                FpRegisters_x86_64_Xmm6,
                                                FpRegisters_x86_64_Xmm7 });

    return regs;
}

MATHEXPR_NAMESPACE_END
