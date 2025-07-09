// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/arena.h>
#include <cu/allocators.h>
#include <assert.h>

#define BLOCK_SIZE (alignof(max_align_t) * 4)

static void test_cu_arena_nonaligned(struct cu_allocator *alloc)
{
	struct cu_arena *arena = cu_arena_new(BLOCK_SIZE, alloc);
	size_t num_allocatable = BLOCK_SIZE / alignof(max_align_t);
	for (size_t i = 0; i < num_allocatable; ++i) {
		volatile int *foo = cu_arena_alloc(sizeof(int), arena);
		assert(foo != NULL);
		*foo = 1234;
	}
	int *asdf = cu_arena_alloc(sizeof(int), arena);
	assert(asdf == NULL);
	cu_arena_free(arena, alloc);
}

static void test_cu_arena_aligned(struct cu_allocator *alloc)
{
	struct cu_arena *arena = cu_arena_new(BLOCK_SIZE, alloc);
	for (size_t i = 0; i < BLOCK_SIZE / sizeof(int); ++i) {
		volatile int *foo = cu_arena_aligned_alloc(sizeof(int), alignof(int), arena);
		assert(foo != NULL);
		*foo = 1234;
	}
	int *asdf = cu_arena_aligned_alloc(sizeof(int), alignof(int), arena);
	assert(asdf == NULL);
	cu_arena_free(arena, alloc);
}


static void test_cu_arena_dyn_nonaligned(struct cu_allocator *alloc)
{
	struct cu_arena_dyn *arena = cu_arena_dyn_new(BLOCK_SIZE, alloc);
	size_t num_allocatable = BLOCK_SIZE / alignof(max_align_t);
	for (size_t i = 0; i < num_allocatable; ++i) {
		volatile int *foo = cu_arena_dyn_alloc(sizeof(int), arena);
		assert(foo != NULL);
		*foo = 1234;
	}
	cu_arena_dyn_free(arena);
}

static void test_cu_arena_dyn_aligned(struct cu_allocator *alloc)
{
	struct cu_arena_dyn *arena = cu_arena_dyn_new(BLOCK_SIZE, alloc);
	for (size_t i = 0; i < BLOCK_SIZE / sizeof(int); ++i) {
		volatile int *foo = cu_arena_dyn_aligned_alloc(sizeof(int), alignof(int), arena);
		assert(foo != NULL);
		*foo = 1234;
	}
	cu_arena_dyn_free(arena);
}

int main(void) {
	struct cu_allocator alloc = cu_get_dummy_test_alloc();
	test_cu_arena_nonaligned(&alloc);
	assert(sizeof(int) == alignof(int) && "int is weird on your platform");
	test_cu_arena_aligned(&alloc);
	test_cu_arena_dyn_nonaligned(&alloc);
	test_cu_arena_dyn_aligned(&alloc);
	cu_free_dummy_test_alloc(&alloc);
}
