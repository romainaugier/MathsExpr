// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_AST)
#define __MATHSEXPR_AST

#include "mathsexpr/mathsexpr.h"

MATHSEXPR_CPP_ENTER

typedef enum {
    ASTNodeType_ASTLiteral,
    ASTNodeType_ASTVariable,
    ASTNodeType_ASTBinOP,
    ASTNodeType_ASTUnOP,
} ASTNodeType;

typedef enum {
    ASTOpType_Add,
    ASTOpType_Sub,
    ASTOpType_Mul,
    ASTOpType_Div,
    ASTOpType_Neg,
    ASTOpType_Mod,
    ASTOpType_Pow,
} ASTOpType;

typedef struct {
    ASTNodeType type;
} ASTNode;

typedef struct {
    ASTNode base;
    double value;
} ASTLiteral;

typedef struct {
    ASTNode base;
    char name;
} ASTVariable;

typedef struct {
    ASTNode base;
    ASTOpType op;
    ASTNode* left;
    ASTNode* right;
} ASTBinOP;

typedef struct {
    ASTNode base;
    ASTOpType op;
    ASTNode* operand;
} ASTUnOP;

#define AST_CAST(type, node) ((type*)((node)->type == ASTNodeType_##type ? node : NULL))

MATHSEXPR_API uint32_t mathsexpr_parse(const char* expression, uint32_t expression_size);

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_AST) */