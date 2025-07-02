// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <rudp/utils/alloc.h>
#include <rudp/utils/hashmap.h>
#include <assert.h>

void *rudp_allocator_alloc(size_t memsize, struct rudp_allocator *alloc)
{
	if (alloc == NULL)
		return malloc(memsize);

	return alloc->alloc(memsize, alloc->ctx);
}

void rudp_allocator_free(void *mem, size_t memsize, struct rudp_allocator *alloc)
{
	if (alloc == NULL) {
		free(mem);
		return;
	}
	
	if (alloc->free != NULL)
		alloc->free(mem, memsize, alloc->ctx);
}


void *rudp_allocator_realloc(void *mem, size_t newsize, size_t oldsize, struct rudp_allocator *alloc)
{
	if (alloc == NULL)
		return realloc(mem, newsize);
	
	if (alloc->realloc == NULL) {
		void *newbuf = alloc->alloc(newsize, alloc->ctx);
		if (newbuf == NULL) {
			return NULL;
		}
		memcpy(newbuf, mem, oldsize);
		rudp_allocator_free(mem, oldsize, alloc);
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

void *rudp_allocator_reallocarray(void *mem, size_t new_nel, size_t old_nel, size_t elem_size, struct rudp_allocator *alloc)
{
	if (check_mult_overflow(new_nel, elem_size)) {
		return NULL;
	}
	return rudp_allocator_realloc(mem, new_nel * elem_size, old_nel * elem_size, alloc);
}

static RUDP_HASHMAP_TYPE(void *, size_t) dummy_map;
bool dummy_map_init = false;

union voidptrunion {
	void *ptr;
	char bytes[sizeof(void *)];
};

static inline size_t ptrhash(void *ptr)
{
	union voidptrunion un = {.ptr = ptr};
	if (sizeof(size_t) == 8) {
		const uint64_t offset_basis = 0xcbf29ce484222325;
		const uint64_t fnv_prime = 0x00000100000001b3;
		uint64_t hash = offset_basis;
		for (size_t i = 0; i < sizeof(void *); ++i) {
			hash ^= un.bytes[i];
			hash *= fnv_prime;
		}
		return hash;
	}
	else if (sizeof(size_t) == 4) {
		const uint32_t offset_basis = 0x811c9dc5;
		const uint32_t fnv_prime = 0x01000193;
		uint32_t hash = offset_basis;
		for (size_t i = 0; i < sizeof(void *); ++i) {
			hash ^= un.bytes[i];
			hash *= fnv_prime;
		}
		return hash;
	}
	else {
		abort();
	}
}

static inline int ptrcmp(void *p1, void *p2)
{
	if (p1 > p2) {
		return 1;
	}
	else if (p1 < p2) {
		return -1;
	}
	return 0;
}

static inline void freedummyhashmap(void)
{
	rudp_hashmap_delete(dummy_map, NULL);
}

static inline void *dummy_test_allocfn(size_t amount, void *ctx)
{
	if (!dummy_map_init) {
		rudp_hashmap_new(dummy_map, NULL);
		dummy_map_init = true;
		atexit(freedummyhashmap);
	}
	void *newptr = malloc(amount);
	if (newptr == NULL)
		return NULL;
	assert(rudp_hashmap_insert(dummy_map, newptr, amount, NULL, ptrhash, ptrcmp) == 0);
	return newptr;
}
static inline void dummy_test_free(void *mem, size_t amount, void *ctx)
{
	size_t *ptr = rudp_hashmap_at(dummy_map, mem, ptrhash, ptrcmp);
	assert(ptr != NULL);
	assert(*ptr == amount);
	free(mem);
}

static struct rudp_allocator dummy_test_alloc_val = {
	.alloc = dummy_test_allocfn,
	.free = dummy_test_free,
	.realloc = NULL,
	.ctx = NULL,
};

struct rudp_allocator *dummy_test_alloc = &dummy_test_alloc_val;
