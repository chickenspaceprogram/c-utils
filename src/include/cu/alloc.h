// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

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

extern struct cu_allocator *dummy_test_alloc; // used to test that mem sizes are passed to realloc and free properly
// this is only safe to use in a single-threaded context!

// generic functions, these default to libc functions when alloc == NULL

void *cu_allocator_alloc(size_t memsize, struct cu_allocator *alloc);
void cu_allocator_free(void *mem, size_t memsize, struct cu_allocator *alloc);
void *cu_allocator_realloc(void *mem, size_t newsize, size_t oldsize, struct cu_allocator *alloc);

inline static void *cu_allocator_allocarray(size_t nel, size_t elemsize, struct cu_allocator *alloc)
{
	return cu_allocator_alloc(nel * elemsize, alloc);
}

inline static void cu_allocator_freearray(void *mem, size_t nel, size_t elemsize, struct cu_allocator *alloc)
{
	cu_allocator_free(mem, nel * elemsize, alloc);
}

// same as freebsd reallocf api, frees the pointer if reallocation fails
inline static void *cu_allocator_reallocf(void *mem, size_t newsize, size_t oldsize, struct cu_allocator *alloc)
{
	void *result = cu_allocator_realloc(mem, newsize, oldsize, alloc);
	if (result == NULL) {
		cu_allocator_free(mem, oldsize, alloc);
		return NULL;
	}
	return result;
}
void *cu_allocator_reallocarray(void *mem, size_t new_nel, size_t old_nel, size_t elem_size, struct cu_allocator *alloc);

