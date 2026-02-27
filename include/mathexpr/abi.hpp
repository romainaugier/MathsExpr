// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_ABI)
#define __MATHEXPRABI_

#include "mathexpr/platform.hpp"
#include "mathexpr/bytecode.hpp"

/* 
    ABIs

    https://www.thejat.in/learn/system-v-amd64-calling-convention
    https://learn.microsoft.com/en-us/cpp/build/x64-software-conventions?view=msvc-170#x64-register-usage
    https://developer.apple.com/documentation/xcode/writing-arm64-code-for-apple-platforms
*/

MATHEXPR_NAMESPACE_BEGIN

enum PlatformABIID : uint32_t
{
    PlatformABIID_WindowsX64,
    PlatformABIID_LinuxX64,
};

class MATHEXPR_API PlatformABI 
{
public:
    virtual ~PlatformABI() = default;

    virtual std::string_view get_as_string() const noexcept = 0;

    virtual uint32_t get_id() const noexcept = 0;

    virtual uint32_t get_target_isa() const noexcept = 0;

    /* base ptr for the variables values is passed as the first parameter */
    virtual RegisterId get_variable_base_ptr() const noexcept = 0;

    /* base ptr for the variables values is passed as the second parameter */
    virtual RegisterId get_literal_base_ptr() const noexcept = 0;

    /* 
        Returns the maximum number of registers that can be used simultaneously, used by 
        the register allocator to know how many registers we can use
    */
    virtual uint64_t get_max_available_gp_registers() const noexcept = 0;
    virtual uint64_t get_max_available_fp_registers() const noexcept = 0;

    /* Returns the register id of the register used to store the return value of a function call */
    virtual RegisterId get_call_return_value_gp_register() const noexcept = 0;
    virtual RegisterId get_call_return_value_fp_register() const noexcept = 0;

    /* Returns the number of registers we can use to store arguments for a function call */
    virtual uint64_t get_call_max_args_gp_registers() const noexcept = 0;
    virtual uint64_t get_call_max_args_fp_registers() const noexcept = 0;

    /* Returns the register order for arguments placement before a function call */
    virtual const std::vector<RegisterId>& get_call_args_gp_registers() const noexcept = 0;
    virtual const std::vector<RegisterId>& get_call_args_fp_registers() const noexcept = 0;
};

using PlatformABIPtr = std::shared_ptr<PlatformABI>;

PlatformABIPtr get_current_platform_abi(uint32_t isa, uint32_t platform) noexcept;

class WindowsX64ABI : public PlatformABI
{
public:
    virtual ~WindowsX64ABI() = default;

    virtual std::string_view get_as_string() const noexcept override { return "Windows x64"; }

    virtual uint32_t get_id() const noexcept override { return PlatformABIID_WindowsX64; };

    virtual uint32_t get_target_isa() const noexcept override { return ISA_x86_64; }

    /* RCX */
    virtual RegisterId get_variable_base_ptr() const noexcept override;

    /* RDX */
    virtual RegisterId get_literal_base_ptr() const noexcept override;

    /* 4 */
    virtual uint64_t get_max_available_gp_registers() const noexcept override;

    /* Xmm0-Xmm5 */
    virtual uint64_t get_max_available_fp_registers() const noexcept override;

    /* RAX */
    virtual RegisterId get_call_return_value_gp_register() const noexcept override;

    /* Xmm0 */
    virtual RegisterId get_call_return_value_fp_register() const noexcept override;

    /* RCX, RDX, R8, R9 */
    virtual uint64_t get_call_max_args_gp_registers() const noexcept override;

    /* Xmm0-Xmm3 if __fastcall, Xmm0-Xmm5 if __vectorcall */
    virtual uint64_t get_call_max_args_fp_registers() const noexcept override;

    /* RCX, RDX, R8, R9 */
    virtual const std::vector<RegisterId>& get_call_args_gp_registers() const noexcept override;

    /* Xmm0-Xmm5 */
    virtual const std::vector<RegisterId>& get_call_args_fp_registers() const noexcept override;
};

/* Or SysV ABI */
class LinuxX64ABI : public PlatformABI
{
public:
    virtual ~LinuxX64ABI() = default;

    virtual std::string_view get_as_string() const noexcept override { return "Linux x64"; }

    virtual uint32_t get_id() const noexcept override { return PlatformABIID_LinuxX64; };

    virtual uint32_t get_target_isa() const noexcept override { return ISA_x86_64; }

    /* RDI */
    virtual RegisterId get_variable_base_ptr() const noexcept override;

    /* RSI */
    virtual RegisterId get_literal_base_ptr() const noexcept override;

    /* 6 */
    virtual uint64_t get_max_available_gp_registers() const noexcept override;

    /* 8 */
    virtual uint64_t get_max_available_fp_registers() const noexcept override;

    /* RAX */
    virtual RegisterId get_call_return_value_gp_register() const noexcept override;

    /* Xmm0 */
    virtual RegisterId get_call_return_value_fp_register() const noexcept override;

    /* 6 */
    virtual uint64_t get_call_max_args_gp_registers() const noexcept override;

    /* 8 */
    virtual uint64_t get_call_max_args_fp_registers() const noexcept override;

    /* RSI, RDI, RCX, RDX, R8, R9 */
    virtual const std::vector<RegisterId>& get_call_args_gp_registers() const noexcept override;

    /* Xmm0-Xmm7 */
    virtual const std::vector<RegisterId>& get_call_args_fp_registers() const noexcept override;
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_ABI) */
