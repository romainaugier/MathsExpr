// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_SSA_OPTIMIZE)
#define __MATHSEXPR_SSA_OPTIMIZE

#include "mathsexpr/ssa.h"

MATHSEXPR_CPP_ENTER

typedef enum {
    SSAOptimizationFlags_ConstantsFolding = 0x1UL,
    SSAOptimizationFlags_DeadCodeElimination = 0x2UL,
    SSAOptimizationFlags_CommonSubexpressionElimination = 0x4UL,
    SSAOptimizationFlags_InlineLiterals = 0x8UL,
    SSAOptimizationFlags_CodeSinking = 0x10UL,
    SSAOptimizationFlags_StrengthReduction = 0x20UL,
    SSAOptimizationFlags_MultiPass = 0x40UL,
    SSAOptimizationFlags_All = 0xFFFFFFFFUL,
} SSAOptimizationFlags;

MATHSEXPR_API bool mathsexpr_ssa_optimize(SSA* ssa, uint64_t optimization_flags);

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_SSA_OPTIMIZE) */
