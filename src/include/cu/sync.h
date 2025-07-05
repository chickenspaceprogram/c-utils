#pragma once

#ifdef CUTILS_HAVE_C11_THREADS
#include <threads.h>
#elif defined(CUTILS_HAVE_PTHREADS)
// define c11 threads in terms of pthreads
enum {
    thrd_success = 0,
    thrd_nomem = -2,
    thrd_timedout = -3,
    thrd_busy = -4,
    thrd_error = -1
};
#else
#error "c-utils requires either C11 threads or pthreads to compile"
#endif

// presume c11 threading stuff is defined
#ifdef CUTILS_HAVE_PTHREADS // prefer pthreads over my ugly implementation
#include <semaphore.h>
typedef sem_t cu_sem;
static inline int cu_sem_init(cu_sem *sem, unsigned int init_value)
{
	int retval = sem_init(sem, 0, init_value);
	if (retval == 0)
		return thrd_success;
	else
		return thrd_error;
}
int cu_sem_post(cu_sem *sem)
{
	if (sem_post(sem) == 0)
		return thrd_success;
	else
		return thrd_error;
}
int cu_sem_try_wait(cu_sem *sem);
int cu_sem_timedwait(cu_sem *sem, const struct timespec *restrict time);
int cu_sem_wait(cu_sem *sem);
void cu_sem_destroy(cu_sem *sem);
#else
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
#endif
