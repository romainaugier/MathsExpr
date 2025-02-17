// SPDX-License-Identifier: BSD-3-Clause 
// Copyright (c) 2025 - Present Romain Augier
// All rights reserved. 

#include "mathsexpr/arena.h"

#include <stdlib.h>
#include <string.h>

void mathsexpr_arena_init(Arena* arena, const size_t size)
{
    arena->ptr = malloc(size);
    arena->offset = 0;
    arena->capacity = size;
}

MATHSEXPR_FORCE_INLINE bool mathsexpr_arena_check_resize(Arena* arena, 
                                                         const size_t new_size)
{
    return (arena->offset + new_size) >= arena->capacity;
}

void mathsexpr_arena_resize(Arena* arena)
{
    const size_t new_capacity = (size_t)((float)arena->capacity * ARENA_GROWTH_RATE);

    void* new_ptr = realloc(arena->ptr, new_capacity);

    MATHSEXPR_ASSERT(new_ptr != NULL, "Error during arena reallocation");

    arena->ptr = new_ptr;
    arena->capacity = new_capacity;
}

void* mathsexpr_arena_push(Arena* arena, void* data, const size_t data_size)
{
    if(mathsexpr_arena_check_resize(arena, data_size))
    {
        mathsexpr_arena_resize(arena);
    }

    void* data_address = (void*)((char*)arena->ptr + arena->offset);
    memcpy(data_address, data, data_size);

    arena->offset += data_size;

    return data_address;
}

void* mathsexpr_arena_at(Arena* arena, const size_t offset)
{
    MATHSEXPR_ASSERT(offset < arena->capacity, "offset too large");

    void* data_address = (char*)arena->ptr + offset;

    return data_address;
}

void mathsexpr_arena_destroy(Arena* arena)
{
    free(arena->ptr);
    arena->ptr = NULL;
    arena->offset = 0;
    arena->capacity = 0;
}