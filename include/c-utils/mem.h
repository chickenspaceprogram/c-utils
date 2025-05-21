
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <stddef.h>

struct cu_allocator {
	// Required, setting alloc = NULL is undefined behavior
	void *(*alloc)(size_t amount, void *ctx); // May return NULL if a failure occurs

	// Nullable
	
	// If free == NULL, free will be a no-op.
	void (*free)(void *mem, size_t amount, void *ctx); // Should no-op on NULL `mem`
	// If realloc == NULL, a manual alloc, memcpy, free will take place.
	void *(*realloc)(void *mem, size_t newsize, size_t oldsize, void *ctx);
	void *ctx; // passed into all functions
};

struct cu_arena_elem;

struct cu_arena {
	// Required
	struct cu_arena_elem *first;
	size_t default_block_size;
	
	// Nullable (malloc will get used instead)
	const struct cu_allocator *allocator;
};

// generic functions, these default to libc functions when alloc == NULL

void *cu_allocator_alloc(size_t memsize, const struct cu_allocator *alloc);
void cu_allocator_free(void *mem, size_t memsize, const struct cu_allocator *alloc);
void *cu_allocator_realloc(void *mem, size_t newsize, size_t oldsize, const struct cu_allocator *alloc);
// same as freebsd reallocf api, frees the pointer if reallocation fails
inline static void *cu_allocator_reallocf(void *mem, size_t newsize, size_t oldsize, const struct cu_allocator *alloc)
{
	void *result = cu_allocator_realloc(mem, newsize, oldsize, alloc);
	if (result == NULL) {
		cu_allocator_free(mem, oldsize, alloc);
		return NULL;
	}
	return result;
}
void *cu_allocator_reallocarray(void *mem, size_t new_nel, size_t old_nel, size_t elem_size, const struct cu_allocator *alloc);

// This function is inefficient if you're doing `realloc` often.
// If you need to `realloc` an arena is a bad datastructure.
//
// However, if you are just allocating things, this allows for genericity.
void cu_arena_to_allocator(struct cu_allocator *allocator, struct cu_arena *arena);

// Arena functions
inline static void cu_arena_new(struct cu_arena *arena, size_t block_size, const struct cu_allocator *alloc)
{
	arena->first = NULL;
	arena->default_block_size = block_size;
	arena->allocator = alloc;
}
void *cu_arena_alloc(struct cu_arena *arena, size_t item_size);
void cu_arena_free(struct cu_arena *arena);

