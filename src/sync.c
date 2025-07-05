#include <cu/sync.h>

int cu_sem_post(cu_sem *sem)
{
	int retval = mtx_lock(&(sem->mutex));
	++(sem->counter);
	if (retval != thrd_success)
		return retval;
	
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
