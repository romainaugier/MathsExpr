// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathexpr/codegen.hpp"
#include "mathexpr/op.hpp"
#include "mathexpr/log.hpp"

MATHEXPR_NAMESPACE_BEGIN

/* Code Generation */

CodeGenerator::CodeGenerator(uint32_t isa, PlatformABIPtr platform_abi) : _isa(isa), 
                                                                          _platform_abi(platform_abi)
{
    this->_target_generator = CodeGenerator::create_target_generator(isa, platform_abi);

    if(this->_target_generator == nullptr)
    {
        log_error("Cannot create code generator, unsupported isa: {}", 
                  isa_as_string(this->_isa));
    }
}

bool CodeGenerator::build(const SSA& ssa,
                          const RegisterAllocator& regalloc,
                          SymbolTable& symtable) noexcept
{
    if(this->_target_generator == nullptr)
    {
        log_error("Cannot build code generator, unsupported isa: {}", 
                  isa_as_string(this->_isa));

        return false;
    }

    this->_instructions.clear();

    uint64_t epilogue_stack_size = 0;

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
                    MemLocPtr mem = std::make_shared<Memory>(this->_platform_abi->get_variable_base_ptr(), 
                                                            symtable.get_variable_offset(variable->get_name()));

                    this->_instructions.push_back(this->_target_generator->create_mov(mem, loc));
                }

                break;
            }
            case SSAStmtTypeId_Literal:
            {
                auto literal = statement_cast<SSAStmtLiteral>(stmt.get());

                MemLocPtr loc = regalloc.get_memloc(stmt);

                if(loc->type_id() == MemLocTypeId_Register)
                {
                    MemLocPtr mem = std::make_shared<Memory>(this->_platform_abi->get_literal_base_ptr(), 
                                                             symtable.get_literal_offset(literal->get_name()));

                    this->_instructions.push_back(this->_target_generator->create_mov(mem, loc));
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
                        this->_instructions.push_back(this->_target_generator->create_add(left, right));
                        break;
                    }
                    case BinaryOpType_Sub:
                    {
                        this->_instructions.push_back(this->_target_generator->create_sub(left, right));
                        break;
                    }
                    case BinaryOpType_Mul:
                    {
                        this->_instructions.push_back(this->_target_generator->create_mul(left, right));
                        break;
                    }
                    case BinaryOpType_Div:
                    {
                        this->_instructions.push_back(this->_target_generator->create_div(left, right));
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

                this->_instructions.push_back(this->_target_generator->create_call(funcop->get_name()));

                break;
            }
            case SSAStmtTypeId_AllocateStackOp:
            {
                auto allocstackop = statement_cast<SSAStmtAllocateStackOp>(stmt.get());

                if(allocstackop == nullptr)
                {
                    log_error("Internal error during codegen: expected alloc stack op, got: {}",
                              stmt->type_id());
                }

                epilogue_stack_size = allocstackop->get_stack_size();

                this->_instructions.insert(this->_instructions.begin(), 
                                           this->_target_generator->create_prologue(allocstackop->get_stack_size()));

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

                this->_instructions.push_back(this->_target_generator->create_mov(reg, mem));

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

                this->_instructions.push_back(this->_target_generator->create_mov(mem, reg));

                break;
            }

            default:
                break;
        }
    }

    if(epilogue_stack_size > 0)
    {
        this->_instructions.push_back(this->_target_generator->create_epilogue(epilogue_stack_size));
    }

    this->_instructions.push_back(this->_target_generator->create_ret());

    return true;
}

std::tuple<bool, ByteCode> CodeGenerator::as_bytecode() const noexcept
{
    ByteCode code;

    for(const auto& instruction : this->_instructions)
    {
        instruction->as_bytecode(code);
    }

    return std::make_tuple(true, code);
}

std::tuple<bool, std::string> CodeGenerator::as_string() const noexcept
{
    std::string code;

    for(const auto& instruction : this->_instructions)
    {
        instruction->as_string(code);
        std::format_to(std::back_inserter(code), "\n");
    }

    return std::make_tuple(true, code);
}

std::tuple<bool, std::string> CodeGenerator::as_bytecode_hex_string() const noexcept
{
    std::string hexcode;

    ByteCode tmp;
    tmp.reserve(16);

    for(const auto& instruction : this->_instructions)
    {
        tmp.clear();
        instruction->as_bytecode(tmp);

        for(const auto& byte : tmp)
        {
            std::format_to(std::back_inserter(hexcode), "{}", byte);
        }

        std::format_to(std::back_inserter(hexcode), " ; ");

        instruction->as_string(hexcode);

        std::format_to(std::back_inserter(hexcode), "\n");
    }

    return std::make_tuple(true, hexcode);
}

TargetCodeGeneratorPtr CodeGenerator::create_target_generator(uint32_t isa,
                                                              PlatformABIPtr platform_abi) noexcept
{
    return TargetRegistry::create_target(isa, platform_abi);
}

/* Registry */

std::unordered_map<uint32_t, TargetRegistry::TargetFactory>& TargetRegistry::get_registry() noexcept
{
    static std::unordered_map<uint32_t, TargetFactory> registry;
    return registry;
}

void TargetRegistry::register_target(uint32_t isa, TargetFactory factory) noexcept
{
    auto& registry = get_registry();
    
    if(registry.find(isa) != registry.end()) 
    {
        log_error("Target ISA {} is already registered", isa_as_string(isa));
        return;
    }
    
    registry[isa] = std::move(factory);
}

TargetCodeGeneratorPtr TargetRegistry::create_target(uint32_t isa, 
                                                     PlatformABIPtr platform_abi) noexcept
{
    auto& registry = get_registry();
    
    auto it = registry.find(isa);

    if(it == registry.end()) 
    {
        return nullptr;
    }
    
    return it->second(platform_abi);
}

std::unordered_set<uint32_t> TargetRegistry::get_supported_isas() noexcept
{
    auto& registry = get_registry();
    std::unordered_set<uint32_t> isas;
    
    isas.reserve(registry.size());

    for(const auto& [isa, _] : registry) 
    {
        isas.insert(isa);
    }
    
    return isas;
}

bool TargetRegistry::is_supported(uint32_t isa, PlatformABIPtr platform_abi) noexcept 
{
    auto& registry = get_registry();
    
    auto it = registry.find(isa);

    if (it == registry.end()) 
    {
        return false;
    }
    
    auto target = it->second(platform_abi);

    return target->is_valid();
}

MATHEXPR_NAMESPACE_END
