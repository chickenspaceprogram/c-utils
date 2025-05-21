// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <assert.h>

#include <c-utils/mem.h>
#include <stdlib.h>


void test_malloc(struct cu_allocator *alloc) {
    struct cu_arena arena;
    cu_arena_new(&arena, 2 * sizeof(int), alloc); // yeah it's a small block size ik
    int *int1 = cu_arena_alloc(&arena, sizeof(int));
    *int1 = 12;
    int *int2 = cu_arena_alloc(&arena, sizeof(int));
    *int2  = 34;
    int *int3 = cu_arena_alloc(&arena, sizeof(int));
    *int3  = 56;
    int *int4 = cu_arena_alloc(&arena, sizeof(int));
    *int4 = 78;

    int *massive_object = cu_arena_alloc(&arena, sizeof(int) * 20);
    for (int i = 0; i < 20; ++i) {
        massive_object[i] = 100 * i;
    }

    assert(*int1 == 12);
    assert(*int2 == 34);
    assert(*int3 == 56);
    assert(*int4 == 78);

    for (int i = 0; i < 20; ++i) {
        assert(massive_object[i] == 100 * i);
    }

    cu_arena_free(&arena);

}

static void *dummy_malloc(size_t amt, void *ctx)
{
	return malloc(amt);
}

static void dummy_free(void *mem, size_t amt, void *ctx)
{
	free(mem);
}

static void *dummy_realloc(void *mem, size_t newsize, size_t oldsize, void *ctx)
{
	return realloc(mem, newsize);
}

struct cu_allocator test_alloc1 = {
	.alloc = dummy_malloc,
	.free = dummy_free,
};

struct cu_allocator test_alloc2 = {
	.alloc = dummy_malloc,
	.free = dummy_free,
	.realloc = dummy_realloc,
};

int main(void) {
    test_malloc(NULL);
    test_malloc(&test_alloc1);
    test_malloc(&test_alloc2);
}
