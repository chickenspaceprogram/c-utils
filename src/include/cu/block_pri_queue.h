#pragma once

#include <cu/minheap.h>
#include <cu/sync.h>

#define CU_BLOCK_PRI_QUEUE_TYPE(TYPENAME)\
struct {\
	CU_MINHEAP_TYPE(TYPENAME) minheap;\
	cu_sem sem;\
	int tmp_storage;\
}

#define cu_block_pri_queue_new(QUEUE, ALLOC) (\
	cu_minheap_new((QUEUE).minheap, ALLOC) == 0 ? (\
		cu_sem_init((QUEUE).sem, 0)\
	) : (thrd_error)\
)

#define cu_block_pri_queue_add(QUEUE, ELEM, CMPFN, ALLOC) (\
	(cu_sem_post_lock(&((QUEUE).sem)) == NULL) ? (thrd_error) : (\
		(cu_minheap_push((QUEUE).minheap, ELEM, CMPFN, ALLOC) == 0) ? (\
			mtx_unlock(&((QUEUE).sem.mutex))\
		) : (\
			mtx_unlock(&((QUEUE).sem.mutex)),thrd_error\
		)\
	)\
)
#define cu_block_pri_queue_try_remove(QUEUE, OUTELEM, CMPFN, ALLOC) (\
	(QUEUE).tmp_storage = cu_sem_try_wait_lock(NULL, &((QUEUE).sem)),\
	((QUEUE).tmp_storage == thrd_success ? (\
		(OUTELEM) = cu_minheap_top((QUEUE).minheap),\
		cu_minheap_pop((QUEUE).minheap, CMPFN),\
		mtx_unlock(&((QUEUE).sem.mutex))\
	) : ((QUEUE).tmp_storage))\
)

#define cu_block_pri_queue_remove(QUEUE, OUTELEM, CMPFN, ALLOC) (\
	cu_sem_wait_lock(&((QUEUE).sem)) == NULL ? (thrd_error) : (\
		(OUTELEM) = cu_minheap_top((QUEUE).minheap),\
		cu_minheap_pop((QUEUE).minheap, CMPFN),\
		mtx_unlock(&((QUEUE).sem.mutex))\
	)\
)

#define cu_block_pri_queue_delete(QUEUE, ALLOC) do {\
	cu_minheap_delete((QUEUE).minheap, ALLOC);\
	cu_sem_destroy((QUEUE).sem);\
} while (0)
