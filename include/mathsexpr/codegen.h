// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_CODEGEN)
#define __MATHSEXPR_CODEGEN

#include "mathsexpr/ssa.h"

MATHSEXPR_CPP_ENTER

typedef enum {
    InstructionType_InstructionLoad,
} InstructionType;

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_CODEGEN) */