// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#pragma once

#if !defined(__MATHSEXPR_ARENA)
#define __MATHSEXPR_ARENA

#include "mathsexpr/mathsexpr.h"

#define ARENA_GROWTH_RATE 1.6180339887f

typedef struct
{
    void* ptr;
    size_t capacity;
    size_t offset;
} Arena;

MATHSEXPR_API void mathsexpr_arena_init(Arena* arena, const size_t size);

MATHSEXPR_API void mathsexpr_arena_resize(Arena* arena);

MATHSEXPR_API void* mathsexpr_arena_push(Arena* arena, void* data, const size_t data_size);

MATHSEXPR_API void* mathsexpr_arena_at(Arena* arena, const size_t offset);

MATHSEXPR_API void mathsexpr_arena_destroy(Arena* arena);

#endif /* !defined(__MATHSEXPR_ARENA) */