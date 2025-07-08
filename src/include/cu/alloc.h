// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <stddef.h>

// A struct defining a generic allocator.
// You aren't intended to call these function pointers directly from other code;
// instead, check out the cu_allocator_* functions down below.
struct cu_allocator {
	// Required function!
	// If you set alloc == NULL you'll get a segfault.
	//
	// `alloc` allocates `amount` bytes of memory, and returns a pointer to them.
	// If a failure occurs, `alloc` may return NULL.
	//
	// `ctx` is equal to the value of the `ctx` member of this struct,
	// and it is passed unchaned into `alloc`.
	void *(*alloc)(size_t amount, void *ctx); // May return NULL if a failure occurs

	// Nullable functions:
	
	// If free == NULL, free will be a no-op.
	// This might be useful with arena allocators and such.
	//
	// `free` should check for and handle when `NULL` is passed in for `mem`.
	// Doing a no-op in this case is acceptable.
	//
	// `mem` is the memory to be freed.
	// It will either be NULL or be equal to a pointer previously returned by
	// `alloc` or `realloc`.
	//
	// `amount` is the size, in bytes, of the memory originally allocated by
	// `malloc` or `realloc`.
	// This is provided since it makes allocator design simpler.
	// `ctx` is equal to the `ctx` member of this struct; it is passed unchanged.
	void (*free)(void *mem, size_t amount, void *ctx);


	// If realloc == NULL, a manual alloc, memcpy, free will take place.
	void *(*realloc)(void *mem, size_t newsize, size_t oldsize, void *ctx);

	void *ctx; // passed into all functions
};


// Generic functions


// Allocates `memsize` bytes of memory on the heap using `alloc`, and returns
// a pointer to it.
[[nodiscard("Discarding allocated pointer")]]
void *cu_allocator_alloc(size_t memsize, struct cu_allocator *alloc);

// Frees the heap-allocated memory at `mem` with the allocator `alloc`. 
// The size of the memory at `mem` is `memsize`. 
void cu_allocator_free(void *mem, size_t memsize, struct cu_allocator *alloc);

// Extends the size of the memory allocated at `mem`.
// This either happens in the cu_allocator's callbacks
// (if alloc->realloc != NULL), or this function just 
// allocates a new block of memory, memcpy's data over,
// and frees the old one.
[[nodiscard("Discarding allocated pointer")]]
void *cu_allocator_realloc(void *mem, size_t newsize, size_t oldsize, struct cu_allocator *alloc);

// Allocates a new array of `nel` elements, each `elem_size` in size, using
// the allocator `alloc`.
//
// This function is provided because it's convenient.
[[nodiscard("Discarding allocated pointer")]]
inline static void *cu_allocator_allocarray(size_t nel, size_t elemsize, struct cu_allocator *alloc)
{
	return cu_allocator_alloc(nel * elemsize, alloc);
}

// Frees an array of `nel` elements located at `mem` with the allocator `alloc`.
// Each element is `elemsize` bytes in size.
inline static void cu_allocator_freearray(void *mem, size_t nel, size_t elemsize, struct cu_allocator *alloc)
{
	cu_allocator_free(mem, nel * elemsize, alloc);
}

// realloc()'s an array of elements.
//
// This checks for integer overflow, similar to Linux's reallocarray(),
// and fails in that case.
[[nodiscard("Discarding allocated pointer")]]
void *cu_allocator_reallocarray(void *mem, size_t new_nel, size_t old_nel, size_t elem_size, struct cu_allocator *alloc);

// Attempts to call cu_allocator_realloc() on its arguments.
// If cu_allocator_realloc() fails, the pointer gets freed automatically.
//
// This function is present in FreeBSD and it's convenient sometimes.
[[nodiscard("Discarding allocated pointer")]]
inline static void *cu_allocator_reallocf(void *mem, size_t newsize, size_t oldsize, struct cu_allocator *alloc)
{
	void *result = cu_allocator_realloc(mem, newsize, oldsize, alloc);
	if (result == NULL) {
		cu_allocator_free(mem, oldsize, alloc);
		return NULL;
	}
	return result;
}

