// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <cu/alloc.h>
#include <cu/bitmanip.h>

void *cu_malloc(size_t memsize, cu_alloc *alloc)
{
	if (alloc == NULL)
		return malloc(memsize);

	return alloc->alloc(memsize, alloc->ctx);
}

void cu_free(void *mem, size_t memsize, cu_alloc *alloc)
{
	if (alloc == NULL) {
		free(mem);
		return;
	}
	
	if (alloc->free != NULL)
		alloc->free(mem, memsize, alloc->ctx);
}


void *cu_realloc(void *mem, size_t newsize, size_t oldsize, cu_alloc *alloc)
{
	if (alloc == NULL)
		return realloc(mem, newsize);
	
	if (alloc->realloc == NULL) {
		void *newbuf = alloc->alloc(newsize, alloc->ctx);
		if (newbuf == NULL) {
			return NULL;
		}
		memcpy(newbuf, mem, oldsize);
		cu_free(mem, oldsize, alloc);
		return newbuf;
	}
	return alloc->realloc(mem, newsize, oldsize, alloc->ctx);
}

int cu_try_realloc(void **mem, size_t newsize, size_t oldsize, cu_alloc *alloc)
{
	void *newptr = cu_realloc(*mem, newsize, oldsize, alloc);
	if (newptr == NULL)
		return -1;
	*mem = newptr;
	return 0;
}

void *cu_reallocarray(void *mem, size_t new_nel, size_t old_nel, size_t elem_size, cu_alloc *alloc)
{
	size_t result = 0;
	if (cu_ckd_mul(&result, new_nel, old_nel)) {
		return NULL;
	}
	return cu_realloc(mem, new_nel * elem_size, old_nel * elem_size, alloc);
}

int cu_try_reallocarray(void **mem, size_t new_nel, size_t old_nel, size_t elem_size, cu_alloc *alloc)
{
	void *newptr = cu_reallocarray(*mem, new_nel, old_nel, elem_size, alloc);
	if (newptr == NULL)
		return -1;
	*mem = newptr;
	return 0;
}
