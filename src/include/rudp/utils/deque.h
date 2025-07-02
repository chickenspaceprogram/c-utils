#pragma once

#include <rudp/utils/alloc.h>
#include <rudp/utils/vector.h>
#include <stddef.h>
#include <string.h>

#ifndef RUDP_DEQUE_INITSIZE
#define RUDP_DEQUE_INITSIZE 16
#endif

#define RUDP_DEQUE_TYPE(TYPENAME)\
struct {\
	TYPENAME *array;\
	size_t arrsize;\
	size_t firstel;\
	size_t nel;\
}

#define rudp_deque_new(DEQUE, ALLOC)\
	((DEQUE).array = rudp_allocator_allocarray((RUDP_DEQUE_INITSIZE), sizeof(*((DEQUE).array)), (ALLOC)),\
	(((DEQUE).array == NULL) ? -1 : (\
	(DEQUE).arrsize = (RUDP_DEQUE_INITSIZE),\
	(DEQUE).firstel = 0,\
	(DEQUE).nel = 0,\
	0)))

#define rudp_deque_at(DEQUE, INDEX)\
	((DEQUE).array[((DEQUE).firstel + (INDEX)) & ((DEQUE).arrsize - 1)])
#define rudp_deque_front(DEQUE) rudp_deque_at((DEQUE), 0)
#define rudp_deque_back(DEQUE) rudp_deque_at((DEQUE), (DEQUE).nel - 1)
#define rudp_deque_size(DEQUE) ((DEQUE).nel)
#define rudp_deque_capacity(DEQUE) ((DEQUE).arrsize)

#define rudp_deque_reserve(DEQUE, AMOUNT, ALLOC) ({\
	int RUDP_DEQUE_RESERVE_RETVAL_INTERNAL_ = 0;\
	if ((AMOUNT) > (DEQUE).arrsize) {\
		typeof((DEQUE).array) RUDP_DEQUE_RESERVE_NEWARR_INTERNAL_ = rudp_allocator_allocarray(rudp_next_pwr_2(AMOUNT), sizeof(*((DEQUE).array)), (ALLOC));\
		if (RUDP_DEQUE_RESERVE_NEWARR_INTERNAL_ == NULL) {\
			RUDP_DEQUE_RESERVE_RETVAL_INTERNAL_ = -1;\
		}\
		else {\
			memcpy(RUDP_DEQUE_RESERVE_NEWARR_INTERNAL_, (DEQUE).array + (DEQUE).firstel, ((DEQUE).arrsize - (DEQUE).firstel) * sizeof(*((DEQUE).array)));\
			memcpy(RUDP_DEQUE_RESERVE_NEWARR_INTERNAL_ + (DEQUE).arrsize - (DEQUE).firstel, (DEQUE).array, (DEQUE).firstel * sizeof(*((DEQUE).array)));\
			rudp_allocator_freearray((DEQUE).array, (DEQUE).arrsize, sizeof(*((DEQUE).array)), (ALLOC));\
			(DEQUE).array = RUDP_DEQUE_RESERVE_NEWARR_INTERNAL_;\
			(DEQUE).firstel = 0;\
			(DEQUE).arrsize = rudp_next_pwr_2(AMOUNT);\
		}\
	}\
	RUDP_DEQUE_RESERVE_RETVAL_INTERNAL_;\
})

#define rudp_deque_push_back(DEQUE, ELEM, ALLOC)\
	((rudp_deque_reserve(DEQUE, (DEQUE).nel + 1, ALLOC) == 0) ?\
	(++((DEQUE).nel), rudp_deque_at(DEQUE, (DEQUE).nel - 1) = (ELEM),0)\
	: (-1))
#define rudp_deque_pop_back(DEQUE) do {\
	--((DEQUE).nel);\
} while (0)
#define rudp_deque_push_front(DEQUE, ELEM, ALLOC)\
	((rudp_deque_reserve(DEQUE, (DEQUE).nel + 1, ALLOC) == 0) ?\
	(\
		(((DEQUE).firstel == 0) ? ((DEQUE).firstel = (DEQUE).arrsize - 1) : (--(DEQUE).firstel)),\
		((DEQUE).array[(DEQUE).firstel] = (ELEM)),\
		++((DEQUE).nel),0\
	)\
	: (-1))

#define rudp_deque_pop_front(DEQUE) do {\
	(DEQUE).firstel = ((DEQUE).firstel + 1) & ((DEQUE).arrsize - 1);\
	--((DEQUE).nel);\
} while (0)

#define rudp_deque_delete(DEQUE, ALLOC) do {\
	rudp_allocator_freearray((DEQUE).array, (DEQUE).arrsize, sizeof(*((DEQUE).array)), (ALLOC));\
	(DEQUE).array = NULL;\
	(DEQUE).firstel = 0;\
	(DEQUE).nel = 0;\
	(DEQUE).arrsize = 0;\
} while (0)
