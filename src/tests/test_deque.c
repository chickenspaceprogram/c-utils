#define CU_DEQUE_INITSIZE 4 // allows for testing resize
#include <cu/deque.h>
#include <assert.h>


int main(void)
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

	cu_deque_pop_back(deque);
	cu_deque_pop_back(deque);
	cu_deque_pop_front(deque);

	assert(cu_deque_at(deque, 0) == 456);
	assert(cu_deque_at(deque, 1) == 123);
	assert(cu_deque_at(deque, 2) == 11);
	assert(cu_deque_size(deque) == 3);
	assert(cu_deque_capacity(deque) == 8);
	assert(cu_deque_front(deque) == 456);
	assert(cu_deque_back(deque) == 11);


	cu_deque_delete(deque, dummy_test_alloc);
}
