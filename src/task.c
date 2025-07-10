#include <cu/task.h>

#define CMP_TASK_ORDERED(T1, T2) ((T1).index - (T2).index)


int cu_task_queue_new(struct cu_task_queue *queue, struct cu_allocator *alloc)
{
	if (cu_deque_new(queue->queue, alloc) != 0)
		return thrd_error;
	if (cu_sem_init(&queue->sem, 0) != thrd_success)
		return thrd_error;
	queue->current_index = 0;
	queue->alloc = alloc;
	return thrd_success;
}


int cu_task_queue_submit(struct cu_task_queue *queue, const struct cu_task *task)
{
	mtx_t *mut = cu_sem_post_lock(&queue->sem);
	if (mut == NULL)
		return thrd_error;
	if (cu_deque_push_back(queue->queue, *task, queue->alloc) != 0) {
		mtx_unlock(mut);
		return thrd_error; 
	}
	int retval = mtx_unlock(mut);
	if (retval != thrd_success)
		return retval;
	return thrd_success;
}

int64_t cu_task_queue_accept(struct cu_task_queue *queue, struct cu_task *task)
{
	mtx_t *mut = cu_sem_wait_lock(&queue->sem);
	if (mut == NULL)
		return -2;
	*task = cu_deque_front(queue->queue);
	cu_deque_pop_front(queue->queue);
	int64_t retval = (queue->current_index)++;
	if (mtx_unlock(mut) != thrd_success)
		return -2;
	return retval;
}

int64_t cu_task_queue_try_accept(struct cu_task_queue *queue, struct cu_task *task)
{
	mtx_t *mut;
	int ret = cu_sem_try_wait_lock(&queue->sem, &mut);
	if (ret == thrd_success) {
		*task = cu_deque_front(queue->queue);
		cu_deque_pop_front(queue->queue);
		int64_t retindex = (queue->current_index)++;
		if (mtx_unlock(mut) != thrd_success)
			return -2;
		return retindex;
	}
	else if (ret == thrd_busy) {
		return -1;
	}
	else {
		return -2;
	}
}

void cu_task_queue_delete(struct cu_task_queue *queue)
{
	cu_deque_delete(queue->queue, queue->alloc);
	cu_sem_destroy(&queue->sem);
}

int cu_task_orderer_submit(struct cu_task_orderer *orderer, const struct cu_task *task, int64_t index)
{
	int retval = mtx_lock(&orderer->mtx);
	if (retval != thrd_success)
		return retval;
	struct cu_task_ordered tsk = {
		.task = *task,
		.index = index,
	};
	retval = cu_minheap_push(orderer->heap, tsk, CMP_TASK_ORDERED, orderer->alloc);
	if (retval != 0)
		goto err;
	retval = cnd_signal(&orderer->cnd);
	if (retval != thrd_success)
		goto err;
	
	return mtx_unlock(&orderer->mtx);

	err:
	mtx_unlock(&orderer->mtx);
	return thrd_error;
}

int cu_task_orderer_accept(struct cu_task_orderer *orderer, struct cu_task *task, int64_t min_index)
{
	int retval = mtx_lock(&orderer->mtx);
	if (retval != thrd_success)
		return retval;
	if (cu_minheap_size(orderer->heap) != 0 && cu_minheap_top(orderer->heap).index <= min_index)
		goto success;

	retval = cnd_wait(&orderer->cnd, &orderer->mtx);
	if (retval != thrd_success) {
		mtx_unlock(&orderer->mtx);
		return retval;
	}

	success:
	*task = cu_minheap_top(orderer->heap).task;
	cu_minheap_pop(orderer->heap, CMP_TASK_ORDERED);
	return mtx_unlock(&orderer->mtx);
}

int64_t cu_task_orderer_try_accept(struct cu_task_orderer *orderer, struct cu_task *task, int64_t min_index)
{
	int retval = mtx_lock(&orderer->mtx);
	if (retval != thrd_success)
		return retval;
	if (cu_minheap_size(orderer->heap) != 0 && cu_minheap_top(orderer->heap).index <= min_index) {
		*task = cu_minheap_top(orderer->heap).task;
		cu_minheap_pop(orderer->heap, CMP_TASK_ORDERED);
		return mtx_unlock(&orderer->mtx);
	}

	retval = mtx_unlock(&orderer->mtx);
	if (retval != thrd_success)
		return thrd_error;
	return thrd_busy;
}
