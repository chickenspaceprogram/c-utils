// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <stdint.h>
#include <assert.h>
#include <cu/arena.h>
#include <cu/bitmanip.h>

struct cu_arena_fixed {
	uint8_t *end;
	uint8_t *bump;
	alignas(alignof(max_align_t)) uint8_t start[];
};
struct cu_arena {
	struct cu_arena_elem *first;
	cu_alloc *alloc;
	size_t init_block_size;
	uint8_t *bump;
	alignas(alignof(max_align_t)) uint8_t buf_start[];
};
struct cu_arena_elem {
	struct cu_arena_elem *next;
	uint8_t *buf_end;
	uint8_t *bump;
	alignas(alignof(max_align_t)) uint8_t buf_start[];
};

static_assert(alignof(cu_arena_fixed) <= alignof(max_align_t),
	"Your platform aligns structs weirdly.");
static_assert(alignof(cu_arena) <= alignof(max_align_t),
	"Your platform aligns structs weirdly.");
static_assert(alignof(struct cu_arena_elem) <= alignof(max_align_t),
	"Your platform aligns structs weirdly.");

cu_arena_fixed *cu_arena_fixed_new(size_t arena_size, cu_alloc *alloc)
{
	cu_arena_fixed *arena = cu_malloc(
		sizeof(struct cu_arena) + arena_size, alloc);
	if (arena == NULL)
		return NULL;
	arena->end = arena->start + arena_size;
	arena->bump = arena->end;
	return arena;
}

void *
cu_arena_fixed_aligned_alloc(
	size_t amt,
	size_t align,
	cu_arena_fixed *arena
) {
	assert(cu_bit_ceil(align) == align);
	if (amt > (uintptr_t)arena->bump)
		return NULL; // can't subtract amt from bump ptr
	// ensuring there's enough space
	uintptr_t new_bump = (uintptr_t)(arena->bump - amt);
	new_bump &= ~(align - 1);
	if ((uint8_t *)new_bump < arena->start)
		return NULL; // can't find pointer in arena
	arena->bump = (uint8_t *)new_bump;
	return arena->bump;
}

void cu_arena_fixed_free(cu_arena_fixed *arena, cu_alloc *alloc)
{
	cu_free(arena, arena->end - (uint8_t *)arena, alloc);
}

void cu_arena_fixed_rst(cu_arena_fixed *arena)
{
	arena->bump = arena->end;
}

static void *arena_allocator_alloc(size_t amount, void *ctx)
{
	cu_arena_fixed *arena = ctx;
	return cu_arena_fixed_alloc(amount, arena);
}

void cu_arena_fixed_cast(cu_alloc *alloc, cu_arena_fixed *arena)
{
	alloc->alloc = arena_allocator_alloc;
	alloc->free = NULL;
	alloc->realloc = NULL;
	alloc->ctx = arena;
}

cu_arena *cu_arena_new(size_t block_size, cu_alloc *alloc)
{
	size_t init_block_size = cu_bit_ceil(block_size);
	cu_arena *arena = cu_malloc(sizeof(cu_arena) + init_block_size, alloc);
	if (arena == NULL)
		return NULL;
	arena->first = NULL;
	arena->alloc = alloc;
	arena->init_block_size = init_block_size;
	arena->bump = arena->buf_start + init_block_size;
	return arena;
}

static inline size_t max_reqd_size(size_t amt, size_t align)
{
	if (align <= alignof(max_align_t))
		return amt;
	return amt + align - alignof(max_align_t);
}
void *cu_arena_aligned_alloc(size_t amt, size_t align, cu_arena *arena)
{
	if ((uintptr_t)arena->bump >= amt) {
		uintptr_t new_bump = (uintptr_t)(arena->bump - amt);
		new_bump &= ~(align - 1);
		if ((uint8_t *)new_bump >= arena->buf_start) {
			arena->bump = (uint8_t *)new_bump;
			return arena->bump;
		}
	}
	// else: not enough space! check linked list

	size_t nbytes = arena->init_block_size;
	for (struct cu_arena_elem *elem = arena->first;
		elem != NULL;
		elem = elem->next
	) {
		nbytes += elem->buf_end - elem->buf_start;
		if ((uintptr_t)elem->bump < amt)
			// in the unlikely case we can't subtract amt from the
			// bump ptr, there's no space, continue
			continue;

		uintptr_t new_bump = (uintptr_t)(elem->bump - amt);
		new_bump &= ~(align - 1);
		if ((uint8_t *)new_bump >= elem->buf_start) {
			elem->bump = (uint8_t *)new_bump;
			return arena->bump; // found some space!
		}
	}
	// no place in linked list! get a new node
	
	
	assert(cu_bit_ceil(nbytes) == nbytes
		&& "nbytes should be a power of two");
	
	size_t alloc_sz =
		cu_bit_ceil(nbytes + max_reqd_size(amt, align)) - nbytes;
	struct cu_arena_elem *new_first = cu_malloc(
		sizeof(struct cu_arena_elem) + alloc_sz, arena->alloc);
	if (new_first == NULL)
		return NULL;

	new_first->next = arena->first;
	new_first->buf_end = new_first->buf_start + alloc_sz;
	new_first->bump = new_first->buf_end;
	arena->first = new_first;
	// will definitely have enough space
	
	assert((uintptr_t)new_first->bump >= amt
		&& "allocated block not large enough");
	new_first->bump -= amt;
	new_first->bump =
		(uint8_t *)((uintptr_t)new_first->bump & ~(align - 1));
	assert(new_first->bump >= new_first->buf_start
		&& "allocated block not large enough");
	return new_first->bump;
}
void cu_arena_free(cu_arena *arena)
{
	struct cu_arena_elem *elem = arena->first;
	while (elem != NULL) {
		struct cu_arena_elem *tmp = elem;
		elem = elem->next;
		cu_free(tmp, tmp->buf_end - (uint8_t *)tmp, arena->alloc);
	}
	cu_free(
		arena,
		sizeof(cu_arena) + arena->init_block_size,
		arena->alloc
	);
}
void cu_arena_rst(cu_arena *arena)
{
	arena->bump = arena->buf_start + arena->init_block_size;
	for (struct cu_arena_elem *elem = arena->first; elem != NULL;
		elem = elem->next
	) {
		elem->bump = elem->buf_end;
	}
}

static void *arena_alloc(size_t amount, void *ctx)
{
	cu_arena *arena = ctx;
	return cu_arena_alloc(amount, arena);
}

void cu_arena_cast(cu_alloc *alloc, cu_arena *arena)
{
	alloc->alloc = arena_alloc;
	alloc->free = NULL;
	alloc->realloc = NULL;
	alloc->ctx = arena;
}

