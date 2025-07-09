#pragma once
#include <stdbool.h>
#include <cu/minheap.h>
#include <cu/block_queue.h>
#include <cu/block_pri_queue.h>

struct cu_task {
	thrd_start_t func;
	void *arg;
	int retval;
};

struct cu_pool {
	struct cu_allocator *alloc;
	bool is_ordered:1;
	CU_BLOCK_QUEUE_TYPE(struct cu_task) in_queue;
	union {
		CU_BLOCK_PRI_QUEUE_TYPE(struct cu_task) order;
		CU_BLOCK_QUEUE_TYPE(struct cu_task) unorder;
	} out_queue;
};

int cu_pool_new(struct cu_pool *pool, struct cu_allocator *alloc, size_t nthreads, bool order_output);

int cu_pool_delete(struct cu_pool *pool);

// Submits a task without blocking for a substantial amount of time
int cu_pool_submit(struct cu_pool *pool, const struct cu_task *task);

// Blocks until a task has fully made it through the threadpool
int cu_pool_wait(struct cu_pool *pool, struct cu_task *task);

// Checks whether any tasks are sitting in the output queue;
// if one exists, the task is returned, if not, no blocking occurs.
int cu_pool_try_wait(struct cu_pool *pool, struct cu_task *task);
