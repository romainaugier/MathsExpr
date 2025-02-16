// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_AST)
#define __MATHSEXPR_AST

#include "mathsexpr/parser.h"
#include "mathsexpr/arena.h"

#include "libromano/vector.h"

MATHSEXPR_CPP_ENTER

typedef enum {
    ASTNodeType_ASTLiteral,
    ASTNodeType_ASTVariable,
    ASTNodeType_ASTBinOP,
    ASTNodeType_ASTUnOP,
} ASTNodeType;

typedef enum {
    ASTUnOPType_Neg,
} ASTUnOPType;

typedef enum {
    ASTBinOPType_Add,
    ASTBinOPType_Sub,
    ASTBinOPType_Mul,
    ASTBinOPType_Div,
    ASTBinOPType_Mod,
    ASTBinOPType_Pow,
} ASTBinOPType;

typedef struct {
    ASTNodeType type;
} ASTNode;

typedef struct {
    ASTNode base;
    float value;
} ASTLiteral;

typedef struct {
    ASTNode base;
    char name;
} ASTVariable;

typedef struct {
    ASTNode base;
    ASTBinOPType op;
    ASTNode* left;
    ASTNode* right;
} ASTBinOP;

typedef struct {
    ASTNode base;
    ASTUnOPType op;
    ASTNode* operand;
} ASTUnOP;

#define AST_CAST(__type__, __node__) ((__type__*)((__node__)->type == ASTNodeType_##__type__ ? __node__ : NULL))

typedef struct 
{
    Arena nodes;
    ASTNode* root;
} AST;

MATHSEXPR_API AST* mathsexpr_ast_new();

MATHSEXPR_API ASTNode* mathsexpr_ast_new_literal(AST* ast, float value);

MATHSEXPR_API ASTNode* mathsexpr_ast_new_variable(AST* ast, char name);

MATHSEXPR_API ASTNode* mathsexpr_ast_new_binop(AST* ast, ASTBinOPType op, ASTNode* left, ASTNode* right);

MATHSEXPR_API ASTNode* mathsexpr_ast_new_unop(AST* ast, ASTUnOPType op, ASTNode* operand);

MATHSEXPR_API bool mathsexpr_ast_from_infix_parser_tokens(AST* ast, Vector* tokens);

MATHSEXPR_API bool mathsexpr_ast_from_postfix_parser_tokens(AST* ast, Vector* tokens);

MATHSEXPR_API void mathsexpr_ast_print(AST* ast); 

MATHSEXPR_API void mathsexpr_ast_destroy(AST* ast);

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_AST) */