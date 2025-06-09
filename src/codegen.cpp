// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/codegen.hpp"
#include "mathsexpr/op.hpp"
#include "mathsexpr/log.hpp"

/*
    For reference:
    https://www.felixcloutier.com/x86/
    https://asmjit.com/parser.html
*/

MATHSEXPR_NAMESPACE_BEGIN

/* Memory instructions */

void InstrMov::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
                    std::format_to(std::back_inserter(out), "movsd ");
                    this->_mem_loc_to->as_string(out, isa, platform);
                    std::format_to(std::back_inserter(out), ", ");
                    this->_mem_loc_from->as_string(out, isa, platform);
                    break;
            }
        }
    }
}

std::pair<std::byte, std::byte> get_x86_64_modrm_and_offset(MemLocPtr from, 
                                                            MemLocPtr to,
                                                            uint32_t isa,
                                                            uint32_t platform) noexcept
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
                                            to->as_reg_byte(isa, platform) |
                                            from->as_rm_byte(isa, platform);

                    return std::make_pair(modrm, BYTE(0));
                }

                case MemLocTypeId_Stack:
                {
                    auto stack = memloc_cast<Stack>(to.get());

                    const std::byte modrm = x86_64::MOD_INDIRECT_DISP8 | 
                                            from->as_reg_byte(isa, platform) |
                                            to->as_rm_byte(isa, platform);

                    return std::make_pair(modrm, BYTE(stack->get_signed_offset()));
                }

                case MemLocTypeId_Memory:
                {
                    auto memory = memloc_cast<Memory>(to.get());

                    const std::byte mod = memory->get_offset() > 0 ? x86_64::MOD_INDIRECT_DISP8 : 
                                                                     x86_64::MOD_INDIRECT;
                    const std::byte modrm = mod |
                                            from->as_reg_byte(isa, platform) |
                                            to->as_rm_byte(isa, platform);

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
                                            to->as_reg_byte(isa, platform) |
                                            from->as_rm_byte(isa, platform);

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
                                            to->as_reg_byte(isa, platform) |
                                            from->as_rm_byte(isa, platform);

                    return std::make_pair(modrm, BYTE(memory->get_offset()));
                }
            }

            break;
        }
    }

}

void InstrMov::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
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

                    auto [mod_reg_rm_byte, offset] = get_x86_64_modrm_and_offset(this->_mem_loc_from,
                                                                                 this->_mem_loc_to,
                                                                                 isa,
                                                                                 platform);

                    out.push_back(mod_reg_rm_byte);

                    if(offset > BYTE(0) || 
                       (mod_reg_rm_byte & (x86_64::MOD_INDIRECT_DISP8 | x86_64::MOD_INDIRECT_DISP32)) > BYTE(0))
                    {
                        out.push_back(offset);
                    }

                    break;
                }
            }

            break;
        }
    }
}

void InstrPrologue::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
                    std::format_to(std::back_inserter(out), "push rbp\n");
                    std::format_to(std::back_inserter(out), "mov rbp, rsp\n");
                    std::format_to(std::back_inserter(out), "sub rsp, {}", this->_stack_size);
                    break;
            }
        }
    }
}

void InstrPrologue::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
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
            }

            break;
        }
    }
}

void InstrEpilogue::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
                    std::format_to(std::back_inserter(out), "leave");
                    break;
            }
        }
    }
}

void InstrEpilogue::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
                {
                    out.push_back(BYTE(0xC9));
                    break;
                }
            }

            break;
        }
    }
}

/* Unary ops instructions */

/* Binary ops instructions */

void InstrAdd::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(platform)
    {
        case Platform_Linux:
        case Platform_Windows:
            std::format_to(std::back_inserter(out), "addsd ");
            this->_left->as_string(out, isa, platform);
            std::format_to(std::back_inserter(out), ", ");
            this->_right->as_string(out, isa, platform);
            break;
    }
}

void InstrAdd::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
                {
                    out.push_back(BYTE(0xF2)); /* Prefix */
                    out.push_back(BYTE(0x0F));
                    out.push_back(BYTE(0x58));

                    auto [mod_reg_rm_byte, offset] = get_x86_64_modrm_and_offset(this->_right,
                                                                                 this->_left,
                                                                                 isa,
                                                                                 platform);

                    out.push_back(mod_reg_rm_byte);

                    if(offset > BYTE(0))
                    {
                        out.push_back(offset);
                    }

                    break;
                }
            }

            break;
        }
    }
}

void InstrSub::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(platform)
    {
        case Platform_Linux:
        case Platform_Windows:
            std::format_to(std::back_inserter(out), "subsd ");
            this->_left->as_string(out, isa, platform);
            std::format_to(std::back_inserter(out), ", ");
            this->_right->as_string(out, isa, platform);
            break;
    }
}

void InstrSub::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
                {
                    out.push_back(BYTE(0xF2)); /* Prefix */
                    out.push_back(BYTE(0x0F));
                    out.push_back(BYTE(0x5C));

                    auto [mod_reg_rm_byte, offset] = get_x86_64_modrm_and_offset(this->_right,
                                                                                 this->_left,
                                                                                 isa,
                                                                                 platform);

                    out.push_back(mod_reg_rm_byte);

                    if(offset > BYTE(0))
                    {
                        out.push_back(offset);
                    }

                    break;
                }
            }

            break;
        }
    }
}

void InstrMul::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(platform)
    {
        case Platform_Linux:
        case Platform_Windows:
            std::format_to(std::back_inserter(out), "mulsd ");
            this->_left->as_string(out, isa, platform);
            std::format_to(std::back_inserter(out), ", ");
            this->_right->as_string(out, isa, platform);
            break;
    }
}

void InstrMul::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
                {
                    out.push_back(BYTE(0xF2)); /* Prefix */
                    out.push_back(BYTE(0x0F));
                    out.push_back(BYTE(0x59));

                    auto [mod_reg_rm_byte, offset] = get_x86_64_modrm_and_offset(this->_right,
                                                                                 this->_left,
                                                                                 isa,
                                                                                 platform);

                    out.push_back(mod_reg_rm_byte);

                    if(offset > BYTE(0))
                    {
                        out.push_back(offset);
                    }

                    break;
                }
            }

            break;
        }
    }
}

void InstrDiv::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(platform)
    {
        case Platform_Linux:
        case Platform_Windows:
            std::format_to(std::back_inserter(out), "divsd ");
            this->_left->as_string(out, isa, platform);
            std::format_to(std::back_inserter(out), ", ");
            this->_right->as_string(out, isa, platform);
            break;
    }
}

void InstrDiv::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
                {
                    out.push_back(BYTE(0xF2)); /* Prefix */
                    out.push_back(BYTE(0x0F));
                    out.push_back(BYTE(0x5E));

                    auto [mod_reg_rm_byte, offset] = get_x86_64_modrm_and_offset(this->_right,
                                                                                 this->_left,
                                                                                 isa,
                                                                                 platform);

                    out.push_back(mod_reg_rm_byte);

                    if(offset > BYTE(0))
                    {
                        out.push_back(offset);
                    }

                    break;
                }
            }

            break;
        }
    }
}

/* Func ops instructions */

void InstrCall::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept
{
    switch(platform)
    {
        case Platform_Linux:
        case Platform_Windows:
            std::format_to(std::back_inserter(out), "call {}", this->_call_name);
            break;
    }
}

void InstrCall::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept
{

}

/* Terminator instructions */

void InstrRet::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
                    std::format_to(std::back_inserter(out), "ret");
                    break;
            }

            break;
        }
    }
}

void InstrRet::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept
{
    switch(isa)
    {
        case ISA_x86_64:
        {
            switch(platform)
            {
                case Platform_Linux:
                case Platform_Windows:
                    out.push_back(BYTE(0xC3));
                    break;
            }

            break;
        }
    }
}

/* Code Generation */

bool CodeGenerator::build(const SSA& ssa,
                          const RegisterAllocator& regalloc,
                          SymbolTable& symtable) noexcept
{
    this->_instructions.clear();

    bool needs_epilogue = false;

    for(auto stmt : ssa.get_statements())
    {
        switch(stmt->type_id())
        {
            case SSAStmtTypeId_Variable:
            {
                auto variable = statement_cast<SSAStmtVariable>(stmt.get());

                MemLocPtr loc = regalloc.get_memloc(stmt);

                if(loc->type_id() == MemLocTypeId_Register)
                {
                    MemLocPtr mem = std::make_shared<Memory>(MemLocRegister_Variables, 
                                                            symtable.get_variable_offset(variable->get_name()));

                    InstrPtr mov = std::make_shared<InstrMov>(mem, loc);

                    this->_instructions.push_back(mov);
                }

                break;
            }
            case SSAStmtTypeId_Literal:
            {
                auto literal = statement_cast<SSAStmtLiteral>(stmt.get());

                MemLocPtr loc = regalloc.get_memloc(stmt);

                if(loc->type_id() == MemLocTypeId_Register)
                {
                    MemLocPtr mem = std::make_shared<Memory>(MemLocRegister_Literals, 
                                                             symtable.get_literal_offset(literal->get_name()));

                    InstrPtr mov = std::make_shared<InstrMov>(mem, loc);

                    this->_instructions.push_back(mov);
                }

                break;
            }
            case SSAStmtTypeId_UnOp:
            {
                break;
            }
            case SSAStmtTypeId_BinOp:
            {
                auto binop = statement_cast<SSAStmtBinOp>(stmt.get());

                MemLocPtr left = regalloc.get_memloc(binop->get_left());

                if(left == nullptr)
                {
                    log_error("Error during codegen. Cannot find location of symbol: {}",
                              binop->get_left()->get_version());

                    return false;
                }

                MemLocPtr right = regalloc.get_memloc(binop->get_right());

                if(right == nullptr)
                {
                    log_error("Error during codegen. Cannot find location of symbol: {}",
                              binop->get_right()->get_version());

                    return false;
                }

                switch(binop->get_op())
                {
                    case BinaryOpType_Add:
                    {
                        InstrPtr instr = std::make_shared<InstrAdd>(left, right);
                        this->_instructions.push_back(instr);
                        break;
                    }
                    case BinaryOpType_Sub:
                    {
                        InstrPtr instr = std::make_shared<InstrSub>(left, right);
                        this->_instructions.push_back(instr);
                        break;
                    }
                    case BinaryOpType_Mul:
                    {
                        InstrPtr instr = std::make_shared<InstrMul>(left, right);
                        this->_instructions.push_back(instr);
                        break;
                    }
                    case BinaryOpType_Div:
                    {
                        InstrPtr instr = std::make_shared<InstrDiv>(left, right);
                        this->_instructions.push_back(instr);
                        break;
                    }
                }

                break;
            }
            case SSAStmtTypeId_FuncOp:
            {
                auto funcop = statement_cast<SSAStmtFunctionOp>(stmt.get());

                if(funcop == nullptr)
                {
                    log_error("Internal error during codegen: expected func op, got: {}", stmt->type_id());
                    return false;
                }

                InstrPtr call = std::make_shared<InstrCall>(funcop->get_name());

                this->_instructions.push_back(call);

                break;
            }
            case SSAStmtTypeId_AllocateStackOp:
            {
                needs_epilogue = true;

                auto allocstackop = statement_cast<SSAStmtAllocateStackOp>(stmt.get());

                if(allocstackop == nullptr)
                {
                    log_error("Internal error during codegen: expected alloc stack op, got: {}",
                              stmt->type_id());
                }

                InstrPtr alloc = std::make_shared<InstrPrologue>(allocstackop->get_stack_size());

                this->_instructions.insert(this->_instructions.begin(), alloc);

                break;
            }
            case SSAStmtTypeId_SpillOp:
            {
                auto spillop = statement_cast<SSAStmtSpillOp>(stmt.get());

                if(spillop == nullptr)
                {
                    log_error("Internal error during codegen: expected spill op, got: {}", stmt->type_id());
                    return false;
                }

                MemLocPtr reg = regalloc.get_memloc(spillop->get_operand());
                MemLocPtr mem = regalloc.get_memloc(stmt);

                InstrPtr mov = std::make_shared<InstrMov>(reg, mem);

                this->_instructions.push_back(mov);

                break;
            }
            case SSAStmtTypeId_LoadOp:
            {
                auto loadop = statement_cast<SSAStmtLoadOp>(stmt.get());

                if(loadop == nullptr)
                {
                    log_error("Internal error during codegen: expected load op, got: {}", stmt->type_id());
                    return false;
                }

                MemLocPtr reg = regalloc.get_memloc(stmt);
                MemLocPtr mem = regalloc.get_memloc(loadop->get_spill());

                InstrPtr mov = std::make_shared<InstrMov>(mem, reg);

                this->_instructions.push_back(mov);

                break;
            }

            default:
                break;
        }
    }

    if(needs_epilogue)
    {
        this->_instructions.push_back(std::make_shared<InstrEpilogue>());
    }

    this->_instructions.push_back(std::make_shared<InstrRet>());

    return true;
}

void CodeGenerator::print(uint32_t isa, uint32_t platform) const noexcept
{
    static std::ostream_iterator<char> out(std::cout);

    std::format_to(out, "CODEGEN\n");

    auto [success, code] = this->as_string(isa, platform);

    std::format_to(out, "{}", code);
}

std::tuple<bool, ByteCode> CodeGenerator::as_bytecode(uint32_t isa, uint32_t platform) const noexcept
{
    ByteCode code;

    for(const auto& instruction : this->_instructions)
    {
        instruction->as_bytecode(code, isa, platform);
    }

    return std::make_tuple(true, code);
}

std::tuple<bool, std::string> CodeGenerator::as_string(uint32_t isa, uint32_t platform) const noexcept
{
    std::string code;

    for(const auto& instruction : this->_instructions)
    {
        instruction->as_string(code, isa, platform);
        std::format_to(std::back_inserter(code), "\n");
    }

    return std::make_tuple(true, code);
}

MATHSEXPR_NAMESPACE_END
