// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/sync.h>

#if !defined(CUTILS_HAVE_C11_THREADS) && defined(CUTILS_HAVE_PTHREADS)
#include <assert.h>
#include <stdint.h>

#include <pthread.h>
#include <time.h>
#include <errno.h>
static_assert(sizeof(intptr_t) >= sizeof(int), "intptr_t should be bigger than int");

#define STR(PARAM) #PARAM
#define TOSTRING(PARAM) STR(PARAM)

struct cbinfo {
	thrd_start_t func;
	void *arg;
	cu_sem blocker;
};

static void *pthread_dummy_cb(void *arg)
{
	struct cbinfo *info = arg;

	void *fnarg = info->arg;
	thrd_start_t fn = info->func;

	cu_sem_post(&(info->blocker)); // gotten everything we need, now post the semaphore and move on

	intptr_t retval = (intptr_t)fn(fnarg);
	return (void *)retval;
}


int thrd_create(thrd_t *thr, thrd_start_t func, void *arg)
{
	struct cbinfo info = {
		.func = func,
		.arg = arg,
	};
	int retval = cu_sem_init(&(info.blocker), 0);
	if (retval != thrd_success) {
		return retval;
	}

	retval = pthread_create(thr, NULL, pthread_dummy_cb, &info);
	if (retval != 0) {
		return thrd_error;
	}
	retval = cu_sem_wait(&(info.blocker));
	if (retval != thrd_success) {
		return retval;
	}
	
	cu_sem_destroy(&(info.blocker));

	return thrd_success;
}

int thrd_equal(thrd_t lhs, thrd_t rhs)
{
	return pthread_equal(lhs, rhs);
}
thrd_t thrd_current(void)
{
	return pthread_self();
}
int thrd_sleep(const struct timespec *duration, struct timespec *remaining)
{
	int retval = nanosleep(duration, remaining);
	if (retval != 0 && errno == EINTR)
		return -1;
	if (retval != 0)
		return -2;
	return 0;
}
void thrd_yield(void)
{
	sched_yield();
}
void thrd_exit(int res)
{
	intptr_t resptr = (intptr_t)res;
	pthread_exit((void *)resptr);
}
int thrd_detach(thrd_t thr)
{
	if (pthread_detach(thr) == 0)
		return thrd_success;
	return thrd_error;
}
int thrd_join(thrd_t thr, int *res)
{
	void *retval;
	if (pthread_join(thr, &retval) != 0)
		return thrd_error;
	intptr_t intretval = (intptr_t)retval;
	if (res != NULL)
		*res = intretval;
	return thrd_success;
}

typedef pthread_mutex_t mtx_t;

int mtx_init(mtx_t *mutex, int type)
{
	if ((type & mtx_plain) && (type & mtx_timed))
		return thrd_error;
	if (!(type & mtx_plain) && !(type & mtx_timed))
		return thrd_error;
	
	if (!(type & mtx_recursive)) {
		pthread_mutex_t new_mutex = PTHREAD_MUTEX_INITIALIZER;
		*mutex = new_mutex;
		return thrd_success;
	}
	
	pthread_mutexattr_t attr;
	if (pthread_mutexattr_init(&attr) != 0)
		return thrd_error;
	if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) != 0)
		return thrd_error;
	
	int retval = pthread_mutex_init(mutex, &attr);

	if (pthread_mutexattr_destroy(&attr) != 0)
		return thrd_error;
	if (retval != 0)
		return thrd_error;
	return thrd_success;
}

int mtx_lock(mtx_t *mutex)
{
	if (pthread_mutex_lock(mutex) != 0)
		return thrd_error;
	return thrd_success;
}
#ifdef CUTILS_HAVE_PTHREAD_MUTEX_TIMEDLOCK // this isn't available on MacOS
int mtx_timedlock(
	mtx_t *restrict mutex,
	const struct timespec *restrict time_point
)
{
	if (pthread_mutex_timedlock(mutex, time_point) != 0)
		return thrd_error;
	return thrd_success;
}
#endif
int mtx_trylock(mtx_t *mutex)
{
	if (pthread_mutex_trylock(mutex) != 0)
		return thrd_error;
	return thrd_success;
}
int mtx_unlock(mtx_t *mutex)
{
	if (pthread_mutex_unlock(mutex) != 0)
		return thrd_error;
	return thrd_success;
}
void mtx_destroy(mtx_t *mutex)
{
	pthread_mutex_destroy(mutex);
}

void call_once(once_flag *flag, void (*func)(void))
{
	int retval = pthread_once(flag, func);
	assert(retval == 0);
}

int cnd_init(cnd_t *cond)
{
	pthread_cond_t cnd = PTHREAD_COND_INITIALIZER;
	*cond = cnd;
	return thrd_success;
}
int cnd_signal(cnd_t *cond)
{
	if (pthread_cond_signal(cond) != 0)
		return thrd_error;
	return thrd_success;
}
int cnd_broadcast(cnd_t *cond)
{
	if (pthread_cond_broadcast(cond) != 0)
		return thrd_error;
	return thrd_success;
}
int cnd_wait(cnd_t *cond, mtx_t *mutex)
{
	if (pthread_cond_wait(cond, mutex) != 0)
		return thrd_error;
	return thrd_success;
}
int cnd_timedwait(
	cnd_t* restrict cond,
	mtx_t* restrict mutex,
	const struct timespec *restrict time_point
) {
	if (pthread_cond_timedwait(cond, mutex, time_point) != 0)
		return thrd_error;
	return thrd_success;
}
void cnd_destroy(cnd_t *cond)
{
	int retval = pthread_cond_destroy(cond);
	assert(retval == 0);
}

int tss_create(tss_t *tss_key, tss_dtor_t destructor)
{
	if (pthread_key_create(tss_key, destructor) != 0)
		return thrd_error;
	return thrd_success;
}
void *tss_get(tss_t tss_key)
{
	return pthread_getspecific(tss_key);
}
int tss_set(tss_t tss_id, void *val)
{
	if (pthread_setspecific(tss_id, val) != 0)
		return thrd_error;
	return thrd_success;
}
void tss_delete(tss_t tss_id)
{
	int retval = pthread_key_delete(tss_id);
	assert(retval == 0);
}


#endif

int cu_sem_init(cu_sem *sem, size_t init_value)
{
	sem->counter = init_value;
	int retval = cnd_init(&(sem->cond));
	if (retval != thrd_success)
		return retval;
	retval = mtx_init(&(sem->mutex), mtx_plain);
	if (retval != thrd_success)
		return retval;
	return cnd_init(&(sem->cond));
}

int cu_sem_post(cu_sem *sem)
{
	int retval = mtx_lock(&(sem->mutex));
	if (retval != thrd_success)
		return retval;
	++(sem->counter);
	
	retval = cnd_signal(&(sem->cond));
	int unlock_res = mtx_unlock(&(sem->mutex));

	if (retval != thrd_success)
		return retval;
	if (unlock_res != thrd_success)
		return unlock_res;
	return thrd_success;

}

mtx_t *cu_sem_post_lock(cu_sem *sem)
{
	int retval = mtx_lock(&(sem->mutex));
	if (retval != thrd_success)
		return NULL;
	++(sem->counter);

	retval = cnd_signal(&(sem->cond));
	if (retval != thrd_success) {
		mtx_unlock(&(sem->mutex));
		return NULL;
	}
	return &(sem->mutex);
}

int cu_sem_try_wait(cu_sem *sem)
{
	int res = mtx_lock(&(sem->mutex));
	if (res != thrd_success)
		return res;
	if (sem->counter > 0) {
		--(sem->counter);
		res = thrd_success;
	}
	else {
		res = thrd_busy;
	}
	int err = mtx_unlock(&(sem->mutex));
	if (err != thrd_success)
		return err;
	return res;
}

int cu_sem_timedwait(cu_sem *sem, const struct timespec *restrict time)
{
	int res = mtx_lock(&(sem->mutex));
	if (res != thrd_success)
		return res;
	if (sem->counter > 0) {
		--(sem->counter);
		res = mtx_unlock(&(sem->mutex));
		if (res != thrd_success)
			return res;
		return thrd_success;
	}
	do {
		if (time == NULL)
			res = cnd_wait(&(sem->cond), &(sem->mutex));
		else
			res = cnd_timedwait(&(sem->cond), &(sem->mutex), time);
		
		if (res != thrd_success) {
			mtx_unlock(&(sem->mutex));
			return res;
		}
	} while (sem->counter == 0);
	--(sem->counter);
	res = mtx_unlock(&(sem->mutex));
	if (res != thrd_success)
		return res;
	return thrd_success;
}

int cu_sem_try_wait_lock(cu_sem *sem, mtx_t **mtx)
{
	int res = mtx_lock(&(sem->mutex));
	if (res != thrd_success)
		return res;
	if (sem->counter > 0) {
		--(sem->counter);
		res = thrd_success;
		if (mtx != NULL)
			*mtx = &(sem->mutex);
	}
	else {
		res = thrd_busy;
		if (mtx_unlock(&(sem->mutex)) != thrd_success)
			return thrd_error;
	}
	return res;
}

mtx_t *cu_sem_wait_lock(cu_sem *sem)
{
	int retval = mtx_lock(&(sem->mutex));
	if (retval != thrd_success)
		return NULL;
	if (sem->counter > 0) {
		--(sem->counter);
		return &(sem->mutex);
	}
	do {
		retval = cnd_wait(&(sem->cond), &(sem->mutex));
		if (retval != thrd_success) {
			mtx_unlock(&(sem->mutex));
			return NULL;
		}
	} while (sem->counter == 0);
	--(sem->counter);
	return &(sem->mutex);
}
