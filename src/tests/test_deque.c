#define RUDP_DEQUE_INITSIZE 4 // allows for testing resize
#include <rudp/utils/deque.h>
#include <assert.h>


int main(void)
{
	RUDP_DEQUE_TYPE(int) deque;
	assert(rudp_deque_new(deque, dummy_test_alloc) == 0);
	assert(rudp_deque_capacity(deque) == 4);
	assert(rudp_deque_push_back(deque, 11, dummy_test_alloc) == 0);
	assert(rudp_deque_front(deque) == 11);
	assert(rudp_deque_back(deque) == 11);
	assert(rudp_deque_push_back(deque, 22, dummy_test_alloc) == 0);
	assert(rudp_deque_push_front(deque, 123, dummy_test_alloc) == 0);
	assert(rudp_deque_front(deque) == 123);
	assert(rudp_deque_back(deque) == 22);

	assert(rudp_deque_push_front(deque, 456, dummy_test_alloc) == 0);
	assert(rudp_deque_push_front(deque, 789, dummy_test_alloc) == 0);
	assert(rudp_deque_push_back(deque, 333, dummy_test_alloc) == 0);
	assert(rudp_deque_front(deque) == 789);
	assert(rudp_deque_back(deque) == 333);

	assert(rudp_deque_at(deque, 0) == 789);
	assert(rudp_deque_at(deque, 1) == 456);
	assert(rudp_deque_at(deque, 2) == 123);
	assert(rudp_deque_at(deque, 3) == 11);
	assert(rudp_deque_at(deque, 4) == 22);
	assert(rudp_deque_at(deque, 5) == 333);
	assert(rudp_deque_size(deque) == 6);
	assert(rudp_deque_capacity(deque) == 8);

	rudp_deque_pop_back(deque);
	rudp_deque_pop_back(deque);
	rudp_deque_pop_front(deque);

	assert(rudp_deque_at(deque, 0) == 456);
	assert(rudp_deque_at(deque, 1) == 123);
	assert(rudp_deque_at(deque, 2) == 11);
	assert(rudp_deque_size(deque) == 3);
	assert(rudp_deque_capacity(deque) == 8);
	assert(rudp_deque_front(deque) == 456);
	assert(rudp_deque_back(deque) == 11);


	rudp_deque_delete(deque, dummy_test_alloc);
}
