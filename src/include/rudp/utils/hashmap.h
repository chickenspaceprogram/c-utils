// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <rudp/utils/alloc.h>

// Any variable starting with RUDP_HASHMAP and ending in _INTERNAL_ is restricted; you may not use these names.

#ifndef RUDP_HASHMAP_INITSIZE
#define RUDP_HASHMAP_INITSIZE 16
#endif

#ifndef RUDP_HASHMAP_MAXLOADFACTOR
#define RUDP_HASHMAP_MAXLOADFACTOR 0.75f
#endif

#define RUDP_HASHMAP_TYPE(KEYTYPE, VALTYPE)\
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
#define rudp_hashmap_new(MAP, ALLOCATOR) (\
	(MAP).array = rudp_allocator_alloc(sizeof(*((MAP).array)) * (RUDP_HASHMAP_INITSIZE), (ALLOCATOR)),\
	memset((MAP).array, 0, sizeof(*(MAP).array) * (RUDP_HASHMAP_INITSIZE)),\
	(MAP).bufsize = 16,\
	(MAP).nel = 0,\
	((MAP).array == NULL) ? (-1) : 0\
)

#define rudp_hashmap_delete(MAP, ALLOCATOR) do {\
	rudp_allocator_free((MAP).array, (MAP).bufsize * sizeof((*((MAP).array))), (ALLOCATOR));\
} while (0)

#ifdef RUDP_HASHMAP_AT_INTERNAL_
#error "Cannot define RUDP_HASHMAP_AT_INTERNAL_ macro!"
#endif

#define RUDP_HASHMAP_AT_INTERNAL_(MAP, KEY, HASHFN, CMPFN) ({\
	size_t RUDP_HASHMAP_AT_INDEX_INTERNAL_ = HASHFN((KEY)) & ((MAP).bufsize - 1);\
	typeof(((MAP).array)) RUDP_HASHMAP_AT_RETVAL_INTERNAL_ = NULL;\
	while ((MAP).array[RUDP_HASHMAP_AT_INDEX_INTERNAL_].is_filled) {\
		if (CMPFN((KEY), (MAP).array[RUDP_HASHMAP_AT_INDEX_INTERNAL_].key) == 0) {\
			RUDP_HASHMAP_AT_RETVAL_INTERNAL_ = &(MAP).array[RUDP_HASHMAP_AT_INDEX_INTERNAL_];\
			break;\
		}\
		RUDP_HASHMAP_AT_INDEX_INTERNAL_ = (RUDP_HASHMAP_AT_INDEX_INTERNAL_ + 1) & ((MAP).bufsize - 1);\
	}\
	RUDP_HASHMAP_AT_RETVAL_INTERNAL_;\
})

#define rudp_hashmap_at(MAP, KEY, HASHFN, CMPFN) ((RUDP_HASHMAP_AT_INTERNAL_(MAP, KEY, HASHFN, CMPFN) == NULL) ? (NULL) : (&(RUDP_HASHMAP_AT_INTERNAL_(MAP, KEY, HASHFN, CMPFN))->val))

#ifdef RUDP_HASHMAP_INSERT_INTERNAL_
#error "RUDP_HASHMAP_INSERT_INTERNAL_ is reserved for the implementation of rudp_hashmap_insert. Please do not define this macro."
#endif

#ifdef RUDP_HASHMAP_DUMMY_CMP_INTERNAL_
#error "RUDP_HASHMAP_DUMMY_CMP_INTERNAL_ is reserved for the implementation of rudp_hashmap_insert. Please do not define this macro."
#endif

#define RUDP_HASHMAP_DUMMY_CMP_INTERNAL_(A, B) -1

#define RUDP_HASHMAP_INSERT_INTERNAL_(MAP, KEY, VAL, HASHFN, CMPFN) do {\
	size_t RUDP_HASHMAP_INSERT_INDEX_INTERNAL_ = HASHFN((KEY)) & ((MAP).bufsize - 1);\
	while ((MAP).array[RUDP_HASHMAP_INSERT_INDEX_INTERNAL_].is_filled &&\
		(\
			(CMPFN((KEY), ((MAP).array[RUDP_HASHMAP_INSERT_INDEX_INTERNAL_].key))) != 0\
		)\
	)\
		RUDP_HASHMAP_INSERT_INDEX_INTERNAL_ = (RUDP_HASHMAP_INSERT_INDEX_INTERNAL_ + 1) & ((MAP).bufsize - 1);\
	(MAP).array[RUDP_HASHMAP_INSERT_INDEX_INTERNAL_].is_filled = true;\
	(MAP).array[RUDP_HASHMAP_INSERT_INDEX_INTERNAL_].key = (KEY);\
	(MAP).array[RUDP_HASHMAP_INSERT_INDEX_INTERNAL_].val = (VAL);\
} while (0)

#define rudp_hashmap_reserve(MAP, SPACEAMT, ALLOC, HASHFN) (((MAP).nel < ((RUDP_HASHMAP_MAXLOADFACTOR) * (MAP).bufsize)) ? 0 : ({\
	int RUDP_HASHMAP_RESERVE_RETVAL_INTERNAL_ = 0;\
	typeof((MAP).array) RUDP_HASHMAP_RESERVE_NEWARR_INTERNAL_ = rudp_allocator_alloc((MAP).bufsize * 2 * sizeof((*((MAP).array))), (ALLOC));\
	typeof((MAP).array) RUDP_HASHMAP_RESERVE_OLDARR_INTERNAL_ = (MAP).array;\
	if (RUDP_HASHMAP_RESERVE_NEWARR_INTERNAL_ == NULL) {\
		RUDP_HASHMAP_RESERVE_RETVAL_INTERNAL_ = -1;\
	}\
	else {\
		memset(RUDP_HASHMAP_RESERVE_NEWARR_INTERNAL_, 0, ((MAP).bufsize * 2 * sizeof((*((MAP).array)))));\
		(MAP).array = RUDP_HASHMAP_RESERVE_NEWARR_INTERNAL_;\
		(MAP).bufsize *= 2;\
		for (size_t RUDP_HASHMAP_RESERVE_INDEX_INTERNAL_ = 0; RUDP_HASHMAP_RESERVE_INDEX_INTERNAL_ < (MAP).bufsize / 2; ++RUDP_HASHMAP_RESERVE_INDEX_INTERNAL_) {\
			if (RUDP_HASHMAP_RESERVE_OLDARR_INTERNAL_[RUDP_HASHMAP_RESERVE_INDEX_INTERNAL_].is_filled) {\
				RUDP_HASHMAP_INSERT_INTERNAL_(MAP, RUDP_HASHMAP_RESERVE_OLDARR_INTERNAL_[RUDP_HASHMAP_RESERVE_INDEX_INTERNAL_].key, RUDP_HASHMAP_RESERVE_OLDARR_INTERNAL_[RUDP_HASHMAP_RESERVE_INDEX_INTERNAL_].val, HASHFN, RUDP_HASHMAP_DUMMY_CMP_INTERNAL_);\
			}\
		}\
		rudp_allocator_free(RUDP_HASHMAP_RESERVE_OLDARR_INTERNAL_, (MAP).bufsize * sizeof((*((MAP).array))) / 2, (ALLOC));\
	}\
	RUDP_HASHMAP_RESERVE_RETVAL_INTERNAL_;\
}))


#define rudp_hashmap_insert(MAP, KEY, VAL, ALLOC, HASHFN, CMPFN) ((rudp_hashmap_reserve(MAP, (MAP).bufsize + 1, ALLOC, HASHFN) == 0) ? ({\
	++((MAP).nel);\
	RUDP_HASHMAP_INSERT_INTERNAL_(MAP, KEY, VAL, HASHFN, CMPFN);\
	0;\
}) : -1)

#define rudp_hashmap_remove(MAP, KEY, HASHFN, CMPFN) do {\
	if (RUDP_HASHMAP_AT_INTERNAL_(MAP, KEY, HASHFN, CMPFN) != NULL) {\
		RUDP_HASHMAP_AT_INTERNAL_(MAP, KEY, HASHFN, CMPFN)->is_filled = false;\
	}\
} while (0);

#define RUDP_HASHMAP_BUCKETTYPE(MAP)\
	typeof(*(MAP).array)

#define rudp_hashmap_bucket_getkey(BUCKET) ((BUCKET).key)
#define rudp_hashmap_bucket_getval(BUCKET) ((BUCKET).val)

#define RUDP_HASHMAP_ITERTYPE(MAP)\
	struct {\
		typeof(&MAP) map_ptr;\
		size_t index;\
	}

#define rudp_hashmap_new_iter(ITER, MAP)\
	do {\
		(ITER).map_ptr = &MAP;\
		(ITER).index = 0;\
	} while (0)


// returns pointer to RUDP_HASHMAP_BUCKETTYPE
#define rudp_hashmap_iter_next(ITER) ({\
	while ((ITER).index < (ITER).map_ptr->bufsize && !((ITER).map_ptr->array[(ITER).index].is_filled))\
		++((ITER).index);\
	++((ITER).index);\
	((ITER).index - 1 < (ITER).map_ptr->bufsize) ? (\
		(ITER).map_ptr->array + (ITER).index - 1\
	) : NULL;\
})

