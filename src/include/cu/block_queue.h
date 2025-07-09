#include <cu/deque.h>
#include <cu/sync.h>

#define CU_BLOCK_QUEUE_TYPE(TYPENAME)\
struct {\
	CU_DEQUE_TYPE(TYPENAME) deque;\
	cu_sem sem;\
	int tmp_storage;\
}

#define cu_block_queue_new(QUEUE, ALLOC) (\
	cu_deque_new((QUEUE).deque, ALLOC) == 0 ? (\
		cu_sem_init((QUEUE).sem, 0)\
	) : (thrd_error)\
)

#define cu_block_queue_add(QUEUE, ELEM, ALLOC) (\
	(cu_sem_post_lock(&((QUEUE).sem)) == NULL) ? (thrd_error) : (\
		(cu_deque_push_back((QUEUE).deque, ELEM, ALLOC) == 0) ? (\
			mtx_unlock(&((QUEUE).sem.mutex))\
		) : (\
			mtx_unlock(&((QUEUE).sem.mutex)),thrd_error\
		)\
	)\
)

#define cu_block_queue_try_remove(QUEUE, OUTELEM, ALLOC) (\
	(QUEUE).tmp_storage = cu_sem_try_wait_lock(NULL, &((QUEUE).sem)),\
	((QUEUE).tmp_storage == thrd_success ? (\
		(OUTELEM) = cu_deque_front((QUEUE).deque),\
		cu_deque_pop_front((QUEUE).deque),\
		mtx_unlock(&((QUEUE).sem.mutex))\
	) : ((QUEUE).tmp_storage))\
)

#define cu_block_queue_remove(QUEUE, OUTELEM, ALLOC) (\
	cu_sem_wait_lock(&((QUEUE).sem)) == NULL ? (thrd_error) : (\
		(OUTELEM) = cu_deque_front((QUEUE).deque),\
		cu_deque_pop_front((QUEUE).deque),\
		mtx_unlock(&((QUEUE).sem.mutex))\
	)\
)

#define cu_block_queue_delete(QUEUE, ALLOC) do {\
	cu_deque_delete((QUEUE).deque, ALLOC);\
	cu_sem_destroy((QUEUE).sem);\
} while (0)
