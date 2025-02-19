// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_SSA_REGISTER_ALLOC)
#define __MATHSEXPR_SSA_REGISTER_ALLOC

#include "mathsexpr/ssa.h"

MATHSEXPR_CPP_ENTER

typedef struct {
    uint32_t destination;
    uint16_t start;
    uint16_t end;
} SSALiveInterval;

MATHSEXPR_API void mathsexpr_ssa_get_live_intervals(SSA* ssa, Vector* intervals);

MATHSEXPR_API void mathsexpr_ssa_print_live_intervals(Vector* intervals);

MATHSEXPR_API bool mathsexpr_ssa_allocate_registers(SSA* ssa);

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_SSA_REGISTER_ALLOC) */