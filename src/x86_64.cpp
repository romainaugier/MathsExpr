// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/x86_64.hpp"
#include "mathexpr/log.hpp"

MATHEXPR_NAMESPACE_BEGIN

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
                      const MemLocPtr& memloc) noexcept
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
            RegisterId stack_register = GpRegisters_x86_64_RBP;

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

            RegisterId regid = mem->get_base_ptr_register();

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

std::byte memloc_as_m_byte(const MemLocPtr& memloc) noexcept
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
            return RBP;
        }

        case MemLocTypeId_Memory:
        {
            auto mem = memloc_const_cast<Memory>(memloc.get());

            return encode_platform_gp_register(mem->get_base_ptr_register());
        }

        default:
            return BYTE(0);
    }
}

std::byte encode_sib(uint8_t scale, uint8_t index, uint8_t base) noexcept
{
    return BYTE(((scale & 0x3) << 6) | ((index & 0x7) << 3) | (base & 0x7));
}

using ModRmSibOffset = std::tuple<std::byte, std::optional<std::byte>, std::byte>;

ModRmSibOffset memloc_as_modrm_sib_offset(MemLocPtr from,
                                          MemLocPtr to) noexcept
{
    switch(from->type_id())
    {
        case MemLocTypeId_Register:
        {
            switch(to->type_id())
            {
                case MemLocTypeId_Register:
                {
                    const std::byte modrm = x86_64::MOD_DIRECT |
                                            memloc_as_r_byte(to) |
                                            memloc_as_m_byte(from);

                    return std::make_tuple(modrm, std::nullopt, BYTE(0));
                }

                case MemLocTypeId_Stack:
                {
                    auto stack = memloc_cast<Stack>(to.get());

                    const std::byte m_byte = memloc_as_m_byte(to);

                    const std::byte modrm = x86_64::MOD_INDIRECT_DISP8 |
                                            memloc_as_r_byte(from) |
                                            m_byte;

                    /* We need to add the sib byte when using RSP as the base register */
                    if(m_byte == RSP)
                    {
                        /* scale=1, index=none (100), base=RSP (100) = 0x24 */
                        const std::byte sib = encode_sib(0, 4, 4);

                        return std::make_tuple(modrm, sib, BYTE(stack->get_offset() - 8));
                    }

                    return std::make_tuple(modrm, std::nullopt, BYTE(stack->get_signed_offset()));
                }

                case MemLocTypeId_Memory:
                {
                    auto memory = memloc_cast<Memory>(to.get());

                    const std::byte mod = memory->get_offset() > 0 ? x86_64::MOD_INDIRECT_DISP8 :
                                                                     x86_64::MOD_INDIRECT;
                    const std::byte modrm = mod |
                                            memloc_as_r_byte(from) |
                                            memloc_as_m_byte(to);

                    return std::make_tuple(modrm, std::nullopt, BYTE(memory->get_offset()));
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

                    const std::byte m_byte = memloc_as_m_byte(from);

                    const std::byte modrm = x86_64::MOD_INDIRECT_DISP8 |
                                            memloc_as_r_byte(to) |
                                            m_byte;

                    /* We need to add the sib byte when using RSP as the base register */
                    if(m_byte == RSP)
                    {
                        /* scale=1, index=none (100), base=RSP (100) = 0x24 */
                        const std::byte sib = encode_sib(0, 4, 4);

                        return std::make_tuple(modrm, sib, BYTE(stack->get_offset() - 8));
                    }

                    return std::make_tuple(modrm, std::nullopt, BYTE(stack->get_signed_offset()));
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
                                            memloc_as_m_byte(from);

                    return std::make_tuple(modrm, std::nullopt, BYTE(memory->get_offset()));
                }
            }

            break;
        }
    }

    return std::make_tuple(BYTE(0), std::nullopt, BYTE(0));
}

bool modrm_has_displace(const std::byte modrm_byte) noexcept
{
    const std::byte mod = modrm_byte & BYTE(0xC0);

    return mod == x86_64::MOD_INDIRECT_DISP8 || mod == x86_64::MOD_INDIRECT_DISP32;
}

/* Memory instructions */

void InstrMov::as_string(std::string& out) const noexcept
{
    std::format_to(std::back_inserter(out), "movsd ");
    memloc_as_string(out, this->_mem_loc_to);
    std::format_to(std::back_inserter(out), ", ");
    memloc_as_string(out, this->_mem_loc_from);
}

void InstrMov::as_bytecode(ByteCode& out) const noexcept
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

    auto [mod_rm_byte, sib, offset] = memloc_as_modrm_sib_offset(this->_mem_loc_from,
                                                                 this->_mem_loc_to);

    out.push_back(mod_rm_byte);

    if(sib.has_value())
    {
        out.push_back(sib.value());
    }

    if(modrm_has_displace(mod_rm_byte))
    {
        out.push_back(offset);
    }
}

void InstrPrologue::as_string(std::string& out) const noexcept
{
    std::format_to(std::back_inserter(out), "push rbp\n");
    std::format_to(std::back_inserter(out), "mov rbp, rsp\n");
    std::format_to(std::back_inserter(out), "sub rsp, {}", this->_stack_size);
}

void InstrPrologue::as_bytecode(ByteCode& out) const noexcept
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

    /* push rbp adds 8 bytes to rsp and misaligns the stack (we need it to be 16 bytes aligned) */
    const uint32_t stack_size = static_cast<uint32_t>(this->_stack_size) + 8;

    if(this->_stack_size > 127)
    {
        out.push_back(BYTE(stack_size & 0xFF));
        out.push_back(BYTE((stack_size >> 8) & 0xFF));
        out.push_back(BYTE((stack_size >> 16) & 0xFF));
        out.push_back(BYTE((stack_size >> 24) & 0xFF));
    }
    else
    {
        out.push_back(BYTE(stack_size));
    }
}

void InstrEpilogue::as_string(std::string& out) const noexcept
{
    std::format_to(std::back_inserter(out), "leave");
}

void InstrEpilogue::as_bytecode(ByteCode& out) const noexcept
{
    out.push_back(BYTE(0xC9));
}

/* Unary ops instructions */

void InstrNeg::as_string(std::string& out) const noexcept
{

}

void InstrNeg::as_bytecode(ByteCode& out) const noexcept
{

}

/* Binary ops instructions */

void InstrAdd::as_string(std::string& out) const noexcept
{
    std::format_to(std::back_inserter(out), "addsd ");
    memloc_as_string(out, this->_left);
    std::format_to(std::back_inserter(out), ", ");
    memloc_as_string(out, this->_right);
}

void InstrAdd::as_bytecode(ByteCode& out) const noexcept
{
    out.push_back(BYTE(0xF2)); /* Prefix */
    out.push_back(BYTE(0x0F));
    out.push_back(BYTE(0x58));

    auto [mod_reg_rm_byte, sib, offset] = memloc_as_modrm_sib_offset(this->_right,
                                                                     this->_left);

    out.push_back(mod_reg_rm_byte);

    if(sib.has_value())
    {
        out.push_back(sib.value());
    }

    if(offset > BYTE(0) || modrm_has_displace(mod_reg_rm_byte))
    {
        out.push_back(offset);
    }
}

void InstrSub::as_string(std::string& out) const noexcept
{
    std::format_to(std::back_inserter(out), "subsd ");
    memloc_as_string(out, this->_left);
    std::format_to(std::back_inserter(out), ", ");
    memloc_as_string(out, this->_right);
}

void InstrSub::as_bytecode(ByteCode& out) const noexcept
{
    out.push_back(BYTE(0xF2)); /* Prefix */
    out.push_back(BYTE(0x0F));
    out.push_back(BYTE(0x5C));

    auto [mod_reg_rm_byte, sib, offset] = memloc_as_modrm_sib_offset(this->_right,
                                                                     this->_left);

    out.push_back(mod_reg_rm_byte);

    if(sib.has_value())
    {
        out.push_back(sib.value());
    }

    if(offset > BYTE(0) || modrm_has_displace(mod_reg_rm_byte))
    {
        out.push_back(offset);
    }
}

void InstrMul::as_string(std::string& out) const noexcept
{
    std::format_to(std::back_inserter(out), "mulsd ");
    memloc_as_string(out, this->_left);
    std::format_to(std::back_inserter(out), ", ");
    memloc_as_string(out, this->_right);
}

void InstrMul::as_bytecode(ByteCode& out) const noexcept
{
    out.push_back(BYTE(0xF2)); /* Prefix */
    out.push_back(BYTE(0x0F));
    out.push_back(BYTE(0x59));

    auto [mod_reg_rm_byte, sib, offset] = memloc_as_modrm_sib_offset(this->_right,
                                                                     this->_left);

    out.push_back(mod_reg_rm_byte);

    if(sib.has_value())
    {
        out.push_back(sib.value());
    }

    if(offset > BYTE(0) || modrm_has_displace(mod_reg_rm_byte))
    {
        out.push_back(offset);
    }
}

void InstrDiv::as_string(std::string& out) const noexcept
{
    std::format_to(std::back_inserter(out), "divsd ");
    memloc_as_string(out, this->_left);
    std::format_to(std::back_inserter(out), ", ");
    memloc_as_string(out, this->_right);
}

void InstrDiv::as_bytecode(ByteCode& out) const noexcept
{
    out.push_back(BYTE(0xF2)); /* Prefix */
    out.push_back(BYTE(0x0F));
    out.push_back(BYTE(0x5E));

    auto [mod_reg_rm_byte, sib, offset] = memloc_as_modrm_sib_offset(this->_right,
                                                                     this->_left);

    out.push_back(mod_reg_rm_byte);

    if(sib.has_value())
    {
        out.push_back(sib.value());
    }

    if(offset > BYTE(0) || modrm_has_displace(mod_reg_rm_byte))
    {
        out.push_back(offset);
    }
}

/* Func ops instructions */

void InstrCall::as_string(std::string& out) const noexcept
{
    std::format_to(std::back_inserter(out), "call {}", this->_call_name);
}

void InstrCall::as_bytecode(ByteCode& out) const noexcept
{
    /* MEMO: On Windows, we need to allocate 32 bytes of shadow space on the stack */
    /* mov rax, imm64 */
    out.push_back(BYTE(0x48));
    out.push_back(BYTE(0xB8));

    for(uint8_t i = 0; i < 8; i++)
        out.push_back(BYTE(0x00));

    /* call rax */
    out.push_back(BYTE(0xFF));
    out.push_back(BYTE(0xD0));
}

RelocInfo InstrCall::get_link_info(std::size_t bytecode_start) const noexcept
{
    RelocInfo info;
    info.symbol_name = this->_call_name;
    info.bytecode_offset = bytecode_start + 2;
    info.reloc_type = RelocType_Abs64;

    return info;
}

/* Terminator instructions */

void InstrRet::as_string(std::string& out) const noexcept
{
    std::format_to(std::back_inserter(out), "ret");
}

void InstrRet::as_bytecode(ByteCode& out) const noexcept
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

MATHEXPR_NAMESPACE_END
