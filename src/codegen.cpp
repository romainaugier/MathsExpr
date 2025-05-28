// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/codegen.h"

/*
    https://www.thejat.in/learn/system-v-amd64-calling-convention
    https://learn.microsoft.com/en-us/cpp/build/x64-software-conventions?view=msvc-170#x64-register-usage
*/

MATHSEXPR_NAMESPACE_BEGIN

static constexpr uint32_t VARIABLES_MEM_ID = 0;
static constexpr uint32_t LITERALS_MEM_ID = 1;

const char* mem_id_to_string(uint32_t mem_id, uint32_t platform) noexcept
{
    switch(platform)
    {
        case CodeGenPlatform_Linux:
            return mem_id == 0 ? "rdi" : "rsi";
        case CodeGenPlatform_Windows:
            return mem_id == 0 ? "rcx" : "rdx";
        default:
            return "???";
    }
}

void Register::to_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept
{
    if(this->is_virtual())
    {
        std::format_to(std::back_inserter(out), "v{}", this->_id);
    }
    else
    {
        std::format_to(std::back_inserter(out), "v{}", this->_id);
    }
}

void Operand::to_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept
{
    return std::visit([&](auto&& arg) -> void {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, Register>) 
        {
            arg.to_string(out, isa, platform);
        }
        else if constexpr (std::is_same_v<T, StackOffset>)
        {
            std::format_to(std::back_inserter(out), "[rbp]");
        }
        else if constexpr (std::is_same_v<T, MemoryAddress>) 
        {
            /* The values for variables are stored in an array which ptr is in rdi */
            if(platform == CodeGenPlatform_Linux)
            {
                if(arg.get_offset() == 0)
                {
                    std::format_to(std::back_inserter(out), "[{}]", mem_id_to_string(arg.get_id(), platform));
                }
                else
                {
                    std::format_to(std::back_inserter(out), 
                                   "[{}+{}]",
                                   mem_id_to_string(arg.get_id(), platform),
                                   arg.get_offset());
                }
            }
        }
        else
        {
            std::format_to(std::back_inserter(out), "?");
        }
    }, this->data);
}

/* Instructions */

void InstructionMov::as_string(std::string& out, uint32_t isa, uint32_t platform) const noexcept
{
    std::format_to(std::back_inserter(out), "mov ");
    this->_dst.to_string(out, isa, platform),
    std::format_to(std::back_inserter(out), ", ");
    this->_src.to_string(out, isa, platform);
}

void InstructionMov::as_bytecode(ByteCode& out, uint32_t isa, uint32_t platform) const noexcept
{
}

/* Code Generator */

bool CodeGenerator::build(const SSA& ssa, SymbolTable& symtable) noexcept
{
    for(const auto statement : ssa.get_statements())
    {
        switch(statement->type_id())
        {
            case SSAStmtTypeId_Variable:
            {
                const SSAStmtVariable* variable = statement_cast<SSAStmtVariable>(statement.get());

                MemoryAddress addr_offset(VARIABLES_MEM_ID, 
                                          symtable.get_variable_offset(variable->get_name()));

                Register reg(variable->get_version());

                InstructionPtr instruction = std::make_shared<InstructionMov>(Operand::from_memory_address(addr_offset),
                                                                              Operand::from_register(reg));

                this->_instructions.push_back(instruction);

                break;
            }
            case SSAStmtTypeId_Literal:
            {
                const SSAStmtLiteral* literal = statement_cast<SSAStmtLiteral>(statement.get());

                MemoryAddress addr_offset(LITERALS_MEM_ID,
                                          symtable.get_literal_offset(literal->get_name()));

                Register reg(literal->get_version());

                InstructionPtr instruction = std::make_shared<InstructionMov>(Operand::from_memory_address(addr_offset),
                                                                              Operand::from_register(reg));

                this->_instructions.push_back(instruction);

                break;
            }
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
