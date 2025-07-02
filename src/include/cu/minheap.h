// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <cu/vector.h>

// All names beginning with RUDP_VECTOR or RUDP_MINHEAP and ending with _INTERNAL_ are reserved.

#define RUDP_MINHEAP_TYPE(DATATYPE)\
struct {\
	RUDP_VECTOR_TYPE(DATATYPE) vec;\
}

#define cu_minheap_new(MINHEAP, ALLOC) cu_vector_new((MINHEAP).vec, ALLOC)
#define cu_minheap_delete(MINHEAP, ALLOC) cu_vector_delete((MINHEAP).vec, ALLOC)

#define cu_minheap_top(MINHEAP) cu_vector_at((MINHEAP).vec, 0)

#define cu_minheap_empty(MINHEAP) cu_vector_empty((MINHEAP).vec)
#define cu_minheap_size(MINHEAP) cu_vector_size((MINHEAP).vec)

#define RUDP_MINHEAP_LCHILD_INTERNAL_(CUR_INDEX) ((CUR_INDEX) * 2 + 1)
#define RUDP_MINHEAP_RCHILD_INTERNAL_(CUR_INDEX) ((CUR_INDEX) * 2 + 2)
#define RUDP_MINHEAP_PARENT_INTERNAL_(CUR_INDEX) (((CUR_INDEX) - 1) / 2)
#define RUDP_MINHEAP_GOOD_INVARIANTS_INTERNAL_(MINHEAP, CUR_INDEX, CMPFN) (\
	RUDP_MINHEAP_LCHILD_INTERNAL_(CUR_INDEX) > cu_vector_size((MINHEAP).vec) ||\
	(\
		RUDP_MINHEAP_RCHILD_INTERNAL_(CUR_INDEX) > cu_vector_size((MINHEAP).vec) &&\
		CMPFN(cu_vector_at((MINHEAP).vec, (CUR_INDEX)), cu_vector_at((MINHEAP).vec, RUDP_MINHEAP_LCHILD_INTERNAL_(CUR_INDEX))) <= 0\
	)\
	||\
	(\
		CMPFN(cu_vector_at((MINHEAP).vec, (CUR_INDEX)), cu_vector_at((MINHEAP).vec, RUDP_MINHEAP_LCHILD_INTERNAL_(CUR_INDEX))) <= 0 &&\
		CMPFN(cu_vector_at((MINHEAP).vec, (CUR_INDEX)), cu_vector_at((MINHEAP).vec, RUDP_MINHEAP_RCHILD_INTERNAL_(CUR_INDEX))) <= 0\
		\
	)\
)

#define cu_minheap_push(MINHEAP, VAL, CMPFN, ALLOC) ((cu_vector_push((MINHEAP).vec, VAL, ALLOC) == 0) ? ({\
	size_t RUDP_MINHEAP_PUSH_INDEX_INTERNAL_ = cu_vector_size((MINHEAP).vec) - 1;\
	while (\
		RUDP_MINHEAP_PUSH_INDEX_INTERNAL_ > 0 &&\
		(CMPFN(\
			cu_vector_at((MINHEAP).vec, RUDP_MINHEAP_PARENT_INTERNAL_(RUDP_MINHEAP_PUSH_INDEX_INTERNAL_)),\
			cu_vector_at((MINHEAP).vec, RUDP_MINHEAP_PUSH_INDEX_INTERNAL_)\
	)) > 0) {\
		cu_vector_swap_elem((MINHEAP).vec, RUDP_MINHEAP_PARENT_INTERNAL_(RUDP_MINHEAP_PUSH_INDEX_INTERNAL_), RUDP_MINHEAP_PUSH_INDEX_INTERNAL_);\
		RUDP_MINHEAP_PUSH_INDEX_INTERNAL_ = RUDP_MINHEAP_PARENT_INTERNAL_(RUDP_MINHEAP_PUSH_INDEX_INTERNAL_);\
	}\
	0;\
}) : -1)

#define cu_minheap_pop(MINHEAP, CMPFN) do {\
	cu_vector_at((MINHEAP).vec, 0) = cu_vector_at((MINHEAP).vec, cu_vector_size((MINHEAP).vec) - 1);\
	cu_vector_pop((MINHEAP).vec);\
	size_t RUDP_MINHEAP_POP_INDEX_INTERNAL_ = 0;\
	while (\
		!(RUDP_MINHEAP_GOOD_INVARIANTS_INTERNAL_(MINHEAP, RUDP_MINHEAP_POP_INDEX_INTERNAL_, CMPFN))\
	) {\
		if (cu_vector_size((MINHEAP).vec) < RUDP_MINHEAP_RCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_) || \
			(CMPFN(\
			cu_vector_at((MINHEAP).vec, RUDP_MINHEAP_LCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_)),\
			cu_vector_at((MINHEAP).vec, RUDP_MINHEAP_RCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_))\
			\
		) < 0)) {\
			cu_vector_swap_elem((MINHEAP).vec, RUDP_MINHEAP_LCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_), RUDP_MINHEAP_POP_INDEX_INTERNAL_);\
			RUDP_MINHEAP_POP_INDEX_INTERNAL_ = RUDP_MINHEAP_LCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_);\
		}\
		else {\
			cu_vector_swap_elem((MINHEAP).vec, RUDP_MINHEAP_RCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_), RUDP_MINHEAP_POP_INDEX_INTERNAL_);\
			RUDP_MINHEAP_POP_INDEX_INTERNAL_ = RUDP_MINHEAP_RCHILD_INTERNAL_(RUDP_MINHEAP_POP_INDEX_INTERNAL_);\
		}\
	}\
} while (0)

