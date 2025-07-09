// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/vector.h>
#include <cu/allocators.h>
#undef NDEBUG
#include <assert.h>

static void single_push_pop_tests(struct cu_allocator *dummy_test_alloc)
{
	int nums[] = {1, 11, 111, 1111, 11111, 111111, 23, 45, 67, 89, 10, 11, 12, 13};
	CU_VECTOR_TYPE(int) vec;
	cu_vector_new(vec, dummy_test_alloc);
	for (int i = 0; i < 14; ++i) {
		assert(cu_vector_push(vec, nums[i], dummy_test_alloc) == 0);
	}
	assert(vec.nel == cu_vector_size(vec) && cu_vector_size(vec) == 14);
	assert(vec.bufsize == cu_vector_capacity(vec) && cu_vector_capacity(vec) == 16);
	for (int i = 0; i < 14; ++i) {
		assert(cu_vector_at(vec, i) == nums[i]);
	}
	for (int i = 0; i < 14; ++i) {
		cu_vector_at(vec, i) = i;
	}
	for (int i = 0; i < 14; ++i) {
		assert(cu_vector_at(vec, i) == i);
	}
	for (int i = 0; i < 7; ++i) {
		assert(cu_vector_pop(vec) == 14 - i - 1);
	}
	assert(cu_vector_size(vec) == 7);
	assert(cu_vector_capacity(vec) == 16);
	cu_vector_erase(vec);
	assert(cu_vector_size(vec) == 0);
	assert(cu_vector_capacity(vec) == 16);
	cu_vector_delete(vec, dummy_test_alloc);
}

static void multi_push_pop_tests(struct cu_allocator *dummy_test_alloc)
{
	int nums[] = {1, 11, 111, 1111, 11111, 111111, 23, 45, 67, 89, 10, 11, 12, 13};
	CU_VECTOR_TYPE(int) vec;
	cu_vector_new(vec, dummy_test_alloc);
	assert(cu_vector_pushall(vec, nums, 14, dummy_test_alloc) == 0);
	assert(cu_vector_size(vec) == 14);
	assert(cu_vector_capacity(vec) == 16);
	for (size_t i = 0; i < 14; ++i) {
		assert(cu_vector_at(vec, i) == nums[i]);
	}
	assert(cu_vector_popall(vec, 4) == 10);
	assert(cu_vector_size(vec) == 10);
	assert(cu_vector_popall(vec, 123) == 0);
	cu_vector_delete(vec, dummy_test_alloc);
}

int main(void) {
	struct cu_allocator alloc = cu_get_dummy_test_alloc();
	single_push_pop_tests(&alloc);
	multi_push_pop_tests(&alloc);
	cu_free_dummy_test_alloc(&alloc);
}
