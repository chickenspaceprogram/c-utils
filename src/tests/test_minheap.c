// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/minheap.h>
#include <assert.h>

static inline int intcmp(int n1, int n2)
{
	return n1 - n2;
}

static void test_minheap(struct cu_allocator *dummy_test_alloc)
{
	CU_MINHEAP_TYPE(int) intheap;
	cu_minheap_new(intheap, dummy_test_alloc);
	assert(cu_minheap_empty(intheap));
	assert(cu_minheap_size(intheap) == 0);

	int nums[] = {12, 12, 56, 34, -1111};
	for (int i = 0; i < 5; ++i) {
		cu_minheap_push(intheap, nums[i], intcmp, dummy_test_alloc);
		assert(cu_minheap_size(intheap) == i + 1);
		assert(!cu_minheap_empty(intheap));
	}
	assert(cu_minheap_top(intheap) == -1111);
	cu_minheap_pop(intheap, intcmp);
	assert(cu_minheap_top(intheap) == 12);
	cu_minheap_pop(intheap, intcmp);
	assert(cu_minheap_top(intheap) == 12);
	cu_minheap_pop(intheap, intcmp);
	assert(cu_minheap_top(intheap) == 34);
	cu_minheap_pop(intheap, intcmp);
	assert(cu_minheap_top(intheap) == 56);
	cu_minheap_pop(intheap, intcmp);
	assert(cu_minheap_empty(intheap));

	cu_minheap_delete(intheap, dummy_test_alloc);
}

int main(void)
{
	struct cu_allocator alloc = cu_get_dummy_test_alloc();
	test_minheap(&alloc);
	cu_free_dummy_test_alloc(&alloc);
}
