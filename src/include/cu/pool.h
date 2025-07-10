#pragma once
#include <stdbool.h>
#include <cu/minheap.h>
#include <cu/sync.h>

struct cu_pool;

struct cu_pool *cu_pool_new(size_t nthreads, bool order_output, struct cu_allocator *alloc);

// Blocks until the thread pool is empty of tasks, then kills the threads and frees any resources associated with the thread pool.
int cu_pool_delete(struct cu_pool *pool);

// Submits a task without blocking for a substantial amount of time
int cu_pool_submit(struct cu_pool *pool, const struct cu_task *task);

// Blocks until a task has fully made it through the threadpool
int cu_pool_wait(struct cu_pool *pool, struct cu_task *task);

// Blocks until the thread pool's task queue has completed.
// You can then use cu_pool_try_wait to get the tasks
int cu_pool_waitall(struct cu_pool *pool);

// Checks whether any tasks are sitting in the output queue;
// if one exists, the task is returned, if not, no blocking occurs.
int cu_pool_try_wait(struct cu_pool *pool, struct cu_task *task);
