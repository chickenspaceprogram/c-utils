// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <c-utils/mem.h>

struct cu_arena_elem {
	struct cu_arena_elem *next;
	size_t block_size; // the block starts at &arena + sizeof(arena)
	size_t amount_filled;
};

void *cu_allocator_alloc(size_t memsize, const struct cu_allocator *alloc)
{
	if (alloc == NULL)
		return malloc(memsize);

	return alloc->alloc(memsize, alloc->ctx);
}

void cu_allocator_free(void *mem, size_t memsize, const struct cu_allocator *alloc)
{
	if (alloc == NULL) {
		free(mem);
		return;
	}
	
	if (alloc->free != NULL)
		alloc->free(mem, memsize, alloc->ctx);
}


void *cu_allocator_realloc(void *mem, size_t newsize, size_t oldsize, const struct cu_allocator *alloc)
{
	if (alloc == NULL)
		return realloc(mem, newsize);
	
	if (alloc->realloc == NULL) {
		void *newbuf = alloc->alloc(newsize, alloc->ctx);
		if (newbuf == NULL) {
			return NULL;
		}
		memcpy(newbuf, mem, oldsize);
		cu_allocator_free(mem, oldsize, alloc);
		return newbuf;
	}
	return alloc->realloc(mem, newsize, oldsize, alloc->ctx);
}

static bool check_mult_overflow(size_t n1, size_t n2)
{
	// should in theory work
	size_t div1 = SIZE_MAX / n1;
	if (div1 < n2) {
		return true;
	}
	return false;
}

void *cu_allocator_reallocarray(void *mem, size_t new_nel, size_t old_nel, size_t elem_size, const struct cu_allocator *alloc)
{
	if (check_mult_overflow(new_nel, elem_size)) {
		return NULL;
	}
	return cu_allocator_realloc(mem, new_nel * elem_size, old_nel * elem_size, alloc);
}

static void *cu_arena_allocator_fn(size_t amount, void *ctx)
{
	struct cu_arena *arena = ctx;
	return cu_arena_alloc(arena, amount);
}

void cu_arena_to_allocator(struct cu_allocator *allocator, struct cu_arena *arena)
{
	allocator->alloc = cu_arena_allocator_fn;
	allocator->free = NULL;
	allocator->realloc = NULL;
	allocator->ctx = arena;
}

static void *get_valid_space(const struct cu_arena_elem *el, size_t index)
{
	return (char *)el + sizeof(const struct cu_arena_elem) + index;
}

static struct cu_arena_elem *new_cu_arena_elem(size_t block_size, const struct cu_allocator *allocator)
{
	size_t alloc_size = block_size + sizeof(struct cu_arena_elem);
	struct cu_arena_elem *new_elem = NULL;
	if (allocator == NULL)
		new_elem = malloc(alloc_size);
	else
		new_elem = cu_allocator_alloc(alloc_size, allocator);

	if (new_elem != NULL) {
		new_elem->next = NULL;
		new_elem->block_size = block_size;
		new_elem->amount_filled = 0;
	}
	return new_elem;
}


void *cu_arena_alloc(struct cu_arena *arena, size_t item_size)
{
	if (arena->first == NULL) {
		size_t block_size = (item_size > arena->default_block_size) ? item_size : arena->default_block_size;
		arena->first = new_cu_arena_elem(block_size, arena->allocator);
		if (arena->first == NULL)
			return NULL;

		arena->first->amount_filled = item_size;
		return get_valid_space(arena->first, 0);
	}
	struct cu_arena_elem *target_elem = arena->first;
	struct cu_arena_elem *last_elem = NULL;
	while (target_elem != NULL && (target_elem->block_size - target_elem->amount_filled) < item_size) {
		last_elem = target_elem;
		target_elem = target_elem->next;
	}
	if (target_elem == NULL) {
		size_t block_size = (item_size > arena->default_block_size) ? item_size : arena->default_block_size;
		last_elem->next = new_cu_arena_elem(block_size, arena->allocator);
		last_elem->next->amount_filled = item_size;
		return get_valid_space(last_elem->next, 0);
	}
	void *space = get_valid_space(target_elem, target_elem->amount_filled);
	target_elem->amount_filled += item_size;
	return space;
}

void cu_arena_free(struct cu_arena *arena)
{
	struct cu_arena_elem *el = arena->first;
	while (el != NULL) {
		struct cu_arena_elem *tmp = el->next;
		cu_allocator_free(el, el->block_size + sizeof(struct cu_arena_elem), arena->allocator);
		el = tmp;
	}
}

