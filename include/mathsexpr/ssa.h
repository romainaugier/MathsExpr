// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_SSA)
#define __MATHSEXPR_SSA

#include "mathsexpr/ast.h"

#include "libromano/hashmap.h"

MATHSEXPR_CPP_ENTER

typedef enum {
    SSAInstructionType_SSALiteral = 1,
    SSAInstructionType_SSAVariable = 2,
    SSAInstructionType_SSABinOP = 3,
    SSAInstructionType_SSAUnOP = 4,
    SSAInstructionType_SSAFunction = 5,
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

typedef enum {
    SSAFuncType_Abs,
    SSAFuncType_Exp,
    SSAFuncType_Sqrt,
    SSAFuncType_Rcp,
    SSAFuncType_Rsqrt,
    SSAFuncType_Log,
    SSAFuncType_Log2,
    SSAFuncType_Log10,
    SSAFuncType_Floor,
    SSAFuncType_Ceil,
    SSAFuncType_Frac,
    SSAFuncType_Acos,
    SSAFuncType_Asin,
    SSAFuncType_Atan,
    SSAFuncType_Cos,
    SSAFuncType_Sin,
    SSAFuncType_Tan,
    SSAFuncType_Cosh,
    SSAFuncType_Sinh,
    SSAFuncType_Tanh,
} SSAFuncType;

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
    char name[VARIABLE_LENGTH_MAX];
    uint32_t name_length;
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

typedef struct {
    SSAInstruction base;
    SSAFuncType func;
    SSAInstruction* argument;
    uint32_t destination;
} SSAFunction;

#define SSA_CAST(__type__, __instruction__) ((__type__*)((__instruction__)->type == SSAInstructionType_##__type__ ? __instruction__ : NULL))

typedef enum {
    SSAFlags_InlineLiterals = 0x1,
} SSAFlags;

typedef struct {
    Arena instructions_data;
    Vector* instructions;
    HashMap* functions_lookup_table;
    uint32_t counter;
    uint64_t flags;
} SSA;

MATHSEXPR_API SSA* mathsexpr_ssa_new();

MATHSEXPR_API SSAInstruction* mathsexpr_ssa_new_literal(SSA* ssa, 
                                                        float value, 
                                                        uint32_t destination);

MATHSEXPR_API SSAInstruction* mathsexpr_ssa_new_variable(SSA* ssa,
                                                         char* name,
                                                         uint32_t name_length,
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

MATHSEXPR_API SSAInstruction* mathsexpr_ssa_new_function(SSA* ssa,
                                                         SSAFuncType type,
                                                         SSAInstruction* argument,
                                                         uint32_t destination);

MATHSEXPR_FORCE_INLINE size_t mathsexpr_ssa_num_instructions(SSA* ssa)
{
    return vector_size(ssa->instructions);
}

MATHSEXPR_FORCE_INLINE SSAInstruction* mathsexpr_ssa_instruction_at(SSA* ssa, size_t i)
{
    return *((SSAInstruction**)vector_at(ssa->instructions, i));
}

MATHSEXPR_API uint32_t mathsexpr_ssa_get_instruction_destination(SSAInstruction* instruction);

MATHSEXPR_API bool mathsexpr_ssa_from_ast(SSA* ssa, AST* ast);

MATHSEXPR_API void mathsexpr_ssa_print(SSA* ssa);

MATHSEXPR_API void mathsexpr_ssa_destroy(SSA* ssa);

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_SSA) */