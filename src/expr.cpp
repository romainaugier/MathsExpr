// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/expr.hpp"
#include "mathsexpr/log.hpp"
#include "mathsexpr/codegen.hpp"
#include "mathsexpr/regalloc.hpp"

MATHSEXPR_NAMESPACE_BEGIN

std::tuple<bool, double> Expr::_evaluate_internal(const double* values) const noexcept
{
    if(!this->_exec_mem.is_locked())
    {
        log_error("ExecMem is not locked nor ready, compile expr before evaluating it");
        return std::make_tuple(false, 0.0);
    }

    auto exec_func = this->_exec_mem.as_function();

    double result = exec_func(values, this->_literals.data());

    return std::make_tuple(true, result);
}

bool Expr::compile(uint64_t debug_flags) noexcept
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

    if(debug_flags & ExprPrintFlags_PrintAST)
    {
        ast.print();
    }

    SymbolTable symtable;

    symtable.collect(ast);

    if(debug_flags & ExprPrintFlags_PrintSymTable)
    {
        symtable.print();
    }

    /* Variables and literals are stored in order of parsing */
    for(auto [name, _] : symtable.get_variables())
    {
        this->_variables.insert(name);
    }

    for(auto [_, lit] : symtable.get_literals())
    {
        this->_literals.push_back(lit.get_value());
    }

    SSA ssa;

    if(!ssa.build_from_ast(ast))
    {
        log_error("Error while building SSA for expression: {}", this->_expr);
        log_error("Check the log for more information");
        return false;
    }

    if(debug_flags & ExprPrintFlags_PrintSSA)
    {
        ssa.print();
    }

    RegisterAllocator reg_allocator(platform, isa);

    if(!reg_allocator.allocate(ssa, symtable))
    {
        log_error("Error during register allocation for expression: {}", this->_expr);
        log_error("Check the log for more information");
        return false;
    }

    if(debug_flags & ExprPrintFlags_PrintSSARegisterAlloc)
    {
        ssa.print();
    }

    CodeGenerator generator;

    if(!generator.build(ssa, reg_allocator, symtable))
    {
        log_error("Error while building CodeGenerator for expression: {}", this->_expr);
        log_error("Check the log for more information");
        return false;
    }

    if(debug_flags & ExprPrintFlags_PrintCodeGeneratorAsString)
    {
        generator.print(isa, platform);
    }

    auto [gen_success, bytecode] = generator.as_bytecode(isa, platform);

    if(!gen_success)
    {
        log_error("Error during bytecode generation for expression: {}", this->_expr);
        log_error("Check the log for more information");
        return false;
    }

    if(debug_flags & ExprPrintFlags_PrintCodeGeneratorByteCodeAsHexCode)
    {
        std::string hexcode;

        switch(isa)
        {
            case ISA_x86_64:
                bytecode_as_hex_string(bytecode, hexcode, x86_64::prefixes);
        }

        std::cout << "BYTECODE" << "\n" << hexcode << "\n";
    }

    ExecMem exec_mem(bytecode.size() * sizeof(std::byte));

    if(!exec_mem.write(bytecode))
    {
        return false;
    }

    if(!exec_mem.lock())
    {
        return false;
    }

    this->_exec_mem = std::move(exec_mem);

    log_debug("Compiled expression: {}", this->_expr);
    log_debug("Ready to be evaluated");

    return true;
}

MATHSEXPR_NAMESPACE_END