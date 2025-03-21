
// Copyright 2024, 2025 Athena Boose

// This file is part of badsh.

// badsh is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.

// badsh is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with badsh. If not, see <https://www.gnu.org/licenses/>. 

#pragma once

#include <stddef.h>

typedef struct cu_arena {
    struct arena_elem *first;
    size_t default_block_size;
} cu_arena;

// Creates a new arena with a requested block size.
void arena_new(cu_arena *arena, size_t block_size);

void *arena_alloc(cu_arena *arena, size_t item_size);

void arena_free(cu_arena *arena);




// if you have a custom allocator, use the *_size functions to see what size buffer will be needed, allocate the buffer yourself, then pass that buffer to the matching *_buf function

size_t arena_alloc_size(cu_arena *arena, size_t item_size); // if no allocation is required, this function will return 0
void *arena_alloc_buf(cu_arena *arena, size_t item_size, void *buffer); // if arena_alloc_size returned 0, you may safely pass NULL in as `buffer`


// returns one of the pointers you gave to arena_new_buf or arena_alloc_buf
// you must continue calling this function until it returns NULL to successfully free the arena
void *arena_free_buf(cu_arena *arena);


