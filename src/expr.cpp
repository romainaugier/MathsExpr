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
    MATHSEXPR_NOT_IMPLEMENTED;
}

bool Expr::compile(uint64_t debug_flags) noexcept
{
    this->_byte_code.clear();
    this->_variables.clear();

    log_debug("Compiling expression: {}", this->_expr);

    auto [success, tokens] = lexer_lex_expression(this->_expr);

    if(!success)
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

    RegisterAllocator reg_allocator(Platform_Windows, ISA_x86_64);

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
        generator.print(ISA_x86_64, Platform_Windows);
    }

    if(debug_flags & ExprPrintFlags_PrintCodeGeneratorByteCodeAsHexCode)
    {
        auto [success, bytecode] = generator.as_bytecode(ISA_x86_64, Platform_Windows);

        if(!success)
        {
            log_error("Error during bytecode generation for expression: {}", this->_expr);
            log_error("Check the log for more information");
            return false;
        }

        std::string hexcode;
        bytecode_as_hex_string(bytecode, hexcode);

        std::cout << "BYTECODE" << "\n" << hexcode << "\n";
    }

    return true;
}

MATHSEXPR_NAMESPACE_END