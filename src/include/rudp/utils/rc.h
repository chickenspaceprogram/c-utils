#pragma once
#include <stddef.h>

#ifdef __STDC_NO_ATOMICS__
#error "rudp requires C11 atomics to compile."
#endif

#include <stdatomic.h>

// these docs suck but it's 2am and i can't be bothered
//
// as of 2025-06-11 this hasn't been tested because i don't actually
// need it yet
//
// test it yourself idk


// RUDP_RC is a reference-counter that allows memory to be kept alive for
// multiple consumers, and then freed when it's no longer needed.
//
// It is similar to std::shared_ptr in C++ and Rc in Rust, the distinction
// being that RUDP_RC is not thread-safe. Accessing a RUDP_RC simultaneously
// from multiple threads will likely cause issues. If you already have a mutex
// guarding the resource, this isn't an issue, and RUDP_RC avoids the overhead
// of atomic counters.
//
// RUDP_ARC is an atomic reference-counter that functions similarly to RUDP_RC,
// however, it is atomic; you can copy it and free it from multiple threads
// without issue. Note that the data the RUDP_ARC points to is not protected
// in this way, you'll still have to use atomics or mutexes to ensure it's not
// accessed improperly.
//
// RUDP_RC_WEAK and RUDP_ARC_WEAK are weak-references; similar to std::weak_ptr
// in C++. Unlike RUDP_RC and RUDP_ARC, they do not own what they point to.
// The thing they point to might get destroyed if all the RC or ARC pointing
// to the object are freed. 
//
// You can call rudp_rc_lock or rudp_arc_lock to try to convert the weak 
// reference to an RC or ARC. If the object has been freed already, the RC or
// ARC you get will be set to NULL (check this with rudp_rc_isnull or
// rudp_arc_isnull). If the object still exists, you'll get the corresponding
// RC or ARC to it!
//
// Make sure to free your RC, ARC, RC_WEAK, and ARC_WEAK after you're done
// with them.


#define RUDP_RC(DATATYPE)\
struct {\
	struct {\
		DATATYPE *dt;\
		size_t refs;\
		size_t weakrefs;\
	} *ptr;\
}

#define RUDP_RC_WEAK(DATATYPE)\
struct {\
	struct {\
		DATATYPE *dt;\
		size_t refs;\
		size_t weakrefs;\
	} *weak_ptr;\
}
#define RUDP_RC_TO_WEAKRC_TYPE_INTERNAL_(RUDP_RC)\
struct {\
	typeof((RUDP_RC).ptr) weak_ptr;\
}

#define RUDP_WEAKRC_TO_RC_TYPE_INTERNAL_(RUDP_WEAK_RC)\
struct {\
	typeof((RUDP_WEAK_RC).weak_ptr) ptr;\
}


#define rudp_rc_new(RUDP_RC, ALLOCPTR) ({\
	(RUDP_RC).ptr = rudp_allocator_alloc(sizeof(*((RUDP_RC).ptr)), (ALLOCPTR));\
	if ((RUDP_RC).ptr != NULL) {\
		(RUDP_RC).ptr->dt = rudp_allocator_alloc(sizeof(*((RUDP_RC).ptr->dt)), (ALLOCPTR));\
		if ((RUDP_RC).ptr->dt != NULL) {\
			(RUDP_RC).ptr->refs = 1;\
			(RUDP_RC).ptr->weakrefs = 0;\
		}\
		else {\
			rudp_allocator_free((RUDP_RC).ptr, sizeof(*((RUDP_RC).ptr)), (ALLOCPTR));\
			(RUDP_RC).ptr = NULL;\
		}\
	}\
	(RUDP_RC).ptr == NULL ? -1 : 0;\
})

#define rudp_rc_isnull(RUDP_RC) ((RUDP_RC).ptr == NULL)

#define rudp_rc_copy(RUDP_RC) (++((RUDP_RC).ptr->refs),(RUDP_RC))
#define rudp_rc_ptr(RUDP_RC) ((RUDP_RC).ptr->dt)
#define rudp_rc_free(RUDP_RC, ALLOCPTR) do {\
	if ((RUDP_RC).ptr->refs == 1) {\
		rudp_allocator_free((RUDP_RC).ptr->dt, sizeof(*((RUDP_RC).ptr->dt)), (ALLOCPTR))\
		(RUDP_RC).ptr->dt = NULL;\
		(RUDP_RC).ptr->refs = 0;\
		if ((RUDP_RC).ptr->weakrefs == 0)\
			rudp_allocator_free((RUDP_RC).ptr, sizeof(*((RUDP_RC).ptr)), (ALLOCPTR));\
		(RUDP_RC).ptr = NULL;\
	}\
	else {\
		--((RUDP_RC).ptr->refs);\
	}\
while (0)


#define rudp_rc_weak_new(RUDP_RC) (++((RUDP_RC).ptr->weakrefs),(RUDP_RC_TO_WEAKRC_TYPE_INTERNAL_(RUDP_RC)) weak_ptr;}){.weak_ptr = (RUDP_RC).ptr})
#define rudp_rc_weak_expired(RUDP_RC_WEAK) ((RUDP_RC_WEAK).ptr->refs == 0)

#define rudp_rc_weak_lock(RUDP_RC_WEAK) ((RUDP_RC_WEAK).weak_ptr->refs == 0) ? (RUDP_WEAKRC_TO_RC_TYPE_INTERNAL_(RUDP_RC_WEAK)){.ptr = NULL} : (RUDP_WEAKRC_TO_RC_TYPE_INTERNAL_(RUDP_RC_WEAK)){.ptr = ++((RUDP_RC_WEAK).weak_ptr->refs),(RUDP_RC_WEAK).weak_ptr}

#define rudp_rc_weak_free(RUDP_RC_WEAK, ALLOCPTR) do {\
	--((RUDP_RC_WEAK).weak_ptr->weakrefs);\
	if ((RUDP_RC_WEAK).weak_ptr->weakrefs == 0 && (RUDP_RC_WEAK).weak_ptr->refs == 0)\
		rudp_allocator_free((RUDP_RC_WEAK).weak_ptr, sizeof(*((RUDP_RC_WEAK).ptr)), (ALLOCPTR));\
	(RUDP_RC_WEAK).weak_ptr = NULL;\
} while (0)


#define RUDP_ARC(DATATYPE)\
struct {\
	struct {\
		DATATYPE *dt;\
		_Atomic size_t refs;\
		_Atomic size_t weakrefs;\
	} *ptr;\
}

#define RUDP_ARC_WEAK(DATATYPE)\
struct {\
	struct {\
		DATATYPE *dt;\
		_Atomic size_t refs;\
		_Atomic size_t weakrefs;\
	} *weak_ptr;\
}

#define rudp_arc_new(RUDP_ARC, ALLOCPTR) ({\
	(RUDP_ARC).ptr = rudp_allocator_alloc(sizeof(*((RUDP_ARC).ptr)), (ALLOCPTR));\
	if ((RUDP_ARC).ptr != NULL) {\
		(RUDP_ARC).ptr->dt = rudp_allocator_alloc(sizeof(*((RUDP_ARC).ptr->dt)), (ALLOCPTR));\
		if ((RUDP_ARC).ptr->dt != NULL) {\
			atomic_init(&(RUDP_ARC).ptr->refs, 1);\
			atomic_init(&(RUDP_ARC).ptr->weakrefs, 0);\
		}\
		else {\
			rudp_allocator_free((RUDP_ARC).ptr, sizeof(*((RUDP_ARC).ptr)), (ALLOCPTR));\
			(RUDP_ARC).ptr = NULL;\
		}\
	}\
	(RUDP_ARC).ptr == NULL ? -1 : 0;\
})

#define rudp_arc_isnull rudp_rc_isnull
#define rudp_arc_copy rudp_rc_copy
#define rudp_arc_ptr rudp_rc_ptr
#define rudp_arc_free rudp_rc_free
#define rudp_arc_weak_new rudp_rc_weak_new
#define rudp_arc_weak_expired rudp_rc_weak_expired
#define rudp_arc_weak_lock(RUDP_ARC_WEAK) ({\
	RUDP_WEAKRC_TO_RC_TYPE_INTERNAL_(RUDP_ARC_WEAK) RUDP_ARC_WEAK_LOCK_RETVAL_INTERNAL_ = {.ptr = NULL};\
	/* keep looping until either all the arcs have died or we lknow at least one is alive and hasn't died by the time the loop ends, in which case we increment rcnt and make a new ref */\
	do {\
		size_t rcnt = (RUDP_ARC_WEAK).weak_ptr->refs;\
		if (rcnt == 0) {\
			RUDP_ARC_WEAK_LOCK_RETVAL_INTERNAL_.ptr = NULL;\
			break;/* the only way this happens */\
			      /* is if all arcs are dead anyways */\
		}\
		else {\
			RUDP_ARC_WEAK_LOCK_RETVAL_INTERNAL_.ptr = (RUDP_ARC_WEAK).weak_ptr;\
		}\
		size_t rcnt_inc = rcnt + 1;\
	} while (!atomic_compare_exchange_weak(&(RUDP_ARC_WEAK).weak_ptr->refs, &rcnt, rcnt_inc));\
	RUDP_ARC_WEAK_LOCK_RETVAL_INTERNAL_;\
})
#define rudp_arc_weak_free rudp_rc_weak_free
