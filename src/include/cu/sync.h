#pragma once

#include <threads.h>
#include <stdatomic.h>


typedef struct {
	unsigned int counter; // unsigned int to correspond with POSIX semaphores
	cnd_t cond;
	mtx_t mutex;
} cu_sem;

static inline int cu_sem_init(cu_sem *sem, unsigned int init_value)
{
	sem->counter = init_value;
	return cnd_init(&(sem->cond));
}
int cu_sem_post(cu_sem *sem);
int cu_sem_try_wait(cu_sem *sem);
int cu_sem_timedwait(cu_sem *sem, const struct timespec *restrict time);
static inline int cu_sem_wait(cu_sem *sem)
{
	return cu_sem_timedwait(sem, NULL);
}
static inline void cu_sem_destroy(cu_sem *sem)
{
	cnd_destroy(&(sem->cond));
	mtx_destroy(&(sem->mutex));
}
