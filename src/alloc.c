// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <cu/alloc.h>
#include <cu/hashmap.h>

void *cu_allocator_alloc(size_t memsize, struct cu_allocator *alloc)
{
	if (alloc == NULL)
		return malloc(memsize);

	return alloc->alloc(memsize, alloc->ctx);
}

void cu_allocator_free(void *mem, size_t memsize, struct cu_allocator *alloc)
{
	if (alloc == NULL) {
		free(mem);
		return;
	}
	
	if (alloc->free != NULL)
		alloc->free(mem, memsize, alloc->ctx);
}


void *cu_allocator_realloc(void *mem, size_t newsize, size_t oldsize, struct cu_allocator *alloc)
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

void *cu_allocator_reallocarray(void *mem, size_t new_nel, size_t old_nel, size_t elem_size, struct cu_allocator *alloc)
{
	if (check_mult_overflow(new_nel, elem_size)) {
		return NULL;
	}
	return cu_allocator_realloc(mem, new_nel * elem_size, old_nel * elem_size, alloc);
}

