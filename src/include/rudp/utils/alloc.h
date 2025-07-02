// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <stddef.h>

struct rudp_allocator {
	// Required, setting alloc = NULL is undefined behavior
	void *(*alloc)(size_t amount, void *ctx); // May return NULL if a failure occurs

	// Nullable
	
	// If free == NULL, free will be a no-op.
	void (*free)(void *mem, size_t amount, void *ctx); // Should no-op on NULL `mem`
	// If realloc == NULL, a manual alloc, memcpy, free will take place.
	void *(*realloc)(void *mem, size_t newsize, size_t oldsize, void *ctx);
	void *ctx; // passed into all functions
};

extern struct rudp_allocator *dummy_test_alloc; // used to test that mem sizes are passed to realloc and free properly
// this is only safe to use in a single-threaded context!

// generic functions, these default to libc functions when alloc == NULL

void *rudp_allocator_alloc(size_t memsize, struct rudp_allocator *alloc);
void rudp_allocator_free(void *mem, size_t memsize, struct rudp_allocator *alloc);
void *rudp_allocator_realloc(void *mem, size_t newsize, size_t oldsize, struct rudp_allocator *alloc);

inline static void *rudp_allocator_allocarray(size_t nel, size_t elemsize, struct rudp_allocator *alloc)
{
	return rudp_allocator_alloc(nel * elemsize, alloc);
}

inline static void rudp_allocator_freearray(void *mem, size_t nel, size_t elemsize, struct rudp_allocator *alloc)
{
	rudp_allocator_free(mem, nel * elemsize, alloc);
}

// same as freebsd reallocf api, frees the pointer if reallocation fails
inline static void *rudp_allocator_reallocf(void *mem, size_t newsize, size_t oldsize, struct rudp_allocator *alloc)
{
	void *result = rudp_allocator_realloc(mem, newsize, oldsize, alloc);
	if (result == NULL) {
		rudp_allocator_free(mem, oldsize, alloc);
		return NULL;
	}
	return result;
}
void *rudp_allocator_reallocarray(void *mem, size_t new_nel, size_t old_nel, size_t elem_size, struct rudp_allocator *alloc);

