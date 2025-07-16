// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once
#include <stdint.h>

#if defined(CUTILS_HAVE_C11_THREADS)
#include <threads.h>
#elif defined(CUTILS_HAVE_PTHREADS)
// define c11 threads in terms of pthreads

#ifdef __STDC_NO_THREADS__
#undef __STDC_NO_THREADS__
#endif

#include <pthread.h>
enum {
    thrd_success = 0,
    thrd_nomem = -2,
    thrd_timedout = -3,
    thrd_busy = -4,
    thrd_error = -1
};

typedef int (*thrd_start_t)(void *);
typedef pthread_mutex_t mtx_t;
typedef pthread_cond_t cnd_t;
typedef pthread_t thrd_t;

enum {
	mtx_plain = 0x1,
	mtx_timed = 0x2,
	mtx_recursive = 0x4,
};


typedef pthread_once_t once_flag;
#define ONCE_FLAG_INIT PTHREAD_ONCE_INIT


typedef pthread_key_t tss_t;
#define TSS_DTOR_ITERATIONS PTHREAD_DESTRUCTOR_ITERATIONS
typedef void (*tss_dtor_t)(void *);


int thrd_create(thrd_t *thr, thrd_start_t func, void *arg);
int thrd_equal(thrd_t lhs, thrd_t rhs);
thrd_t thrd_current(void);
int thrd_sleep(const struct timespec *duration, struct timespec *remaining);
void thrd_yield(void);
void thrd_exit(int res);
int thrd_detach(thrd_t thr);
int thrd_join(thrd_t thr, int *res);

int mtx_init(mtx_t *mutex, int type);
int mtx_lock(mtx_t *mutex);
#ifdef CUTILS_HAVE_PTHREAD_MUTEX_TIMEDLOCK // this isn't available on MacOS
int mtx_timedlock(
	mtx_t *restrict mutex,
	const struct timespec *restrict time_point
);
#endif
int mtx_trylock(mtx_t *mutex);
int mtx_unlock(mtx_t *mutex);
void mtx_destroy(mtx_t *mutex);

void call_once(once_flag *flag, void (*func)(void));

int cnd_init(cnd_t *cond);
int cnd_signal(cnd_t *cond);
int cnd_broadcast(cnd_t *cond);
int cnd_wait(cnd_t *cond, mtx_t *mutex);
int cnd_timedwait(
	cnd_t* restrict cond,
	mtx_t* restrict mutex,
	const struct timespec *restrict time_point
);
void cnd_destroy(cnd_t *cond);

int tss_create(tss_t *tss_key, tss_dtor_t destructor);
void *tss_get(tss_t tss_key);
int tss_set(tss_t tss_id, void *val);
void tss_delete(tss_t tss_id);

#else
#error "c-utils requires either C11 threads or pthreads to compile"
#endif

// presume c11 threading stuff is defined

typedef struct {
	cnd_t cond;
	mtx_t mutex;
	unsigned int counter; // unsigned int to correspond with POSIX semaphores, in case using those is more efficient
} cu_sem;

int cu_sem_init(cu_sem *sem, unsigned int init_value);
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
