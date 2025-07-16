// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <cu/alloc.h>

// Any variable starting with CU_HASHMAP and ending in _INTERNAL_ is restricted; you may not use these names.

#ifndef CU_HASHMAP_INITSIZE
#define CU_HASHMAP_INITSIZE 16
#endif

#ifndef CU_HASHMAP_MAXLOADFACTOR
#define CU_HASHMAP_MAXLOADFACTOR 0.75f
#endif

#define CU_HASHMAP_TYPE(KEYTYPE, VALTYPE)\
	struct {\
		struct {\
			bool is_filled;\
			KEYTYPE key;\
			VALTYPE val;\
		} *array;\
		size_t bufsize;\
		size_t nel;\
	}
// god i love the comma operator
#define cu_hashmap_new(MAP, ALLOCATOR) (\
	(MAP).array = cu_allocator_alloc(sizeof(*((MAP).array)) * (CU_HASHMAP_INITSIZE), (ALLOCATOR)),\
	memset((MAP).array, 0, sizeof(*(MAP).array) * (CU_HASHMAP_INITSIZE)),\
	(MAP).bufsize = 16,\
	(MAP).nel = 0,\
	((MAP).array == NULL) ? (-1) : 0\
)

#define cu_hashmap_delete(MAP, ALLOCATOR) do {\
	cu_allocator_free((MAP).array, (MAP).bufsize * sizeof((*((MAP).array))), (ALLOCATOR));\
} while (0)

#ifdef CU_HASHMAP_AT_INTERNAL_
#error "Cannot define CU_HASHMAP_AT_INTERNAL_ macro!"
#endif

#define CU_HASHMAP_AT_INTERNAL_(MAP, KEY, HASHFN, CMPFN) ({\
	size_t CU_HASHMAP_AT_INDEX_INTERNAL_ = HASHFN((KEY)) & ((MAP).bufsize - 1);\
	CU_TYPEOF(((MAP).array)) CU_HASHMAP_AT_RETVAL_INTERNAL_ = NULL;\
	while ((MAP).array[CU_HASHMAP_AT_INDEX_INTERNAL_].is_filled) {\
		if (CMPFN((KEY), (MAP).array[CU_HASHMAP_AT_INDEX_INTERNAL_].key) == 0) {\
			CU_HASHMAP_AT_RETVAL_INTERNAL_ = &(MAP).array[CU_HASHMAP_AT_INDEX_INTERNAL_];\
			break;\
		}\
		CU_HASHMAP_AT_INDEX_INTERNAL_ = (CU_HASHMAP_AT_INDEX_INTERNAL_ + 1) & ((MAP).bufsize - 1);\
	}\
	CU_HASHMAP_AT_RETVAL_INTERNAL_;\
})

#define cu_hashmap_at(MAP, KEY, HASHFN, CMPFN) ((CU_HASHMAP_AT_INTERNAL_(MAP, KEY, HASHFN, CMPFN) == NULL) ? (NULL) : (&(CU_HASHMAP_AT_INTERNAL_(MAP, KEY, HASHFN, CMPFN))->val))

#ifdef CU_HASHMAP_INSERT_INTERNAL_
#error "CU_HASHMAP_INSERT_INTERNAL_ is reserved for the implementation of cu_hashmap_insert. Please do not define this macro."
#endif

#ifdef CU_HASHMAP_DUMMY_CMP_INTERNAL_
#error "CU_HASHMAP_DUMMY_CMP_INTERNAL_ is reserved for the implementation of cu_hashmap_insert. Please do not define this macro."
#endif

#define CU_HASHMAP_DUMMY_CMP_INTERNAL_(A, B) -1

#define CU_HASHMAP_INSERT_INTERNAL_(MAP, KEY, VAL, HASHFN, CMPFN) do {\
	size_t CU_HASHMAP_INSERT_INDEX_INTERNAL_ = HASHFN((KEY)) & ((MAP).bufsize - 1);\
	while ((MAP).array[CU_HASHMAP_INSERT_INDEX_INTERNAL_].is_filled &&\
		(\
			(CMPFN((KEY), ((MAP).array[CU_HASHMAP_INSERT_INDEX_INTERNAL_].key))) != 0\
		)\
	)\
		CU_HASHMAP_INSERT_INDEX_INTERNAL_ = (CU_HASHMAP_INSERT_INDEX_INTERNAL_ + 1) & ((MAP).bufsize - 1);\
	(MAP).array[CU_HASHMAP_INSERT_INDEX_INTERNAL_].is_filled = true;\
	(MAP).array[CU_HASHMAP_INSERT_INDEX_INTERNAL_].key = (KEY);\
	(MAP).array[CU_HASHMAP_INSERT_INDEX_INTERNAL_].val = (VAL);\
} while (0)

#define cu_hashmap_reserve(MAP, SPACEAMT, ALLOC, HASHFN) (((MAP).nel < ((CU_HASHMAP_MAXLOADFACTOR) * (MAP).bufsize)) ? 0 : ({\
	int CU_HASHMAP_RESERVE_RETVAL_INTERNAL_ = 0;\
	CU_TYPEOF((MAP).array) CU_HASHMAP_RESERVE_NEWARR_INTERNAL_ = cu_allocator_alloc((MAP).bufsize * 2 * sizeof((*((MAP).array))), (ALLOC));\
	CU_TYPEOF((MAP).array) CU_HASHMAP_RESERVE_OLDARR_INTERNAL_ = (MAP).array;\
	if (CU_HASHMAP_RESERVE_NEWARR_INTERNAL_ == NULL) {\
		CU_HASHMAP_RESERVE_RETVAL_INTERNAL_ = -1;\
	}\
	else {\
		memset(CU_HASHMAP_RESERVE_NEWARR_INTERNAL_, 0, ((MAP).bufsize * 2 * sizeof((*((MAP).array)))));\
		(MAP).array = CU_HASHMAP_RESERVE_NEWARR_INTERNAL_;\
		(MAP).bufsize *= 2;\
		for (size_t CU_HASHMAP_RESERVE_INDEX_INTERNAL_ = 0; CU_HASHMAP_RESERVE_INDEX_INTERNAL_ < (MAP).bufsize / 2; ++CU_HASHMAP_RESERVE_INDEX_INTERNAL_) {\
			if (CU_HASHMAP_RESERVE_OLDARR_INTERNAL_[CU_HASHMAP_RESERVE_INDEX_INTERNAL_].is_filled) {\
				CU_HASHMAP_INSERT_INTERNAL_(MAP, CU_HASHMAP_RESERVE_OLDARR_INTERNAL_[CU_HASHMAP_RESERVE_INDEX_INTERNAL_].key, CU_HASHMAP_RESERVE_OLDARR_INTERNAL_[CU_HASHMAP_RESERVE_INDEX_INTERNAL_].val, HASHFN, CU_HASHMAP_DUMMY_CMP_INTERNAL_);\
			}\
		}\
		cu_allocator_free(CU_HASHMAP_RESERVE_OLDARR_INTERNAL_, (MAP).bufsize * sizeof((*((MAP).array))) / 2, (ALLOC));\
	}\
	CU_HASHMAP_RESERVE_RETVAL_INTERNAL_;\
}))


#define cu_hashmap_insert(MAP, KEY, VAL, ALLOC, HASHFN, CMPFN) ((cu_hashmap_reserve(MAP, (MAP).bufsize + 1, ALLOC, HASHFN) == 0) ? ({\
	++((MAP).nel);\
	CU_HASHMAP_INSERT_INTERNAL_(MAP, KEY, VAL, HASHFN, CMPFN);\
	0;\
}) : -1)

#define cu_hashmap_remove(MAP, KEY, HASHFN, CMPFN) do {\
	if (CU_HASHMAP_AT_INTERNAL_(MAP, KEY, HASHFN, CMPFN) != NULL) {\
		CU_HASHMAP_AT_INTERNAL_(MAP, KEY, HASHFN, CMPFN)->is_filled = false;\
	}\
} while (0);

#define CU_HASHMAP_BUCKETTYPE(MAP)\
	CU_TYPEOF(*(MAP).array)

#define cu_hashmap_bucket_getkey(BUCKET) ((BUCKET).key)
#define cu_hashmap_bucket_getval(BUCKET) ((BUCKET).val)

#define CU_HASHMAP_ITERTYPE(MAP)\
	struct {\
		CU_TYPEOF(&MAP) map_ptr;\
		size_t index;\
	}

#define cu_hashmap_new_iter(ITER, MAP)\
	do {\
		(ITER).map_ptr = &MAP;\
		(ITER).index = 0;\
	} while (0)


// returns pointer to CU_HASHMAP_BUCKETTYPE
#define cu_hashmap_iter_next(ITER) ({\
	while ((ITER).index < (ITER).map_ptr->bufsize && !((ITER).map_ptr->array[(ITER).index].is_filled))\
		++((ITER).index);\
	++((ITER).index);\
	((ITER).index - 1 < (ITER).map_ptr->bufsize) ? (\
		(ITER).map_ptr->array + (ITER).index - 1\
	) : NULL;\
})

