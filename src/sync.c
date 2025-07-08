#include <cu/sync.h>

#if 1// !defined(CUTILS_HAVE_C11_THREADS) && defined(CUTILS_HAVE_PTHREADS)
#include <assert.h>
#include <stdint.h>

#include <pthread.h>
#include <time.h>
static_assert(sizeof(intptr_t) >= sizeof(int), "intptr_t should be bigger than int");

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
	if (retval != 0)
		return thrd_error;

	retval = pthread_create(thr, NULL, pthread_dummy_cb, &info);
	if (retval != 0)
		return thrd_error;
	retval = cu_sem_wait(&(info.blocker));
	if (retval != thrd_success)
		return retval;
	
	cu_sem_destroy(&(info.blocker));

	return thrd_error;
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
	return nanosleep(duration, remaining);
}
void thrd_yield(void)
{
	sched_yield();
}
[[noreturn]] void thrd_exit(int res)
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
	return intretval;
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
int mtx_timedlock(
	mtx_t *restrict mutex,
	const struct timespec *restrict time_point
)
{
	if (pthread_mutex_timedlock(mutex, time_point) != 0)
		return thrd_error;
	return thrd_success;
}
int mtx_trylock(mtx_t *mutex)
{
	
}
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


#endif

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
		res = thrd_timedout;
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
