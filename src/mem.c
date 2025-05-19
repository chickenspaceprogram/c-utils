
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

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <c-utils/mem.h>

struct arena_elem {
	struct arena_elem *next;
	size_t block_size; // the block starts at &arena + sizeof(arena)
	size_t amount_filled;
};

void *cu_allocator_realloc(void *mem, size_t newsize, size_t oldsize, const struct cu_allocator *alloc)
{
	if (alloc->realloc == NULL) {
		void *newbuf = alloc->alloc(newsize, alloc->ctx);
		if (newbuf == NULL) {
			return NULL;
		}
		memcpy(newbuf, mem, oldsize);
		alloc->free(mem, oldsize, alloc->ctx);
	}
	return alloc->realloc(mem, newsize, oldsize, alloc->ctx);
}

bool check_mult_overflow(size_t n1, size_t n2)
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



size_t cu_arena_new_size(size_t block_size)
{
	return sizeof(struct cu_arena) + block_size;
}

static struct arena_elem *find_allocatable_elem(struct arena_elem *first, size_t item_size)
{
	for (struct arena_elem *elem = first; elem != NULL; elem = elem->next) {
		if (item_size < (elem->block_size - elem->amount_filled)) {
			return elem;
		}
	}
	return NULL;
}

// returns a pointer to the data space
static void *setup_new_node(struct cu_arena *arena, void *buffer, size_t item_size)
{
		size_t alloc_size = (item_size > arena->default_block_size) ? item_size : arena->default_block_size; 
		struct arena_elem new_node = {
			.next = arena->first,
			.block_size = alloc_size,
			.amount_filled = item_size,
		};
		memcpy(buffer, &new_node, sizeof(struct arena_elem));
		arena->first = buffer;
		return (char *)buffer + sizeof(struct arena_elem);
	
}

void cu_arena_new(struct cu_arena *arena, size_t block_size)
{
	arena->first = NULL;
	arena->default_block_size = block_size;
}

void *cu_arena_alloc(struct cu_arena *arena, size_t item_size)
{
	size_t alloc_size = cu_arena_alloc_size(arena, item_size);
	void *buffer = NULL;
	if (alloc_size != 0) {
		buffer = malloc(alloc_size);
		if (buffer == NULL) {
			return NULL;
		}
	}
	return cu_arena_alloc_buf(arena, item_size, buffer);
}

void cu_arena_free(struct cu_arena *arena)
{
	for (void *freeable = cu_arena_free_buf(arena); freeable != NULL; freeable = cu_arena_free_buf(arena)) {
		free(freeable);
	}
}
