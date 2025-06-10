// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/x86_64.hpp"
#include "mathsexpr/log.hpp"

MATHSEXPR_NAMESPACE_BEGIN

REGISTER_TARGET(ISA_x86_64, X86_64_CodeGenerator);

X86_64_NAMESPACE_BEGIN

/*
    For reference:
    https://www.felixcloutier.com/x86/
    https://asmjit.com/parser.html
*/

/* Utilities to remap memory locations assigned by the register allocator */

/* 
    Translates a platform general purpose register (from enums in platform.hpp) 
    to the encoding used in ModR/M 
*/
std::byte encode_platform_gp_register(RegisterId platform_register) noexcept
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

/* 
    Translates a platform floating point register (from enums in platform.hpp) 
    to the encoding used in ModR/M 
*/
std::byte encode_platform_fp_register(RegisterId platform_register) noexcept
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

void memloc_as_string(std::string& out, 
                      const MemLocPtr& memloc,
                      uint32_t platform) noexcept
{
    switch(memloc->type_id())
    {
        case MemLocTypeId_Invalid:
        {
            std::format_to(std::back_inserter(out), "inv");
            break;
        }

        case MemLocTypeId_Register:
        {
            auto reg = memloc_const_cast<Register>(memloc.get());

            std::format_to(std::back_inserter(out),
                           "{}",
                           fp_register_as_string(reg->get_id(), ISA_x86_64));

            break;
        }

        case MemLocTypeId_Stack:
        {
            RegisterId stack_register = platform == Platform_Linux ? GpRegisters_x86_64_RBP : 
                                                                     GpRegisters_x86_64_RSP;

            auto stack = memloc_const_cast<Stack>(memloc.get());

            std::format_to(std::back_inserter(out), 
                           "[{} - {}]",
                           gp_register_as_string(stack_register, ISA_x86_64),
                           stack->get_offset());

            break;
        }

        case MemLocTypeId_Memory:
        {
            auto mem = memloc_const_cast<Memory>(memloc.get());

            RegisterId regid = mem->get_mem_loc_register() == MemLocRegister_Variables ? 
                               get_base_ptr_variable_register(platform, ISA_x86_64) : 
                               get_base_ptr_literal_register(platform, ISA_x86_64);

            std::format_to(std::back_inserter(out), 
                           "[{} + {}]",
                           gp_register_as_string(regid, ISA_x86_64),
                           mem->get_offset());

            break;
        }
    }
}

/* Only useful for fp register since only fp register are used to move to and from */
std::byte memloc_as_r_byte(const MemLocPtr& memloc) noexcept
{
    switch(memloc->type_id())
    {
        case MemLocTypeId_Register:
        {
            auto reg = memloc_const_cast<Register>(memloc.get());

            return encode_platform_fp_register(reg->get_id()) << 3;
        }

        default:
            return BYTE(0);
    }
}

std::byte memloc_as_m_byte(const MemLocPtr& memloc,
                           uint32_t platform) noexcept
{
    switch(memloc->type_id())
    {
        case MemLocTypeId_Register:
        {
            auto reg = memloc_const_cast<Register>(memloc.get());

            return encode_platform_fp_register(reg->get_id());
        }

        case MemLocTypeId_Stack:
        {
            std::byte stack_register = platform == Platform_Linux ? RBP : 
                                                                    RSP ;

            return stack_register;
        }

        case MemLocTypeId_Memory:
        {
            auto mem = memloc_const_cast<Memory>(memloc.get());

            if(mem->get_mem_loc_register() == MemLocRegister_Variables)
            {
                return encode_platform_gp_register(get_base_ptr_variable_register(platform, 
                                                                                  ISA_x86_64));
            }
            else if(mem->get_mem_loc_register() == MemLocRegister_Literals)
            {
                return encode_platform_gp_register(get_base_ptr_literal_register(platform,
                                                                                 ISA_x86_64));
            }
            else
            {
                return BYTE(0);
            }
        }

        default:
            return BYTE(0);
    }
}

std::pair<std::byte, std::byte> memloc_as_modrm_and_offset(MemLocPtr to, 
                                                           MemLocPtr from,
                                                           uint32_t platform) noexcept
{
    switch(from->type_id())
    {
        case MemLocTypeId_Register:
        {
            auto from_register = memloc_const_cast<Register>(from.get());
            const std::byte from_reg_byte = encode_platform_gp_register(from_register->get_id());

            switch(to->type_id())
            {
                case MemLocTypeId_Register:
                {
                    auto to_register = memloc_const_cast<Register>(to.get());

                    const std::byte to_reg = encode_platform_gp_register(to_register->get_id()) << 3;

                    const std::byte modrm = x86_64::MOD_DIRECT | 
                                            memloc_as_r_byte(to) |
                                            memloc_as_m_byte(from, platform);

                    return std::make_pair(modrm, BYTE(0));
                }

                case MemLocTypeId_Stack:
                {
                    auto stack = memloc_cast<Stack>(to.get());

                    const std::byte modrm = x86_64::MOD_INDIRECT_DISP8 | 
                                            memloc_as_r_byte(from) |
                                            memloc_as_m_byte(to, platform);

                    return std::make_pair(modrm, BYTE(stack->get_signed_offset()));
                }

                case MemLocTypeId_Memory:
                {
                    auto memory = memloc_cast<Memory>(to.get());

                    const std::byte mod = memory->get_offset() > 0 ? x86_64::MOD_INDIRECT_DISP8 : 
                                                                     x86_64::MOD_INDIRECT;
                    const std::byte modrm = mod |
                                            memloc_as_r_byte(from) |
                                            memloc_as_m_byte(to, platform);

                    return std::make_pair(modrm, BYTE(memory->get_offset()));
                }
            }

            break;
        }

        case MemLocTypeId_Stack:
        {
            switch(to->type_id())
            {
                case MemLocTypeId_Register:
                {
                    auto stack = memloc_cast<Stack>(from.get());

                    const std::byte modrm = x86_64::MOD_INDIRECT_DISP8 | 
                                            memloc_as_r_byte(to) |
                                            memloc_as_m_byte(from, platform);

                    return std::make_pair(modrm, BYTE(stack->get_signed_offset()));
                }
            }

            break;
        }

        case MemLocTypeId_Memory:
        {
            switch(to->type_id())
            {
                case MemLocTypeId_Register:
                {
                    auto memory = memloc_cast<Memory>(from.get());

                    const std::byte mod = memory->get_offset() > 0 ? x86_64::MOD_INDIRECT_DISP8 : 
                                                                     x86_64::MOD_INDIRECT;
                    const std::byte modrm = mod | 
                                            memloc_as_r_byte(to) |
                                            memloc_as_m_byte(from, platform);

                    return std::make_pair(modrm, BYTE(memory->get_offset()));
                }
            }

            break;
        }
    }

    return std::make_pair(BYTE(0), BYTE(0));
}

bool modrm_has_displace(const std::byte modrm_byte) noexcept
{
    return (modrm_byte & (x86_64::MOD_INDIRECT_DISP8 | x86_64::MOD_INDIRECT_DISP32)) > BYTE(0);
}

/* Memory instructions */

void InstrMov::as_string(std::string& out, uint32_t platform) const noexcept 
{
    std::format_to(std::back_inserter(out), "movsd ");
    memloc_as_string(out, this->_mem_loc_to, platform);
    std::format_to(std::back_inserter(out), ", ");
    memloc_as_string(out, this->_mem_loc_from, platform);
}

void InstrMov::as_bytecode(ByteCode& out, uint32_t platform) const noexcept 
{
    out.push_back(BYTE(0xF2)); /* Prefix */
    out.push_back(BYTE(0x0F));

    if(this->_mem_loc_to->type_id() == MemLocTypeId_Register)
    {
        out.push_back(BYTE(0x10));
    }
    else
    {
        out.push_back(BYTE(0x11));
    }

    auto [mod_rm_byte, offset] = memloc_as_modrm_and_offset(this->_mem_loc_to,
                                                            this->_mem_loc_from,
                                                            platform);

    out.push_back(mod_rm_byte);

    if(offset > BYTE(0) || modrm_has_displace(mod_rm_byte))
    {
        out.push_back(offset);
    }
}

void InstrPrologue::as_string(std::string& out, uint32_t platform) const noexcept 
{
    switch(platform)
    {
        case Platform_Linux:
            std::format_to(std::back_inserter(out), "push rbp\n");
            std::format_to(std::back_inserter(out), "mov rbp, rsp\n");
            std::format_to(std::back_inserter(out), "sub rsp, {}", this->_stack_size);
            break;

        case Platform_Windows:
            std::format_to(std::back_inserter(out), "sub rsp, {}", this->_stack_size);
            break;
    }
}

void InstrPrologue::as_bytecode(ByteCode& out, uint32_t platform) const noexcept 
{
    switch(platform)
    {
        case Platform_Linux:
        {
            out.push_back(BYTE(0x55)); /* push rbp */

            out.push_back(BYTE(0x48)); /* mov rbp, rsp */
            out.push_back(BYTE(0x89));
            out.push_back(BYTE(0xE5));

            out.push_back(x86_64::REX_BASE | x86_64::REX_W); /* REX.W prefix */

            if(this->_stack_size > 127)
            {
                out.push_back(BYTE(0x81)); /* sub with single-byte immediate (imm32) */
            }
            else
            {
                out.push_back(BYTE(0x83)); /* sub with single-byte immediate (imm8) */
            }

            out.push_back(BYTE(0xEC)); /* rsp */

            if(this->_stack_size > 127)
            {
                const uint32_t stack_size = static_cast<uint32_t>(this->_stack_size);

                out.push_back(BYTE(stack_size & 0xFF));
                out.push_back(BYTE((stack_size >> 8) & 0xFF));
                out.push_back(BYTE((stack_size >> 16) & 0xFF));
                out.push_back(BYTE((stack_size >> 24) & 0xFF));
            }
            else
            {
                out.push_back(BYTE(this->_stack_size));
            }

            break;
        }

        case Platform_Windows:
        {
            out.push_back(x86_64::REX_BASE | x86_64::REX_W); /* REX.W prefix */

            if(this->_stack_size > 127)
            {
                out.push_back(BYTE(0x81)); /* sub with single-byte immediate (imm32) */
            }
            else
            {
                out.push_back(BYTE(0x83)); /* sub with single-byte immediate (imm8) */
            }

            out.push_back(BYTE(0xEC)); /* rsp */

            out.push_back(BYTE(this->_stack_size));

            break;
        }
    }
}

void InstrEpilogue::as_string(std::string& out, uint32_t platform) const noexcept 
{
    switch(platform)
    {
        case Platform_Linux:
            std::format_to(std::back_inserter(out), "leave");
            break;

        case Platform_Windows:
            std::format_to(std::back_inserter(out), "add rsp, {}", this->_stack_size);
            break;
    }
}

void InstrEpilogue::as_bytecode(ByteCode& out, uint32_t platform) const noexcept 
{
    switch(platform)
    {
        case Platform_Linux:
            out.push_back(BYTE(0xC9));
            break;

        case Platform_Windows:
            out.push_back(x86_64::REX_BASE | x86_64::REX_W); /* REX.W prefix */
            out.push_back(BYTE(0x83));
            out.push_back(BYTE(0xC4)); /* rsp */
            out.push_back(BYTE(this->_stack_size));
            break;
    }
}

/* Unary ops instructions */

void InstrNeg::as_string(std::string& out, uint32_t platform) const noexcept
{

}

void InstrNeg::as_bytecode(ByteCode& out, uint32_t platform) const noexcept
{

}

/* Binary ops instructions */

void InstrAdd::as_string(std::string& out, uint32_t platform) const noexcept 
{
    std::format_to(std::back_inserter(out), "addsd ");
    memloc_as_string(out, this->_left, platform);
    std::format_to(std::back_inserter(out), ", ");
    memloc_as_string(out, this->_right, platform);
}

void InstrAdd::as_bytecode(ByteCode& out, uint32_t platform) const noexcept 
{
    out.push_back(BYTE(0xF2)); /* Prefix */
    out.push_back(BYTE(0x0F));
    out.push_back(BYTE(0x58));

    auto [mod_reg_rm_byte, offset] = memloc_as_modrm_and_offset(this->_right,
                                                                this->_left,
                                                                platform);

    out.push_back(mod_reg_rm_byte);

    if(offset > BYTE(0) || modrm_has_displace(mod_reg_rm_byte))
    {
        out.push_back(offset);
    }
}

void InstrSub::as_string(std::string& out, uint32_t platform) const noexcept 
{
    std::format_to(std::back_inserter(out), "subsd ");
    memloc_as_string(out, this->_left, platform);
    std::format_to(std::back_inserter(out), ", ");
    memloc_as_string(out, this->_right, platform);
}

void InstrSub::as_bytecode(ByteCode& out, uint32_t platform) const noexcept 
{
    out.push_back(BYTE(0xF2)); /* Prefix */
    out.push_back(BYTE(0x0F));
    out.push_back(BYTE(0x5C));

    auto [mod_reg_rm_byte, offset] = memloc_as_modrm_and_offset(this->_right,
                                                                this->_left,
                                                                platform);

    out.push_back(mod_reg_rm_byte);

    if(offset > BYTE(0) || modrm_has_displace(mod_reg_rm_byte))
    {
        out.push_back(offset);
    }
}

void InstrMul::as_string(std::string& out, uint32_t platform) const noexcept 
{
    std::format_to(std::back_inserter(out), "mulsd ");
    memloc_as_string(out, this->_left, platform);
    std::format_to(std::back_inserter(out), ", ");
    memloc_as_string(out, this->_right, platform);
}

void InstrMul::as_bytecode(ByteCode& out, uint32_t platform) const noexcept 
{
    out.push_back(BYTE(0xF2)); /* Prefix */
    out.push_back(BYTE(0x0F));
    out.push_back(BYTE(0x59));

    auto [mod_reg_rm_byte, offset] = memloc_as_modrm_and_offset(this->_right,
                                                                this->_left,
                                                                platform);

    out.push_back(mod_reg_rm_byte);

    if(offset > BYTE(0) || modrm_has_displace(mod_reg_rm_byte))
    {
        out.push_back(offset);
    }
}

void InstrDiv::as_string(std::string& out, uint32_t platform) const noexcept 
{
    std::format_to(std::back_inserter(out), "divsd ");
    memloc_as_string(out, this->_left, platform);
    std::format_to(std::back_inserter(out), ", ");
    memloc_as_string(out, this->_right, platform);
}

void InstrDiv::as_bytecode(ByteCode& out, uint32_t platform) const noexcept 
{
    out.push_back(BYTE(0xF2)); /* Prefix */
    out.push_back(BYTE(0x0F));
    out.push_back(BYTE(0x5E));

    auto [mod_reg_rm_byte, offset] = memloc_as_modrm_and_offset(this->_right,
                                                                this->_left,
                                                                platform);

    out.push_back(mod_reg_rm_byte);

    if(offset > BYTE(0) || modrm_has_displace(mod_reg_rm_byte))
    {
        out.push_back(offset);
    }
}

/* Func ops instructions */

void InstrCall::as_string(std::string& out, uint32_t platform) const noexcept
{
    std::format_to(std::back_inserter(out), "call {}", this->_call_name);
}

void InstrCall::as_bytecode(ByteCode& out, uint32_t platform) const noexcept
{
    /* TODO: decide which register to pass the function address to */
    /* MEMO: On Windows, we need to allocate 40 bytes of shadow space on the stack */
}

/* Terminator instructions */

void InstrRet::as_string(std::string& out, uint32_t platform) const noexcept
{
    std::format_to(std::back_inserter(out), "ret");
}

void InstrRet::as_bytecode(ByteCode& out, uint32_t platform) const noexcept
{
    out.push_back(BYTE(0xC3));
}

X86_64_NAMESPACE_END

InstrPtr X86_64_CodeGenerator::create_mov(MemLocPtr& from, MemLocPtr& to) 
{
    return std::make_shared<x86_64::InstrMov>(from, to);
}

InstrPtr X86_64_CodeGenerator::create_prologue(uint64_t stack_size) 
{
    return std::make_shared<x86_64::InstrPrologue>(stack_size);
}

InstrPtr X86_64_CodeGenerator::create_epilogue(uint64_t stack_size) 
{
    return std::make_shared<x86_64::InstrEpilogue>(stack_size);
}

InstrPtr X86_64_CodeGenerator::create_neg(MemLocPtr& operand) 
{
    return std::make_shared<x86_64::InstrNeg>(operand);
}

InstrPtr X86_64_CodeGenerator::create_add(MemLocPtr& left, MemLocPtr& right) 
{
    return std::make_shared<x86_64::InstrAdd>(left, right);
}

InstrPtr X86_64_CodeGenerator::create_sub(MemLocPtr& left, MemLocPtr& right) 
{
    return std::make_shared<x86_64::InstrSub>(left, right);
}

InstrPtr X86_64_CodeGenerator::create_mul(MemLocPtr& left, MemLocPtr& right) 
{
    return std::make_shared<x86_64::InstrMul>(left, right);
}

InstrPtr X86_64_CodeGenerator::create_div(MemLocPtr& left, MemLocPtr& right) 
{
    return std::make_shared<x86_64::InstrDiv>(left, right);
}

InstrPtr X86_64_CodeGenerator::create_call(std::string_view call_name) 
{
    return std::make_shared<x86_64::InstrCall>(call_name);
}

InstrPtr X86_64_CodeGenerator::create_ret() 
{
    return std::make_shared<x86_64::InstrRet>();
}

void X86_64_CodeGenerator::optimize_instr_sequence(std::vector<InstrPtr>& instructions) noexcept
{

}

MATHSEXPR_NAMESPACE_END