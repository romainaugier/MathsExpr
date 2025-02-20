// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_SSA_REGISTER_ALLOC)
#define __MATHSEXPR_SSA_REGISTER_ALLOC

#include "mathsexpr/ssa.h"

MATHSEXPR_CPP_ENTER

#define NO_REG 0xFFFF

typedef struct {
    SSAInstruction* instruction;
    uint16_t reg;
    uint16_t start;
    uint16_t end;
    uint16_t last_use;
} SSALiveInterval;

MATHSEXPR_API void mathsexpr_ssa_get_live_intervals(SSA* ssa, Vector* intervals);

MATHSEXPR_API void mathsexpr_ssa_print_live_intervals(Vector* intervals);

MATHSEXPR_API bool mathsexpr_ssa_allocate_registers(SSA* ssa, uint32_t num_registers);

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_SSA_REGISTER_ALLOC) */