#pragma once
#include <stdbool.h>
#include <cu/task.h>

struct cu_pool_unordered;
struct cu_pool_ordered;
struct cu_pool;

// cu_pool_unordered and cu_pool_ordered can be safely typecast two and from cu_pool.
//
// However, you cannot cast cu_pool_unordered to cu_pool_ordered; those types are incompatible.

struct cu_pool_ordered *
cu_pool_new_ordered(size_t nthreads, struct cu_allocator *alloc);

struct cu_pool_unordered *
cu_pool_new_unordered(size_t nthreads, struct cu_allocator *alloc);



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
