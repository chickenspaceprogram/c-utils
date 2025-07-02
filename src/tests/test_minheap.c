#include <rudp/utils/minheap.h>
#include <assert.h>

static inline int intcmp(int n1, int n2)
{
	return n1 - n2;
}

int main(void)
{
	RUDP_MINHEAP_TYPE(int) intheap;
	rudp_minheap_new(intheap, dummy_test_alloc);
	assert(rudp_minheap_empty(intheap));
	assert(rudp_minheap_size(intheap) == 0);

	int nums[] = {12, 12, 56, 34, -1111};
	for (int i = 0; i < 5; ++i) {
		rudp_minheap_push(intheap, nums[i], intcmp, dummy_test_alloc);
		assert(rudp_minheap_size(intheap) == i + 1);
		assert(!rudp_minheap_empty(intheap));
	}
	assert(rudp_minheap_top(intheap) == -1111);
	rudp_minheap_pop(intheap, intcmp);
	assert(rudp_minheap_top(intheap) == 12);
	rudp_minheap_pop(intheap, intcmp);
	assert(rudp_minheap_top(intheap) == 12);
	rudp_minheap_pop(intheap, intcmp);
	assert(rudp_minheap_top(intheap) == 34);
	rudp_minheap_pop(intheap, intcmp);
	assert(rudp_minheap_top(intheap) == 56);
	rudp_minheap_pop(intheap, intcmp);
	assert(rudp_minheap_empty(intheap));

	rudp_minheap_delete(intheap, dummy_test_alloc);
}
