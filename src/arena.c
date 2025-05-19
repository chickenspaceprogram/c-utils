
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
#include <c-utils/arena.h>

struct arena_elem {
	struct arena_elem *next;
	size_t block_size; // the block starts at &arena + sizeof(arena)
	size_t amount_filled;
};

size_t cu_arena_new_size(size_t block_size)
{
	return sizeof(cu_arena) + block_size;
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
static void *setup_new_node(cu_arena *arena, void *buffer, size_t item_size)
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

void cu_arena_new(cu_arena *arena, size_t block_size)
{
	arena->first = NULL;
	arena->default_block_size = block_size;
}

void *cu_arena_alloc(cu_arena *arena, size_t item_size)
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

void cu_arena_free(cu_arena *arena)
{
	for (void *freeable = cu_arena_free_buf(arena); freeable != NULL; freeable = cu_arena_free_buf(arena)) {
		free(freeable);
	}
}

size_t cu_arena_alloc_size(cu_arena *arena, size_t item_size)
{
	if (find_allocatable_elem(arena->first, item_size) != NULL) {
		return 0;
	}
	if (item_size > arena->default_block_size) {
		return item_size + sizeof(struct arena_elem); // item is too big, so it gets its own node
	}
	return arena->default_block_size + sizeof(struct arena_elem); // will have to make a new node
}

void *cu_arena_alloc_buf(cu_arena *arena, size_t item_size, void *buffer)
{
	if (arena->first == NULL) {
		return setup_new_node(arena, buffer, item_size);
	}

	struct arena_elem *allocatable_node = find_allocatable_elem(arena->first, item_size);
	if (allocatable_node == NULL) {

		return setup_new_node(arena, buffer, item_size);
	}

	allocatable_node->amount_filled += item_size;
	return (char *)allocatable_node + sizeof(struct arena_elem) + allocatable_node->amount_filled;
}

void *cu_arena_free_buf(cu_arena *arena)
{
	if (arena->first == NULL) {
		return NULL;
	}
	void *freeable_ptr = arena->first;
	arena->first = arena->first->next;
	return freeable_ptr;
}
