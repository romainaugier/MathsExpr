// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#pragma once

#if !defined(__MATHEXPR_EXPR)
#define __MATHEXPR_EXPR

#include "mathexpr/constants.hpp"
#include "mathexpr/execmem.hpp"
#include "mathexpr/string_hash.hpp"
#include "mathexpr/log.hpp"
#include "mathexpr/codegen.hpp"
#include "mathexpr/regalloc.hpp"

#include <string>
#include <unordered_map>
#include <set>
#include <vector>
#include <array>

MATHEXPR_NAMESPACE_BEGIN

enum ExprFlags : uint64_t
{
    ExprFlags_PrintAST = 0x1,
    ExprFlags_PrintSymTable = 0x2,
    ExprFlags_PrintSSA = 0x4,
    ExprFlags_PrintSSAOptimized = 0x8,
    ExprFlags_PrintSSAOptimizationSteps = 0x10,
    ExprFlags_PrintSSARegisterAlloc = 0x20,
    ExprFlags_PrintCodeGeneratorAsString = 0x40,
    ExprFlags_PrintCodeGeneratorByteCodeAsHexCode = 0x80,
    ExprFlags_PrintCodeGeneratorRelocations = 0x100,
    ExprFlags_PrintAll = 0x1FF,
    ExprFlags_DoublePrecision = 0x200,
};

template<typename T>
using Variables = std::unordered_map<std::string, T, string_hash, std::equal_to<>>;

template<typename T = double>
class Expr
{
    static_assert(std::is_floating_point_v<T> && "T must be a floating-point type");

    std::string _expr;

    ExecMem _exec_mem;

    std::set<std::string_view> _variables;
    std::vector<T> _literals;

    std::tuple<bool, T> _evaluate_internal(const T* values) const noexcept
    {
        MATHEXPR_ASSERT(values != nullptr, "values is NULL");

        if(!this->_exec_mem.is_locked())
        {
            log_error("ExecMem is not locked nor ready, compile expr before evaluating it");
            return std::make_tuple(false, static_cast<T>(0.0));
        }

        auto exec_func = this->_exec_mem.as_function();

        T result = exec_func(values, this->_literals.data());

        return std::make_tuple(true, result);
    }

public:
    Expr(std::string expr) : _expr(std::move(expr)) {}

    bool compile(uint64_t flags) noexcept
    {
        uint32_t platform = get_current_platform();

        if(platform == Platform_Invalid)
        {
            log_error("Current platform is not supported");
            return false;
        }

        uint32_t isa = get_current_isa();

        if(isa == ISA_Invalid)
        {
            log_error("Current isa is not supported");
            return false;
        }

        PlatformABIPtr platform_abi = get_current_platform_abi(isa, platform);

        if(platform_abi == nullptr)
        {
            log_error("Current ABI is not supported");
            return false;
        }

        this->_variables.clear();
        this->_literals.clear();

        log_debug("Compiling expression: {}", this->_expr);

        auto [lex_success, tokens] = lexer_lex_expression(this->_expr);

        if(!lex_success)
        {
            log_error("Error while lexing expression: {}", this->_expr);
            log_error("Check the log for more information");
            return false;
        }

        AST ast;

        if(!ast.build_from_tokens(tokens))
        {
            log_error("Error while building AST for expression: {}", this->_expr);
            log_error("Check the log for more information");
            return false;
        }

        if(flags & ExprFlags_PrintAST)
            ast.print();

        SymbolTable symtable;

        symtable.collect(ast);

        if(flags & ExprFlags_PrintSymTable)
            symtable.print();

        /* Variables and literals are stored in order of parsing */
        for(auto [name, _] : symtable.get_variables())
            this->_variables.insert(name);

        for(auto [_, lit] : symtable.get_literals())
            this->_literals.push_back(lit.get_value());

        SSA ssa;

        if(!ssa.build_from_ast(ast))
        {
            log_error("Error while building SSA for expression: {}", this->_expr);
            log_error("Check the log for more information");
            return false;
        }

        if(flags & ExprFlags_PrintSSA)
            ssa.print();

        RegisterAllocator reg_allocator(platform_abi);

        if(!reg_allocator.allocate(ssa, symtable))
        {
            log_error("Error during register allocation for expression: {}", this->_expr);
            log_error("Check the log for more information");
            return false;
        }

        if(flags & ExprFlags_PrintSSARegisterAlloc)
            ssa.print();

        CodeGenerator generator(isa, platform_abi);

        if(!generator.build(ssa, reg_allocator, symtable))
        {
            log_error("Error while building CodeGenerator for expression: {}", this->_expr);
            log_error("Check the log for more information");
            return false;
        }

        if(flags & ExprFlags_PrintCodeGeneratorAsString)
        {
            auto [gen_str_success, code] = generator.as_string();

            if(!gen_str_success)
            {
                log_error("Error during code generation for expression: {}", this->_expr);
                log_error("Check the log for more information");

                return false;
            }

            std::cout << "CODEGEN\n" << code << "\n";
        }

        Relocations relocs;

        auto [gen_success, bytecode] = generator.as_bytecode(relocs);

        if(!gen_success)
        {
            log_error("Error during bytecode generation for expression: {}", this->_expr);
            log_error("Check the log for more information");
            return false;
        }

        if(flags & ExprFlags_PrintCodeGeneratorRelocations)
        {
            std::cout << "RELOCATIONS (" << relocs.size() << ")\n";
            std::cout << "\n";
        }

        if(!relocate(bytecode, relocs))
        {
            log_error("Error during relocation for expression: {}", this->_expr);
            log_error("Check the log for more information");
            return false;
        }

        if(flags & ExprFlags_PrintCodeGeneratorByteCodeAsHexCode)
        {
            std::cout << "BYTECODE" << "\n";

            for(const auto byte : bytecode)
                std::cout << std::format("{:02x}", static_cast<uint8_t>(byte));

            std::cout << "\n\n";
        }

        ExecMem exec_mem(bytecode.size() * sizeof(std::byte));

        if(!exec_mem.write(bytecode))
            return false;

        if(!exec_mem.lock())
            return false;

        this->_exec_mem = std::move(exec_mem);

        log_debug("Compiled expression: {}", this->_expr);
        log_debug("Ready to be evaluated");

        return true;
    }

    template<typename... Args>
        requires (std::same_as<std::remove_cvref_t<Args>, T> && ...)
    std::tuple<bool, T> evaluate(Args&&... args) const noexcept
    {
        if(sizeof...(Args) != this->_variables.size())
        {
            log_error("You passed {} arguments but the expression needs {}",
                      sizeof...(Args),
                      this->_variables.size());

            return std::make_tuple(false, static_cast<T>(0.0));
        }

        alignas(16) std::array<T, sizeof...(Args)> values;

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                ((values[Is] = static_cast<T>(args)), ...);
            }(std::make_index_sequence<sizeof...(Args)>{});

        return this->_evaluate_internal(values.data());
    }

    std::tuple<bool, T> evaluate(const Variables<T>& variables) const noexcept
    {
        if(variables.size() != this->_variables.size())
        {
            return std::make_tuple(false, static_cast<T>(0.0));
        }

        std::vector<T> values;
        values.reserve(this->_variables.size());

        for(const std::string_view& variable : this->_variables)
        {
            const auto it = variables.find(variable);

            if(it == variables.end())
            {
                return std::make_tuple(false, static_cast<T>(0.0));
            }

            values.push_back(it->second);
        }

        return this->_evaluate_internal(values.data());
    }
};

MATHEXPR_NAMESPACE_END

#endif /* !defined(__MATHEXPR_EXPR) */
