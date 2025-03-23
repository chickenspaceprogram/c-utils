
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

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>

#include <c-utils/arena.h>


void test_malloc(void) {
    cu_arena arena;
    cu_arena_new(&arena, 2 * sizeof(int)); // yeah it's a small block size ik
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

int main(void) {
    test_malloc();
}
