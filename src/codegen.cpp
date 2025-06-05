// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/codegen.hpp"
#include "mathsexpr/op.hpp"

/*
    https://www.thejat.in/learn/system-v-amd64-calling-convention
    https://learn.microsoft.com/en-us/cpp/build/x64-software-conventions?view=msvc-170#x64-register-usage
    https://www.felixcloutier.com/x86/
*/

MATHSEXPR_NAMESPACE_BEGIN

/* Instructions */

void InstrMov::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept 
{
    switch(platform)
    {
        case Platform_Linux:
        case Platform_Windows:
            std::format_to(std::back_inserter(out), "mov ");
            this->_mem_loc_to->as_string(out, isa, platform);
            std::format_to(std::back_inserter(out), ", ");
            this->_mem_loc_from->as_string(out, isa, platform);
            break;
    }
}

void InstrMov::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept 
{

}

/* Binop instructions */

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

}

/* Code Generation */

bool CodeGenerator::build(const SSA& ssa,
                          const RegisterAllocator& regalloc,
                          SymbolTable& symtable) noexcept
{
    this->_instructions.clear();

    for(auto stmt : ssa.get_statements())
    {
        switch(stmt->type_id())
        {
            case SSAStmtTypeId_Variable:
            {
                const SSAStmtVariable* variable = statement_const_cast<SSAStmtVariable>(stmt.get());

                MemLocPtr reg = regalloc.get_memloc(stmt);
                MemLocPtr mem = std::make_shared<Memory>(MemLocRegister_Variables, 
                                                         symtable.get_variable_offset(variable->get_name()));

                InstrPtr mov = std::make_shared<InstrMov>(mem, reg);

                this->_instructions.push_back(mov);

                break;
            }
            case SSAStmtTypeId_Literal:
            {
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
                MemLocPtr right = regalloc.get_memloc(binop->get_right());

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
                break;
            }
            case SSAStmtTypeId_AllocateStackOp:
            {
                break;
            }
            case SSAStmtTypeId_SpillOp:
            {
                break;
            }
            case SSAStmtTypeId_LoadOp:
            {
                break;
            }

            default:
                break;
        }
    }

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
