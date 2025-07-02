// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once
#include <rudp/utils/alloc.h>
#include <stddef.h>
#include <rudp/utils/intmanip.h>

// Creating any variable whose name starts with RUDP_VECTOR and ends with _INTERNAL_ is disallowed

#ifndef RUDP_VECTOR_INIT_SIZE
#define RUDP_VECTOR_INIT_SIZE 8
#endif

// Expands to the type of a rudp_vector.
// 
// ELEMENTS_TYPE is the type of the vector elements. It should not be const-qualified.
#define RUDP_VECTOR_TYPE(ELEMENTS_TYPE)\
struct {\
	ELEMENTS_TYPE *data;\
	size_t nel;\
	size_t bufsize;\
}

// Creates a new rudp_vector, given the name of a rudp_vector struct.
// VEC must be a rudp_vector struct that has a type given by RUDP_VECTOR_TYPE() for some type.
// ALLOCATOR_PTR is a pointer to a rudp_allocator struct. It may be NULL.
#define rudp_vector_new(VEC, ALLOCATOR_PTR)\
	((VEC).data = rudp_allocator_allocarray((RUDP_VECTOR_INIT_SIZE), sizeof(*(VEC).data), (ALLOCATOR_PTR)),\
	(VEC).nel = 0,\
	(VEC).bufsize = (RUDP_VECTOR_INIT_SIZE),\
	((VEC).data == NULL) ? -1 : 0)\

// Deletes a rudp_vector.
// VEC must be the name of a rudp_vector previously initialized by rudp_vector_new.
// ALLOCATOR_PTR is a pointer to a rudp_allocator struct. It may be NULL.
// ALLOCATOR_PTR must be the same allocator previously used on the vector.
#define rudp_vector_delete(VEC, ALLOCATOR_PTR)\
	(rudp_allocator_freearray((VEC).data, (VEC).bufsize, sizeof(*(VEC).data), (ALLOCATOR_PTR)))
	
// Gets the number of elements in a rudp_vector.
// VEC must be the name of a rudp_vector previously initialized by rudp_vector_new.
#define rudp_vector_size(VEC)	((VEC).nel)

// Gets the capacity of a rudp_vector; in other words, the number of elements it can hold without reallocating.
// VEC must be the name of a rudp_vector previously initialized by rudp_vector_new.
#define rudp_vector_capacity(VEC)	((VEC).bufsize)

// Deletes all elements from a rudp_vector. Returns nothing.
// VEC must be the name of a rudp_vector previously initialized by rudp_vector_new.
#define rudp_vector_erase(VEC)	(VEC).nel = 0;

// Returns 1 if the rudp_vector is empty, 0 if not.
// VEC must be the name of a rudp_vector previously initialized by rudp_vector_new.
#define rudp_vector_empty(VEC)	((VEC).nel == 0)

// Expands to the value of the element of VEC at INDEX.
// Effectively, this is VEC[INDEX] if VEC was a normal array.
// INDEX must be an integer type, and it must be greater than 0 and less than rudp_vector_capacity(VEC)
#define rudp_vector_at(VEC, INDEX) (((VEC).data)[(INDEX)])

#define rudp_vector_swap_elem(VEC, INDEX1, INDEX2) do {\
	typeof(*(VEC).data) RUDP_VECTOR_SWAP_ELEM_TEMP_INTERNAL_ = (VEC).data[(INDEX1)];\
	(VEC).data[(INDEX1)] = (VEC).data[(INDEX2)];\
	(VEC).data[(INDEX2)] = RUDP_VECTOR_SWAP_ELEM_TEMP_INTERNAL_;\
} while (0)

// Appends the value of the variable named by ELEM to the end of VEC.
// VEC must be the name of a rudp_vector previously initialized by rudp_vector_new.
// ELEM must be the expression whose value you wish to append to VEC.
// ALLOC must be the same allocator previously used with the vector.
#define rudp_vector_push(VEC, ELEM, ALLOC)\
_Generic((ELEM), typeof(*(VEC).data):\
	((rudp_vector_reserve((VEC), (VEC).nel + 1, (ALLOC)) == 0) ? (\
	(VEC).data[((VEC).nel)++] = (ELEM),0\
	) : -1)\
)

// Pops the last element from VEC.
// This function is safe in that popping from an empty vector does nothing.
#define rudp_vector_pop(VEC)\
	(VEC).nel = ((VEC).nel == 0 ? 0 : (VEC).nel - 1);\

// Reserves space for at least NEWSIZE elements in VEC.
// VEC must be the name of a rudp_vector previously initialized by rudp_vector_new.
// NEWSIZE must be an integer type.
// ALLOC must be the same allocator previously used with the vector.
//
// Space for more than NEWSIZE elements may be allocated.
// Returns 0 if successful, -1 if an error occurs.
// If an error occurs, the vector is unchanged.
#define rudp_vector_reserve(VEC, NEWSIZE, ALLOC)\
	rudp_vector_reserve_exact((VEC), rudp_next_pwr_2((NEWSIZE)), (ALLOC))

// Reserves space for exactly NEWSIZE elements in VEC.
// If NEWSIZE < rudp_vector_capacity(VEC), nothing happens.
#define rudp_vector_reserve_exact(VEC, NEWSIZE, ALLOC) ({\
	int RUDP_VECTOR_RESERVE_EXACT_RETVAL_INTERNAL_ = 0;\
	if ((NEWSIZE) > (VEC).bufsize) {\
		typeof((VEC).data) RUDP_VECTOR_RESERVE_EXACT_NEWPTR_INTERNAL = rudp_allocator_reallocarray((VEC).data, (NEWSIZE), (VEC).bufsize, sizeof((*(VEC).data)), (ALLOC));\
		if (RUDP_VECTOR_RESERVE_EXACT_NEWPTR_INTERNAL != NULL) {\
			(VEC).data = RUDP_VECTOR_RESERVE_EXACT_NEWPTR_INTERNAL;\
			(VEC).bufsize = (NEWSIZE);\
		}\
		else {\
			RUDP_VECTOR_RESERVE_EXACT_RETVAL_INTERNAL_ = -1;\
		}\
	}\
	RUDP_VECTOR_RESERVE_EXACT_RETVAL_INTERNAL_;\
})

