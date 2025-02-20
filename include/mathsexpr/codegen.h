// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_CODEGEN)
#define __MATHSEXPR_CODEGEN

#include "mathsexpr/ssa.h"

#include "libromano/string.h"

MATHSEXPR_CPP_ENTER

typedef enum {
    CodeGenFlags_PlatformWindows = 0x1,
    CodeGenFlags_PlatformLinux = 0x2,
    CodeGenFlags_Float32 = 0x4,
    CodeGenFlags_Float64 = 0x8,
    CodeGenFlags_Scalar = 0x10,
    CodeGenFlags_Vector4 = 0x20,
    CodeGenFlags_Vector8 = 0x40,
} CodeGenFlags;

MATHSEXPR_API void mathsexpr_codegen_to_text(SSA* ssa, String* output, uint64_t flags);

MATHSEXPR_API void mathsexpr_codegen_to_bytecode(SSA* ssa, Arena* output, uint64_t flags);

MATHSEXPR_CPP_END

#endif /* !defined(__MATHSEXPR_CODEGEN) */