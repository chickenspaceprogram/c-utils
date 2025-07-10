#include <cu/pool.h>
#include <cu/deque.h>

#define CMP_CU_TASK_PRI(P1, P2) ((P1).priority - (P2).priority)

static int kill_task_fn(void *arg)
{
	thrd_exit(0);
}

static const struct cu_task KILL_TASK = {
	.func = kill_task_fn,
	.arg = NULL,
	.retval = 0
};

struct cu_task_pri {
	struct cu_task task;
	int64_t priority;
};

struct cu_task_orderer {
	CU_MINHEAP_TYPE(struct cu_task_pri) queue;
	mtx_t mutex;
	cnd_t cond;
	struct cu_task_pri mailbox;
	int64_t cur_priority;
};


struct cu_pool {
	struct cu_allocator *alloc;
	size_t nthreads;
	bool is_ordered:1;
	struct cu_task_queue in;
	union {
		struct cu_task_queue out;
		struct cu_task_orderer order;
	} out_queue;
	thrd_t thread_ids[];
};


static int pool_runner(void *arg)
{
	struct cu_pool *pool;
	while (1) {
		
	}
}

int cu_pool_delete(struct cu_pool *pool)
{
	for (size_t i = 0; i < pool->nthreads; ++i) {
		if (cu_pool_submit(pool, &KILL_TASK) != thrd_success)
			return thrd_error;
	}
	for (size_t i = 0; i < pool->nthreads; ++i) {
		if (thrd_join(pool->thread_ids[i], NULL) != thrd_success)
			return thrd_error;
	}
	cu_block_queue_delete(pool->in_queue, pool->alloc);
	if (pool->is_ordered) {
		cu_minheap_delete(pool->out_queue.order.queue, pool->alloc);
		cnd_destroy(&(pool->out_queue.order.cond));
		mtx_destroy(&(pool->out_queue.order.mutex));
	}
	else {
		cu_block_queue_delete(pool->out_queue.unorder, pool->alloc);
	}
	cu_allocator_free(pool, sizeof(struct cu_pool) + sizeof(thrd_t) * pool->nthreads, pool->alloc);
	return thrd_success;
}

int cu_pool_submit(struct cu_pool *pool, const struct cu_task *task)
{
	return cu_block_queue_add(pool->in_queue, *task, pool->alloc);
}

int cu_pool_wait(struct cu_pool *pool, struct cu_task *task)
{
	if (pool->is_ordered) {
		struct cu_task_pri task;
		int retval = thrd_success;
		cu_block_pri_queue_remove(pool->out_queue.order, task, retval, CMP_CU_TASK_PRI, pool->alloc);
		if (retval != thrd_success)
			return retval;

	}
	else {
	}
}

