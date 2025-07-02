#include <cu/minheap.h>
#include <assert.h>

static inline int intcmp(int n1, int n2)
{
	return n1 - n2;
}

int main(void)
{
	RUDP_MINHEAP_TYPE(int) intheap;
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
