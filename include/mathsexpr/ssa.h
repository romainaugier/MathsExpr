// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_SSA)
#define __MATHSEXPR_SSA

#include "mathsexpr/ast.h"

MATHSEXPR_CPP_ENTER

typedef enum {
    SSAInstructionType_SSALiteral = 1,
    SSAInstructionType_SSAVariable = 2,
    SSAInstructionType_SSABinOP = 3,
    SSAInstructionType_SSAUnOP = 4,
} SSAInstructionType;

typedef enum {
    SSAUnOPType_Neg,
} SSAUnOPType;

typedef enum {
    SSABinOPType_Add,
    SSABinOPType_Sub,
    SSABinOPType_Mul,
    SSABinOPType_Div,
    SSABinOPType_Mod,
    SSABinOPType_Pow,
} SSABinOPType;

typedef struct {
    SSAInstructionType type;
} SSAInstruction;

typedef struct {
    SSAInstruction base;
    float value;
    uint32_t destination;
} SSALiteral;

typedef struct {
    SSAInstruction base;
    char name;
    uint32_t destination;
} SSAVariable;

typedef struct {
    SSAInstruction base;
    SSABinOPType op;
    SSAInstruction* left;
    SSAInstruction* right;
    uint32_t destination;
} SSABinOP;

typedef struct {
    SSAInstruction base;
    SSAUnOPType op;
    SSAInstruction* operand;
    uint32_t destination;
} SSAUnOP;

#define SSA_CAST(__type__, __instruction__) ((__type__*)((__instruction__)->type == SSAInstructionType_##__type__ ? __instruction__ : NULL))

typedef uint32_t SSAIterator;

typedef struct
{
    Arena instructions;
    uint32_t num_instructions;
    uint32_t counter;
} SSA;

MATHSEXPR_API SSA* mathsexpr_ssa_new();

MATHSEXPR_API SSAInstruction* mathsexpr_ssa_new_literal(SSA* ssa, 
                                                        float value, 
                                                        uint32_t destination);

MATHSEXPR_API SSAInstruction* mathsexpr_ssa_new_variable(SSA* ssa,
                                                         char name,
                                                         uint32_t destination);

MATHSEXPR_API SSAInstruction* mathsexpr_ssa_new_binop(SSA* ssa, 
                                                      SSABinOPType op, 
                                                      SSAInstruction* left,
                                                      SSAInstruction* right,
                                                      uint32_t destination);

MATHSEXPR_API SSAInstruction* mathsexpr_ssa_new_unop(SSA* ssa,
                                                     SSAUnOPType op,
                                                     SSAInstruction* operand,
                                                     uint32_t destination);

MATHSEXPR_API SSAIterator mathsexpr_ssa_iterator_new();

MATHSEXPR_API bool mathsexpr_ssa_next_instruction(SSA* ssa, 
                                                  SSAIterator* it,
                                                  SSAInstruction** instruction);

MATHSEXPR_API bool mathsexpr_ssa_from_ast(SSA* ssa, AST* ast);

MATHSEXPR_API bool mathsexpr_ssa_optimize(SSA* ssa);

MATHSEXPR_API void mathsexpr_ssa_print(SSA* ssa);

MATHSEXPR_API void mathsexpr_ssa_destroy(SSA* ssa);

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_SSA) */