
// Copyright 2024, 2025 Athena Boose

// This file is part of c-utils.

// c-utils is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.

// c-utils is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with c-utils. If not, see <https://www.gnu.org/licenses/>. 

#pragma once

#include <stddef.h>

struct cu_allocator {
	void *(*alloc)(size_t amount, void *ctx); // May return NULL if a failure occurs
	void (*free)(void *mem, size_t amount, void *ctx); // Should no-op on NULL `mem`
	void *(*realloc)(void *mem, size_t newsize, size_t oldsize, void *ctx); // nullable, if NULL a combination of alloc and free will be used instead
	void *ctx; // passed into all functions
};


// generic functions 

void *cu_allocator_realloc(void *mem, size_t newsize, size_t oldsize, const struct cu_allocator *alloc);

void *cu_allocator_reallocarray(void *mem, size_t new_nel, size_t old_nel, size_t elem_size, const struct cu_allocator *alloc);

struct cu_arena {
	struct arena_elem *first;
	size_t default_block_size;
	struct cu_allocator *allocator; // If this is NULL, standard allocator facilities will be used.
};

// Creates a new arena with a requested block size.
void cu_arena_new(struct cu_arena *arena, size_t block_size);

void *cu_arena_alloc(struct cu_arena *arena, size_t item_size);

void cu_arena_free(struct cu_arena *arena);

