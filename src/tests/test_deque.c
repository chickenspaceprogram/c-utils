// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#define CU_DEQUE_INITSIZE 4 // allows for testing resize
#include <cu/deque.h>
#undef NDEBUG
#include <assert.h>


static void test_deque_individual(struct cu_allocator *dummy_test_alloc)
{
	CU_DEQUE_TYPE(int) deque;
	assert(cu_deque_new(deque, dummy_test_alloc) == 0);
	assert(cu_deque_capacity(deque) == 4);
	assert(cu_deque_push_back(deque, 11, dummy_test_alloc) == 0);
	assert(cu_deque_front(deque) == 11);
	assert(cu_deque_back(deque) == 11);
	assert(cu_deque_push_back(deque, 22, dummy_test_alloc) == 0);
	assert(cu_deque_push_front(deque, 123, dummy_test_alloc) == 0);
	assert(cu_deque_front(deque) == 123);
	assert(cu_deque_back(deque) == 22);

	assert(cu_deque_push_front(deque, 456, dummy_test_alloc) == 0);
	assert(cu_deque_push_front(deque, 789, dummy_test_alloc) == 0);
	assert(cu_deque_push_back(deque, 333, dummy_test_alloc) == 0);
	assert(cu_deque_front(deque) == 789);
	assert(cu_deque_back(deque) == 333);

	assert(cu_deque_at(deque, 0) == 789);
	assert(cu_deque_at(deque, 1) == 456);
	assert(cu_deque_at(deque, 2) == 123);
	assert(cu_deque_at(deque, 3) == 11);
	assert(cu_deque_at(deque, 4) == 22);
	assert(cu_deque_at(deque, 5) == 333);
	assert(cu_deque_size(deque) == 6);
	assert(cu_deque_capacity(deque) == 8);

	assert(cu_deque_pop_back(deque) == 5);
	assert(cu_deque_pop_back(deque) == 4);
	assert(cu_deque_pop_front(deque) == 3);

	assert(cu_deque_at(deque, 0) == 456);
	assert(cu_deque_at(deque, 1) == 123);
	assert(cu_deque_at(deque, 2) == 11);
	assert(cu_deque_size(deque) == 3);
	assert(cu_deque_capacity(deque) == 8);
	assert(cu_deque_front(deque) == 456);
	assert(cu_deque_back(deque) == 11);


	cu_deque_delete(deque, dummy_test_alloc);
}

void test_deque_multiple(struct cu_allocator *dummy_test_alloc)
{
	CU_DEQUE_TYPE(int) deque;
	assert(cu_deque_new(deque, dummy_test_alloc) == 0);
	int headels[6] = {11, 22, 33, 44, 55, 66};
	int tailels[3] = {123, 456, 789};
	cu_deque_pushall_front(deque, headels, 6, dummy_test_alloc);
	assert(cu_deque_size(deque) == 6);
	assert(cu_deque_pushall_back(deque, tailels, 3, dummy_test_alloc) == 0);
	assert(cu_deque_size(deque) == 9);
	for (size_t i = 0; i < 6; ++i) {
		assert(cu_deque_at(deque, i) == headels[i]);
	}
	for (size_t i = 6; i < 9; ++i) {
		assert(cu_deque_at(deque, i) == tailels[i - 6]);
	}
	assert(cu_deque_pushall_front(deque, tailels, 3, dummy_test_alloc) == 0);
	assert(cu_deque_size(deque) == 12);
	for (size_t i = 0; i < 3; ++i) {
		assert(cu_deque_at(deque, i) == tailels[i]);
	}
	assert(cu_deque_pushall_back(deque, headels, 6, dummy_test_alloc) == 0);
	assert(cu_deque_size(deque) == 18);
	for (size_t i = 12; i < 18; ++i) {
		assert(cu_deque_at(deque, i) == headels[i - 12]);
	}
	assert(cu_deque_popall_back(deque, 12) == 6);
	assert(cu_deque_size(deque) == 6);
	for (size_t i = 0; i < 3; ++i) {
		assert(cu_deque_at(deque, i) == tailels[i]);
	}
	for (size_t i = 3; i < 6; ++i) {
		assert(cu_deque_at(deque, i) == headels[i - 3]);
	}
	assert(cu_deque_popall_front(deque, 3) == 3);
	assert(cu_deque_size(deque) == 3);
	for (size_t i = 0; i < 3; ++i) {
		assert(cu_deque_at(deque, i) == headels[i]);
	}
	cu_deque_delete(deque, dummy_test_alloc);

}

int main(void) 
{
	struct cu_allocator alloc = cu_get_dummy_test_alloc();
	test_deque_individual(&alloc);
	test_deque_multiple(&alloc);
	cu_free_dummy_test_alloc(&alloc);
}
