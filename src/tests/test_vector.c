#include <cu/vector.h>
#include <assert.h>

int main(void)
{
	int nums[] = {1, 11, 111, 1111, 11111, 111111, 23, 45, 67, 89, 10, 11, 12, 13};
	RUDP_VECTOR_TYPE(int) vec;
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
		cu_vector_pop(vec);
	}
	assert(cu_vector_size(vec) == 7);
	assert(cu_vector_capacity(vec) == 16);
	cu_vector_erase(vec);
	assert(cu_vector_size(vec) == 0);
	assert(cu_vector_capacity(vec) == 16);
	cu_vector_delete(vec, dummy_test_alloc);
}
