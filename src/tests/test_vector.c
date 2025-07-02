#include <rudp/utils/vector.h>
#include <assert.h>

int main(void)
{
	int nums[] = {1, 11, 111, 1111, 11111, 111111, 23, 45, 67, 89, 10, 11, 12, 13};
	RUDP_VECTOR_TYPE(int) vec;
	rudp_vector_new(vec, dummy_test_alloc);
	for (int i = 0; i < 14; ++i) {
		assert(rudp_vector_push(vec, nums[i], dummy_test_alloc) == 0);
	}
	assert(vec.nel == rudp_vector_size(vec) && rudp_vector_size(vec) == 14);
	assert(vec.bufsize == rudp_vector_capacity(vec) && rudp_vector_capacity(vec) == 16);
	for (int i = 0; i < 14; ++i) {
		assert(rudp_vector_at(vec, i) == nums[i]);
	}
	for (int i = 0; i < 14; ++i) {
		rudp_vector_at(vec, i) = i;
	}
	for (int i = 0; i < 14; ++i) {
		assert(rudp_vector_at(vec, i) == i);
	}
	for (int i = 0; i < 7; ++i) {
		rudp_vector_pop(vec);
	}
	assert(rudp_vector_size(vec) == 7);
	assert(rudp_vector_capacity(vec) == 16);
	rudp_vector_erase(vec);
	assert(rudp_vector_size(vec) == 0);
	assert(rudp_vector_capacity(vec) == 16);
	rudp_vector_delete(vec, dummy_test_alloc);
}
