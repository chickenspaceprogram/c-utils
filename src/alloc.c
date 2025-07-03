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
#undef NDEBUG
#include <assert.h>

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

union voidptrunion {
	void *ptr;
	char bytes[sizeof(void *)];
};

struct hmap {
	CU_HASHMAP_TYPE(void *, size_t) map;
};

static inline size_t ptrhash(void *ptr)
{
	static_assert(1, "sizeof(size_t) on your platform is neither 4 nor 8.");
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
	abort();
}

static inline int ptrcmp(const void *p1, const void *p2)
{
	if (p1 > p2) {
		return 1;
	}
	else if (p1 < p2) {
		return -1;
	}
	return 0;
}

static inline void *dummy_test_allocfn(size_t amount, void *ctx)
{
	void *newptr = malloc(amount);
	if (newptr == NULL)
		return NULL;
	struct hmap *map = ctx;
	int retval = cu_hashmap_insert(map->map, newptr, amount, NULL, ptrhash, ptrcmp);
	assert(retval == 0);
	return newptr;
}
static inline void dummy_test_free(void *mem, size_t amount, void *ctx)
{
	struct hmap *map = ctx;
	size_t *ptr = cu_hashmap_at(map->map, mem, ptrhash, ptrcmp);
	assert(ptr != NULL);
	assert(*ptr == amount);
	free(mem);
}

struct cu_allocator cu_get_dummy_test_alloc(void)
{
	struct hmap *map = malloc(sizeof(struct hmap));

	assert(map != NULL && "Failed to allocate dummy test alloc hashmap");
	int retval = cu_hashmap_new(map->map, NULL);
	assert(retval == 0 && "Failed to initialize dummy test alloc hashmap");
	struct cu_allocator dummy = {
		.alloc = dummy_test_allocfn,
		.free = dummy_test_free,
		.realloc = NULL,
		.ctx = map,
	};
	return dummy;
}
void cu_free_dummy_test_alloc(struct cu_allocator *alloc)
{
	cu_hashmap_delete(*(CU_HASHMAP_TYPE(void *, size_t) *)alloc->ctx, NULL);
	free(alloc->ctx);
}
