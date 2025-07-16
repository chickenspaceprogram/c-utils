// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cu/alloc.h>
#include <cu/vector.h>
#include <stddef.h>
#include <string.h>

#ifndef CU_DEQUE_INITSIZE
#define CU_DEQUE_INITSIZE 16
#endif

#define CU_DEQUE_TYPE(TYPENAME)\
struct {\
	TYPENAME *array;\
	size_t arrsize;\
	size_t firstel;\
	size_t nel;\
}

#define cu_deque_new(DEQUE, ALLOC) (\
	(DEQUE).array = cu_allocator_allocarray((CU_DEQUE_INITSIZE), sizeof(*((DEQUE).array)), (ALLOC)),\
	(((DEQUE).array == NULL) ? -1 : (\
	(DEQUE).arrsize = (CU_DEQUE_INITSIZE),\
	(DEQUE).firstel = 0,\
	(DEQUE).nel = 0,\
	0))\
)

#define cu_deque_at(DEQUE, INDEX)\
	((DEQUE).array[((DEQUE).firstel + (INDEX)) & ((DEQUE).arrsize - 1)])
#define cu_deque_front(DEQUE) cu_deque_at((DEQUE), 0)
#define cu_deque_back(DEQUE) cu_deque_at((DEQUE), (DEQUE).nel - 1)
#define cu_deque_size(DEQUE) ((DEQUE).nel)
#define cu_deque_capacity(DEQUE) ((DEQUE).arrsize)

// awful function
static inline int 
CU_DEQUE_GET_NEWARR_INTERNAL_(
	void **arr,
	size_t reserveamt,
	size_t arrsize,
	size_t elsize,
	size_t firstel,
	size_t nel,
	struct cu_allocator *alloc
) {
	char *newarr = cu_allocator_allocarray(cu_next_pwr_2(reserveamt), elsize, alloc);
	char *oldarr = *arr;
	if (newarr == NULL)
		return -1;
	if (firstel + nel > arrsize) {
		memcpy(newarr, oldarr + firstel * elsize, (arrsize - firstel) * elsize);
		memcpy(newarr + (arrsize - firstel) * elsize, oldarr, (firstel + nel - arrsize) * elsize);
	}
	else {
		memcpy(newarr, oldarr + firstel * elsize, nel * elsize);
	}

	cu_allocator_freearray(oldarr, arrsize, elsize, alloc);
	*arr = newarr;
	return 0;
}

#define cu_deque_reserve(DEQUE, AMOUNT, ALLOC) (\
	((AMOUNT) > (DEQUE).arrsize) ? (\
		(CU_DEQUE_GET_NEWARR_INTERNAL_((void **)&((DEQUE).array), AMOUNT, (DEQUE).arrsize, sizeof(*((DEQUE).array)), (DEQUE).firstel, (DEQUE).nel, (ALLOC)) == 0) ? (\
			(DEQUE).arrsize = cu_next_pwr_2(AMOUNT),\
			(DEQUE).firstel = 0,0\
		) : (-1)\
	) : (0)\
)


#define cu_deque_push_back(DEQUE, ELEM, ALLOC)\
	((cu_deque_reserve(DEQUE, (DEQUE).nel + 1, ALLOC) == 0) ?\
	(++((DEQUE).nel), cu_deque_at(DEQUE, (DEQUE).nel - 1) = (ELEM),0)\
	: (-1))

#define cu_deque_pushall_back(DEQUE, ELEMPTR, NEL, ALLOC)\
_Generic((ELEMPTR), CU_TYPEOF((DEQUE).array):\
((cu_deque_reserve(DEQUE, (DEQUE).nel + (NEL), ALLOC) == 0) ? (\
	(((DEQUE).nel + (DEQUE).firstel) & ((DEQUE).arrsize - 1)) + (NEL) > (DEQUE).arrsize ? (\
		memcpy(\
			&cu_deque_at((DEQUE), (DEQUE).nel),\
			(ELEMPTR), (/* number of elements till end*/\
				((DEQUE).arrsize - \
				((DEQUE).nel + ((DEQUE).firstel) & ((DEQUE).arrsize - 1)))\
			) * sizeof(*((DEQUE).array))),\
		memcpy((DEQUE).array, (ELEMPTR) + (/* number of elements till end*/\
			((DEQUE).arrsize - \
			((DEQUE).nel + ((DEQUE).firstel) & ((DEQUE).arrsize - 1)))\
		), ((NEL) - (/* number of elements till end*/\
			((DEQUE).arrsize - \
			((DEQUE).nel + ((DEQUE).firstel) & ((DEQUE).arrsize - 1)))\
		)) * sizeof(*((DEQUE).array))),\
		\
		(DEQUE).nel += (NEL),0\
	) : (\
		memcpy(&cu_deque_at((DEQUE), (DEQUE).nel), (ELEMPTR), (NEL) * sizeof(*((DEQUE).array))),\
		(DEQUE).nel += (NEL),0\
	)\
) : (-1)))

#define cu_deque_pop_back(DEQUE)\
	((DEQUE).nel == 0 ? ((DEQUE).firstel = 0) : (--(DEQUE).nel))

#define cu_deque_popall_back(DEQUE, NEL)\
	((DEQUE).nel < (NEL) ? ((DEQUE).nel = 0,(DEQUE).firstel = 0,0) : ((DEQUE).nel -= (NEL)))

#define cu_deque_push_front(DEQUE, ELEM, ALLOC)\
	((cu_deque_reserve((DEQUE), (DEQUE).nel + 1, (ALLOC)) == 0) ?\
	(\
		(((DEQUE).firstel == 0) ? ((DEQUE).firstel = (DEQUE).arrsize - 1) : (--(DEQUE).firstel)),\
		((DEQUE).array[(DEQUE).firstel] = (ELEM)),\
		++((DEQUE).nel),0\
	)\
	: (-1))

#define cu_deque_pushall_front(DEQUE, ELEMPTR, NEL, ALLOC)\
_Generic((ELEMPTR), CU_TYPEOF((DEQUE).array):\
	((cu_deque_reserve((DEQUE), (DEQUE).nel + (NEL), (ALLOC)) == 0) ? (\
		(DEQUE).firstel == 0 ? (\
			memcpy((DEQUE).array + (DEQUE).arrsize - (NEL), (ELEMPTR), (NEL) * sizeof(*((DEQUE).array)))\
		) : (\
			((DEQUE).firstel > (NEL) ? (\
				memcpy((DEQUE).array + (DEQUE).firstel - (NEL), (ELEMPTR), (NEL) * sizeof(*((DEQUE).array)))\
			) : (\
				memcpy((DEQUE).array + (DEQUE).arrsize - (NEL) + (DEQUE).firstel, (ELEMPTR), ((NEL) - (DEQUE).firstel) * sizeof(*((DEQUE).array))),\
				memcpy((DEQUE).array, (ELEMPTR) + (NEL) - (DEQUE).firstel, (DEQUE).firstel * sizeof(*((DEQUE).array)))\
			))\
		),\
		((DEQUE).nel += (NEL)),\
		(DEQUE).firstel = ((DEQUE).firstel + (DEQUE).arrsize - (NEL)) & ((DEQUE).arrsize - 1),0\
		) : (-1)\
	)\
)


#define cu_deque_pop_front(DEQUE)\
(((DEQUE).nel == 0) ? (\
	(DEQUE).firstel = 0\
) : (\
	(DEQUE).firstel = ((DEQUE).firstel + 1) & ((DEQUE).arrsize - 1),\
	(--((DEQUE).nel))\
))

#define cu_deque_popall_front(DEQUE, NEL)\
(((DEQUE).nel < (NEL)) ? (\
	(DEQUE).nel = 0,\
	(DEQUE).firstel = 0\
) : (\
	(DEQUE).firstel = ((DEQUE).firstel + (NEL)) & ((DEQUE).arrsize - 1),\
	(DEQUE).nel -= (NEL)\
))

#define cu_deque_delete(DEQUE, ALLOC) do {\
	cu_allocator_freearray((DEQUE).array, (DEQUE).arrsize, sizeof(*((DEQUE).array)), (ALLOC));\
	(DEQUE).array = NULL;\
	(DEQUE).firstel = 0;\
	(DEQUE).nel = 0;\
	(DEQUE).arrsize = 0;\
} while (0)
