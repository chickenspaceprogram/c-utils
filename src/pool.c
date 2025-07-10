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

enum {
	POOL_UNORDERED = 1,
	POOL_ORDERED = 2,
};

struct cu_pool {
	size_t nthreads;
	struct cu_allocator *alloc;
	struct cu_task_queue in_queue;
	mtx_t in_empty_mtx;
	uint8_t order;
};

struct cu_pool_unordered {
	struct cu_pool pool;
	struct cu_task_queue out;
	thrd_t thread_ids[];
};

struct cu_pool_ordered {
	struct cu_pool pool;
	struct cu_task_orderer order;
	thrd_t thread_ids[];
};


static int pool_runner(void *arg)
{
	struct cu_pool *pl = arg;
	while (1) {
		struct cu_task task;
		int64_t index = cu_task_queue_accept(&pl->in_queue, &task);
		if (index < 0)
			return 1;

		task.retval = task.func(task.arg);
		if (pl->order == POOL_ORDERED) {
			struct cu_pool_ordered *pool = arg;
			int retval = cu_task_orderer_submit(&pool->order, &task, index);
			if (retval != thrd_success)
				return 1;
		}
		else {
			struct cu_pool_unordered *pool = arg;
			int retval = cu_task_queue_submit(&pool->out, &task);
			if (retval != thrd_success)
				return 1;
		}
	}
	return 0;
}

int cu_pool_submit(struct cu_pool *pool, const struct cu_task *task)
{
	return cu_task_queue_submit(&pool->in_queue, task);
}

