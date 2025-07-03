// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once

#include <cu/rc.h>

#ifdef __STDC_NO_ATOMICS__
#error "cu requires C11 atomics to compile."
#endif

#include <stdatomic.h>

#define CU_ARC(DATATYPE)\
struct {\
	struct {\
		DATATYPE *dt;\
		_Atomic size_t refs;\
		_Atomic size_t weakrefs;\
	} *ptr;\
}

#define CU_ARC_WEAK(DATATYPE)\
struct {\
	struct {\
		DATATYPE *dt;\
		_Atomic size_t refs;\
		_Atomic size_t weakrefs;\
	} *weak_ptr;\
}

#define cu_arc_new(CU_ARC, ALLOCPTR) ({\
	(CU_ARC).ptr = cu_allocator_alloc(sizeof(*((CU_ARC).ptr)), (ALLOCPTR));\
	if ((CU_ARC).ptr != NULL) {\
		(CU_ARC).ptr->dt = cu_allocator_alloc(sizeof(*((CU_ARC).ptr->dt)), (ALLOCPTR));\
		if ((CU_ARC).ptr->dt != NULL) {\
			atomic_init(&(CU_ARC).ptr->refs, 1);\
			atomic_init(&(CU_ARC).ptr->weakrefs, 0);\
		}\
		else {\
			cu_allocator_free((CU_ARC).ptr, sizeof(*((CU_ARC).ptr)), (ALLOCPTR));\
			(CU_ARC).ptr = NULL;\
		}\
	}\
	(CU_ARC).ptr == NULL ? -1 : 0;\
})

#define cu_arc_isnull cu_rc_isnull
#define cu_arc_copy cu_rc_copy
#define cu_arc_ptr cu_rc_ptr
#define cu_arc_free cu_rc_free
#define cu_arc_weak_new cu_rc_weak_new
#define cu_arc_weak_expired cu_rc_weak_expired
#define cu_arc_weak_lock(CU_ARC_WEAK) ({\
	CU_WEAKRC_TO_RC_TYPE_INTERNAL_(CU_ARC_WEAK) CU_ARC_WEAK_LOCK_RETVAL_INTERNAL_ = {.ptr = NULL};\
	/* keep looping until either all the arcs have died or we lknow at least one is alive and hasn't died by the time the loop ends, in which case we increment rcnt and make a new ref */\
	do {\
		size_t rcnt = (CU_ARC_WEAK).weak_ptr->refs;\
		if (rcnt == 0) {\
			CU_ARC_WEAK_LOCK_RETVAL_INTERNAL_.ptr = NULL;\
			break;/* the only way this happens */\
			      /* is if all arcs are dead anyways */\
		}\
		else {\
			CU_ARC_WEAK_LOCK_RETVAL_INTERNAL_.ptr = (CU_ARC_WEAK).weak_ptr;\
		}\
		size_t rcnt_inc = rcnt + 1;\
	} while (!atomic_compare_exchange_weak(&(CU_ARC_WEAK).weak_ptr->refs, &rcnt, rcnt_inc));\
	CU_ARC_WEAK_LOCK_RETVAL_INTERNAL_;\
})
#define cu_arc_weak_free cu_rc_weak_free
