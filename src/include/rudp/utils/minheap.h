// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <rudp/utils/vector.h>

// All names beginning with RUDP_VECTOR or RUDP_MINHEAP and ending with _INTERNAL_ are reserved.

#define RUDP_MINHEAP_TYPE(DATATYPE)\
struct {\
	RUDP_VECTOR_TYPE(DATATYPE) vec;\
}

#define rudp_minheap_new(MINHEAP, ALLOC) rudp_vector_new((MINHEAP).vec, ALLOC)
#define rudp_minheap_delete(MINHEAP, ALLOC) rudp_vector_delete((MINHEAP).vec, ALLOC)

#define rudp_minheap_top(MINHEAP) rudp_vector_at((MINHEAP).vec, 0)

#define rudp_minheap_empty(MINHEAP) rudp_vector_empty((MINHEAP).vec)
#define rudp_minheap_size(MINHEAP) rudp_vector_size((MINHEAP).vec)

#define RUDP_MINHEAP_LCHILD_INTERNAL_(CUR_INDEX) ((CUR_INDEX) * 2 + 1)
#define RUDP_MINHEAP_RCHILD_INTERNAL_(CUR_INDEX) ((CUR_INDEX) * 2 + 2)
#define RUDP_MINHEAP_PARENT_INTERNAL_(CUR_INDEX) (((CUR_INDEX) - 1) / 2)
#define RUDP_MINHEAP_GOOD_INVARIANTS_INTERNAL_(MINHEAP, CUR_INDEX, CMPFN) (\
	RUDP_MINHEAP_LCHILD_INTERNAL_(CUR_INDEX) > rudp_vector_size((MINHEAP).vec) ||\
	(\
		RUDP_MINHEAP_RCHILD_INTERNAL_(CUR_INDEX) > rudp_vector_size((MINHEAP).vec) &&\
		CMPFN(rudp_vector_at((MINHEAP).vec, (CUR_INDEX)), rudp_vector_at((MINHEAP).vec, RUDP_MINHEAP_LCHILD_INTERNAL_(CUR_INDEX))) <= 0\
	)\
	||\
	(\
		CMPFN(rudp_vector_at((MINHEAP).vec, (CUR_INDEX)), rudp_vector_at((MINHEAP).vec, RUDP_MINHEAP_LCHILD_INTERNAL_(CUR_INDEX))) <= 0 &&\
		CMPFN(rudp_vector_at((MINHEAP).vec, (CUR_INDEX)), rudp_vector_at((MINHEAP).vec, RUDP_MINHEAP_RCHILD_INTERNAL_(CUR_INDEX))) <= 0\
		\
	)\
)

#define rudp_minheap_push(MINHEAP, VAL, CMPFN, ALLOC) ((rudp_vector_push((MINHEAP).vec, VAL, ALLOC) == 0) ? ({\
	size_t RUDP_MINHEAP_PUSH_INDEX_INTERNAL_ = rudp_vector_size((MINHEAP).vec) - 1;\
	while (\
		RUDP_MINHEAP_PUSH_INDEX_INTERNAL_ > 0 &&\
		(CMPFN(\
			rudp_vector_at((MINHEAP).vec, RUDP_MINHEAP_PARENT_INTERNAL_(RUDP_MINHEAP_PUSH_INDEX_INTERNAL_)),\
			rudp_vector_at((MINHEAP).vec, RUDP_MINHEAP_PUSH_INDEX_INTERNAL_)\
	)) > 0) {\
		rudp_vector_swap_elem((MINHEAP).vec, RUDP_MINHEAP_PARENT_INTERNAL_(RUDP_MINHEAP_PUSH_INDEX_INTERNAL_), RUDP_MINHEAP_PUSH_INDEX_INTERNAL_);\
		RUDP_MINHEAP_PUSH_INDEX_INTERNAL_ = RUDP_MINHEAP_PARENT_INTERNAL_(RUDP_MINHEAP_PUSH_INDEX_INTERNAL_);\
	}\
	0;\
}) : -1)

#define rudp_minheap_pop(MINHEAP, CMPFN) do {\
	rudp_vector_at((MINHEAP).vec, 0) = rudp_vector_at((MINHEAP).vec, rudp_vector_size((MINHEAP).vec) - 1);\
	rudp_vector_pop((MINHEAP).vec);\
	size_t RUDP_MINHEAP_POP_INDEX_INTERNAL_ = 0;\
	while (\
		!(RUDP_MINHEAP_GOOD_INVARIANTS_INTERNAL_(MINHEAP, RUDP_MINHEAP_POP_INDEX_INTERNAL_, CMPFN))\
	) {\
		if (rudp_vector_size((MINHEAP).vec) < RUDP_MINHEAP_RCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_) || \
			(CMPFN(\
			rudp_vector_at((MINHEAP).vec, RUDP_MINHEAP_LCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_)),\
			rudp_vector_at((MINHEAP).vec, RUDP_MINHEAP_RCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_))\
			\
		) < 0)) {\
			rudp_vector_swap_elem((MINHEAP).vec, RUDP_MINHEAP_LCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_), RUDP_MINHEAP_POP_INDEX_INTERNAL_);\
			RUDP_MINHEAP_POP_INDEX_INTERNAL_ = RUDP_MINHEAP_LCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_);\
		}\
		else {\
			rudp_vector_swap_elem((MINHEAP).vec, RUDP_MINHEAP_RCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_), RUDP_MINHEAP_POP_INDEX_INTERNAL_);\
			RUDP_MINHEAP_POP_INDEX_INTERNAL_ = RUDP_MINHEAP_RCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_);\
		}\
	}\
} while (0)

