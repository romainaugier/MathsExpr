// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved.

#include "mathsexpr/expr.h"
#include "mathsexpr/log.h"
#include "mathsexpr/codegen.h"

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

    CodeGenerator generator;

    if(!generator.build(ssa, symtable))
    {
        log_error("Error while building CodeGenerator for expression: {}", this->_expr);
        log_error("Check the log for more information");
        return false;
    }

    if(debug_flags & ExprPrintFlags_PrintCodeGeneratorAsString)
    {
        generator.print(CodeGenISA_x86_64, CodeGenPlatform_Linux);
    }

    return true;
}

MATHSEXPR_NAMESPACE_END