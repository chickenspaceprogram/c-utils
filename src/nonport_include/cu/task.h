#pragma once
#include <stdbool.h>
#include <cu/deque.h>
#include <cu/minheap.h>
#include <cu/sync.h>

struct cu_task {
	thrd_start_t func;
	void *arg;
	int retval;
};

struct cu_task_ordered {
	struct cu_task task;
	int64_t index;
};

struct cu_task_queue {
	CU_DEQUE_TYPE(struct cu_task) queue;
	cu_sem sem;
	int64_t current_index;
	struct cu_allocator *alloc;
};

struct cu_task_orderer {
	CU_MINHEAP_TYPE(struct cu_task_ordered) heap;
	mtx_t mtx;
	cnd_t cnd;
	struct cu_allocator *alloc;
};

int cu_task_queue_new(struct cu_task_queue *queue, struct cu_allocator *alloc);
void cu_task_queue_delete(struct cu_task_queue *queue);

// nonblocking
int cu_task_queue_submit(struct cu_task_queue *queue, const struct cu_task *task);

// blocking
int64_t cu_task_queue_accept(struct cu_task_queue *queue, struct cu_task *task);

// nonblocking
int64_t cu_task_queue_try_accept(struct cu_task_queue *queue, struct cu_task *task);

static inline int cu_task_orderer_new(struct cu_task_orderer *orderer, struct cu_allocator *alloc)
{
	cu_minheap_new(orderer->heap, alloc);
	orderer->alloc = alloc;
	int retval = mtx_init(&orderer->mtx, mtx_plain);
	if (retval != thrd_success)
		return retval;
	return cnd_init(&orderer->cnd);
}
static inline void cu_task_orderer_delete(struct cu_task_orderer *orderer)
{
	cu_minheap_delete(orderer->heap, orderer->alloc);
	mtx_destroy(&orderer->mtx);
	cnd_destroy(&orderer->cnd);
}

int cu_task_orderer_submit(struct cu_task_orderer *orderer, const struct cu_task *task, int64_t index);
int cu_task_orderer_accept(struct cu_task_orderer *orderer, struct cu_task *task, int64_t min_index);
int64_t cu_task_orderer_try_accept(struct cu_task_orderer *orderer, struct cu_task *task, int64_t min_index);
