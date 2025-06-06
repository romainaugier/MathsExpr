// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/codegen.hpp"
#include "mathsexpr/op.hpp"
#include "mathsexpr/log.hpp"

/*
    https://www.felixcloutier.com/x86/
*/

MATHSEXPR_NAMESPACE_BEGIN

/* Memory instructions */

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

/* Func ops instructions */

void InstrCall::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept
{

}

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

/* Terminator instructions */

void InstrRet::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept
{

}

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
