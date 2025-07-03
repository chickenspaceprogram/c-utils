// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once
#include <stddef.h>

// these docs suck but it's 2am and i can't be bothered
//
// as of 2025-06-11 this hasn't been tested because i don't actually
// need it yet
//
// test it yourself idk


// CU_RC is a reference-counter that allows memory to be kept alive for
// multiple consumers, and then freed when it's no longer needed.
//
// It is similar to std::shared_ptr in C++ and Rc in Rust, the distinction
// being that CU_RC is not thread-safe. Accessing a CU_RC simultaneously
// from multiple threads will likely cause issues. If you already have a mutex
// guarding the resource, this isn't an issue, and CU_RC avoids the overhead
// of atomic counters.
//
// CU_ARC is an atomic reference-counter that functions similarly to CU_RC,
// however, it is atomic; you can copy it and free it from multiple threads
// without issue. Note that the data the CU_ARC points to is not protected
// in this way, you'll still have to use atomics or mutexes to ensure it's not
// accessed improperly.
//
// CU_RC_WEAK and CU_ARC_WEAK are weak-references; similar to std::weak_ptr
// in C++. Unlike CU_RC and CU_ARC, they do not own what they point to.
// The thing they point to might get destroyed if all the RC or ARC pointing
// to the object are freed. 
//
// You can call cu_rc_lock or cu_arc_lock to try to convert the weak 
// reference to an RC or ARC. If the object has been freed already, the RC or
// ARC you get will be set to NULL (check this with cu_rc_isnull or
// cu_arc_isnull). If the object still exists, you'll get the corresponding
// RC or ARC to it!
//
// Make sure to free your RC, ARC, RC_WEAK, and ARC_WEAK after you're done
// with them.


#define CU_RC(DATATYPE)\
struct {\
	struct {\
		DATATYPE *dt;\
		size_t refs;\
		size_t weakrefs;\
	} *ptr;\
}

#define CU_RC_WEAK(DATATYPE)\
struct {\
	struct {\
		DATATYPE *dt;\
		size_t refs;\
		size_t weakrefs;\
	} *weak_ptr;\
}
#define CU_RC_TO_WEAKRC_TYPE_INTERNAL_(CU_RC)\
struct {\
	typeof((CU_RC).ptr) weak_ptr;\
}

#define CU_WEAKRC_TO_RC_TYPE_INTERNAL_(CU_WEAK_RC)\
struct {\
	typeof((CU_WEAK_RC).weak_ptr) ptr;\
}


#define cu_rc_new(CU_RC, ALLOCPTR) ({\
	(CU_RC).ptr = cu_allocator_alloc(sizeof(*((CU_RC).ptr)), (ALLOCPTR));\
	if ((CU_RC).ptr != NULL) {\
		(CU_RC).ptr->dt = cu_allocator_alloc(sizeof(*((CU_RC).ptr->dt)), (ALLOCPTR));\
		if ((CU_RC).ptr->dt != NULL) {\
			(CU_RC).ptr->refs = 1;\
			(CU_RC).ptr->weakrefs = 0;\
		}\
		else {\
			cu_allocator_free((CU_RC).ptr, sizeof(*((CU_RC).ptr)), (ALLOCPTR));\
			(CU_RC).ptr = NULL;\
		}\
	}\
	(CU_RC).ptr == NULL ? -1 : 0;\
})

#define cu_rc_isnull(CU_RC) ((CU_RC).ptr == NULL)

#define cu_rc_copy(CU_RC) (++((CU_RC).ptr->refs),(CU_RC))
#define cu_rc_ptr(CU_RC) ((CU_RC).ptr->dt)
#define cu_rc_free(CU_RC, ALLOCPTR) do {\
	if ((CU_RC).ptr->refs == 1) {\
		cu_allocator_free((CU_RC).ptr->dt, sizeof(*((CU_RC).ptr->dt)), (ALLOCPTR))\
		(CU_RC).ptr->dt = NULL;\
		(CU_RC).ptr->refs = 0;\
		if ((CU_RC).ptr->weakrefs == 0)\
			cu_allocator_free((CU_RC).ptr, sizeof(*((CU_RC).ptr)), (ALLOCPTR));\
		(CU_RC).ptr = NULL;\
	}\
	else {\
		--((CU_RC).ptr->refs);\
	}\
while (0)


#define cu_rc_weak_new(CU_RC) (++((CU_RC).ptr->weakrefs),(CU_RC_TO_WEAKRC_TYPE_INTERNAL_(CU_RC)) weak_ptr;}){.weak_ptr = (CU_RC).ptr})
#define cu_rc_weak_expired(CU_RC_WEAK) ((CU_RC_WEAK).ptr->refs == 0)

#define cu_rc_weak_lock(CU_RC_WEAK) ((CU_RC_WEAK).weak_ptr->refs == 0) ? (CU_WEAKRC_TO_RC_TYPE_INTERNAL_(CU_RC_WEAK)){.ptr = NULL} : (CU_WEAKRC_TO_RC_TYPE_INTERNAL_(CU_RC_WEAK)){.ptr = ++((CU_RC_WEAK).weak_ptr->refs),(CU_RC_WEAK).weak_ptr}

#define cu_rc_weak_free(CU_RC_WEAK, ALLOCPTR) do {\
	--((CU_RC_WEAK).weak_ptr->weakrefs);\
	if ((CU_RC_WEAK).weak_ptr->weakrefs == 0 && (CU_RC_WEAK).weak_ptr->refs == 0)\
		cu_allocator_free((CU_RC_WEAK).weak_ptr, sizeof(*((CU_RC_WEAK).ptr)), (ALLOCPTR));\
	(CU_RC_WEAK).weak_ptr = NULL;\
} while (0)

