// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/arena.h>
#include <stdint.h>
#include <assert.h>

#define IS_PWR_2(V) ((V) && !((V) & ((V) - 1)))

struct cu_arena_fixed {
	uint8_t *end;
	uint8_t *bump;
	alignas(alignof(max_align_t)) uint8_t start[];
};
struct cu_arena {
	struct cu_arena_elem *first;
	cu_alloc *alloc;
	size_t default_block_size;
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
	assert(IS_PWR_2(align));
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
	if (block_size < alignof(max_align_t))
		block_size = alignof(max_align_t);
	cu_arena *arena = cu_malloc(sizeof(cu_arena) + block_size, alloc);
	if (arena == NULL)
		return NULL;
	arena->first = NULL;
	arena->alloc = alloc;
	arena->default_block_size = block_size;
	arena->bump = arena->buf_start + block_size;
	return arena;
}

static size_t max_reqd_size(size_t amt, size_t align)
{
	if (align <= alignof(max_align_t))
		return amt;
	return amt + align - alignof(max_align_t);
}
void *cu_arena_aligned_alloc(size_t amt, size_t align, cu_arena *arena)
{
	size_t reqd_size = max_reqd_size(amt, align);
	if (reqd_size > arena->default_block_size) {
		struct cu_arena_elem *new_block = cu_malloc(
			reqd_size + sizeof(struct cu_arena_elem),
			arena->alloc
		);
		if (new_block == NULL)
			return NULL;
		new_block->next = arena->first;
		new_block->buf_end = arena->buf_start + reqd_size;
		new_block->bump = new_block->buf_end;

		// block definitely big enough
		assert((uintptr_t)new_block->bump > amt
			&& "allocated block not large enough");
		new_block->bump -= amt;
		new_block->bump =
			(uint8_t *)((uintptr_t)new_block->bump & ~(align - 1));
		assert(new_block->bump >= new_block->buf_start
			&& "allocated block not large enough");
		arena->first = new_block;
		return new_block->bump;
	}
	if ((uintptr_t)arena->bump >= amt) {
		uintptr_t new_bump = (uintptr_t)(arena->bump - amt);
		new_bump &= ~(align - 1);
		if ((uint8_t *)new_bump >= arena->buf_start) {
			arena->bump = (uint8_t *)new_bump;
			return arena->bump;
		}
		// not enough space, oh well, check the linked list
	}

	struct cu_arena_elem *elem = arena->first;
	while (elem != NULL) {
		if ((uintptr_t)elem->bump < amt)
			// in the unlikely case we can't subtract amt from the
			// bump ptr, continue
			continue;
		uintptr_t new_bump = (uintptr_t)(elem->bump - amt);
		new_bump &= ~(align - 1);
		if ((uint8_t *)new_bump >= elem->buf_start) {
			elem->bump = (uint8_t *)new_bump;
			return arena->bump; // found some space!
		}
		elem = elem->next;
	}
	struct cu_arena_elem *new_first = cu_malloc(
		sizeof(struct cu_arena_elem) + arena->default_block_size,
		arena->alloc
	);
	if (new_first == NULL)
		return NULL;
	new_first->next = arena->first;
	new_first->buf_end = new_first->buf_start + arena->default_block_size;
	new_first->bump = new_first->buf_end;
	// will definitely have enough space
	assert((uintptr_t)new_first->bump >= amt
		&& "allocated block not large enough");
	new_first->bump -= amt;
	new_first->bump =
		(uint8_t *)((uintptr_t)new_first->bump & ~(align - 1));
	assert(new_first->bump >= new_first->buf_start
		&& "allocated block not large enough");
	arena->first = new_first;
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
		sizeof(cu_arena) + arena->default_block_size,
		arena->alloc
	);
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

