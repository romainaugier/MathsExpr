// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_CODEGEN)
#define __MATHSEXPR_CODEGEN

#include "mathsexpr/ssa.h"

MATHSEXPR_CPP_ENTER

typedef enum {
    OperandType_Register,
    OperandType_Stack,
} OperandType;

typedef struct {
    OperandType type;
} Operand;

typedef struct {
    Operand base;
    uint32_t id;
} Register;

typedef struct {
    Operand base;
    uint32_t offset;
} Stack;

#define OPERAND_CAST(__type__, __operand__) ((__type__*)((__operand__)->type == OperandType_##__type__ ? __operand__ : NULL))

typedef enum {
    InstructionType_InstructionUnknown,
    InstructionType_InstructionLoad,
    InstructionType_InstructionStore,
    InstructionType_InstructionNeg,
    InstructionType_InstructionAdd,
    InstructionType_InstructionSub,
    InstructionType_InstructionMul,
    InstructionType_InstructionDiv,
    InstructionType_InstructionMod,
    InstructionType_InstructionPow,
} InstructionType;

typedef struct {
    InstructionType type;
} Instruction;

typedef struct {
    Instruction base;
    Operand source;
    Operand destination;
} InstructionLoad;

typedef struct {
    Instruction base;
    Operand source;
    Operand destination;
} InstructionStore;

typedef struct {
    Instruction base;
    Operand operand;
} UnaryInstruction;

typedef struct {
    Instruction base;
    Operand source;
    Operand destination;
} BinaryInstruction;

typedef struct {
    Instruction base;
    Operand argument;
} FunctionInstruction;

#define INSTRUCTION_CAST(__type__, __instruction__) ((__type__*)((__instruction__)->type == InstructionType_##__type__ ? __instruction__ : NULL))

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_CODEGEN) */