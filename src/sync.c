#include <cu/sync.h>

#ifdef USE_PTHREAD
#include <errno.h>

#define MAX_EINTR 128



void cu_mutex_new(cu_mutex *mut)
{
	cu_mutex newmut = PTHREAD_MUTEX_INITIALIZER;
	*mut = newmut;
}
int cu_mutex_delete(cu_mutex *mut)
{
	if (pthread_mutex_destroy(mut) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}

int cu_mutex_lock(cu_mutex *mut)
{
	if (pthread_mutex_lock(mut) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}
int cu_mutex_lock_try(cu_mutex *mut)
{
	int retval = pthread_mutex_trylock(mut);
	if (retval == EBUSY)
		return CU_SYNC_BUSY;
	if (retval == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}
int cu_mutex_unlock(cu_mutex *mut)
{
	if (pthread_mutex_destroy(mut) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}

int cu_rwmutex_new(cu_rwmutex *mut)
{
	pthread_rwlock_t asdf = PTHREAD_RWLOCK_INITIALIZER;
	*mut = asdf;
	return 0;
}
int cu_rwmutex_delete(cu_rwmutex *mut)
{
	if (pthread_rwlock_destroy(mut) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}

int cu_rwmutex_rlock(cu_rwmutex *mut)
{
	if (pthread_rwlock_rdlock(mut) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}
int cu_rwmutex_rlock_try(cu_rwmutex *mut)
{
	int retval = pthread_rwlock_tryrdlock(mut);
	if (retval == 0)
		return CU_SYNC_OK;
	if (retval == EBUSY)
		return CU_SYNC_BUSY;
	return CU_SYNC_ERR;
}
int cu_rwmutex_runlock(cu_rwmutex *mut)
{
	if (pthread_rwlock_unlock(mut) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}

int cu_rwmutex_wlock(cu_rwmutex *mut)
{
	if (pthread_rwlock_wrlock(mut) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}
int cu_rwmutex_wlock_try(cu_rwmutex *mut)
{
	int retval = pthread_rwlock_trywrlock(mut);
	if (retval == 0)
		return CU_SYNC_OK;
	if (retval == EBUSY)
		return CU_SYNC_BUSY;
	return CU_SYNC_ERR;
}
int cu_rwmutex_wunlock(cu_rwmutex *mut)
{
	if (pthread_rwlock_unlock(mut) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}

int cu_sem_new(cu_sem *sem, unsigned int initval)
{
	if (sem_init(sem, 0, initval) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}
int cu_sem_post(cu_sem *sem)
{
	if (sem_post(sem) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}
int cu_sem_wait(cu_sem *sem)
{
	int eintr_left = MAX_EINTR;
	errno = 0;
	do {
		if (sem_wait(sem) == 0)
			return CU_SYNC_OK;
		--eintr_left;
	} while (errno == EINTR && eintr_left > 0);
	return CU_SYNC_ERR;
}
int cu_sem_wait_try(cu_sem *sem)
{
	int eintr_left = MAX_EINTR;
	errno = 0;
	do {
		if (sem_wait(sem) == 0)
			return CU_SYNC_OK;
		if (errno == EAGAIN)
			return CU_SYNC_BUSY;
		--eintr_left;
	} while (errno == EINTR && eintr_left > 0);
	return CU_SYNC_ERR;
}
int cu_sem_delete(cu_sem *sem)
{
	int retval = sem_destroy(sem);
	if (retval == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}

int cu_thread_create(cu_thread *restrict thread, const struct cu_thread_attr *attr, cu_threadfunc func, void *restrict arg)
{
	pthread_attr_t attrib;
	pthread_attr_t *attribptr = NULL;
	int retval = 0;
	if (attr != NULL && attr->stack_size != 0) {
		attribptr = &attrib;
		if (pthread_attr_init(attribptr) != 0)
			return CU_SYNC_ERR;
		if (pthread_attr_setstacksize(attribptr, attr->stack_size) != 0)
			return CU_SYNC_ERR;
	}
	if (pthread_create(thread, attribptr, func, arg) != 0)
		retval = CU_SYNC_ERR;
	if (attribptr != NULL) {
		pthread_attr_destroy(attribptr);
	}
	return retval;
}
int cu_thread_join(cu_thread *thread, void **retval)
{
	if (pthread_join(*thread, retval) == 0) {
		return CU_SYNC_OK;
	}
	return CU_SYNC_ERR;
}
int cu_thread_detach(cu_thread *thread)
{
	if (pthread_detach(*thread) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}
[[noreturn]] void cu_thread_exit(void *retval)
{
	pthread_exit(retval);
}
int cu_thread_cancel(cu_thread *thread)
{
	if (pthread_cancel(*thread) == 0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}

#elif defined(USE_WIN32)
#include <Windows.h>
#include <stdlib.h>


void cu_mutex_new(cu_mutex *mut)
{
	InitializeCriticalSection(mut);
}
int cu_mutex_delete(cu_mutex *mut)
{
	DeleteCriticalSection(mut);
	return CU_SYNC_OK;
}

int cu_mutex_lock(cu_mutex *mut)
{
	EnterCriticalSection(mut);
	return CU_SYNC_OK;
}
int cu_mutex_lock_try(cu_mutex *mut)
{
	if (TryEnterCriticalSection(mut))
		return CU_SYNC_OK;
	else
		return CU_SYNC_EBUSY;
}
int cu_mutex_unlock(cu_mutex *mut)
{
	LeaveCriticalSection(mut);
	return CU_SYNC_OK;
}


int cu_rwmutex_new(cu_rwmutex *mut)
{
	SRWLOCK mutex = SRWLOCK_INIT;
	*mut = mutex;
	return CU_SYNC_OK;
}
int cu_rwmutex_delete(cu_rwmutex *mut)
{
	return CU_SYNC_OK; // i think SRWLOCKs can be entirely static? idk
}

int cu_rwmutex_rlock(cu_rwmutex *mut)
{
	AcquireSRWLockShared(mut);
	return CU_SYNC_OK;
}
int cu_rwmutex_rlock_try(cu_rwmutex *mut)
{
	if (TryAcquireSRWLockShared(mut))
		return CU_SYNC_OK;
	return CU_SYNC_BUSY;
}
int cu_rwmutex_runlock(cu_rwmutex *mut)
{
	ReleaseSRWLockShared(mut);
	return CU_SYNC_OK;
}

int cu_rwmutex_wlock(cu_rwmutex *mut)
{
	AcquireSRWLockExclusive(mut);
	return CU_SYNC_OK;
}
int cu_rwmutex_wlock_try(cu_rwmutex *mut)
{
	if (TryAcquireSRWLockExclusive(mut))
		return CU_SYNC_OK;
	return CU_SYNC_BUSY;
}
int cu_rwmutex_wunlock(cu_rwmutex *mut)
{
	ReleaseSRWLockExclusive(mut);
	return CU_SYNC_OK;
}

int cu_sem_new(cu_sem *sem, unsigned int initval)
{
	*sem = CreateSemaphoreA(NULL, initval, 2147483647, NULL);
	if (*sem == NULL)
		return CU_SYNC_ERR;
	return CU_SYNC_OK;
}
int cu_sem_post(cu_sem *sem)
{
	if (ReleaseSemaphore(*sem, 1, NULL))
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}
int cu_sem_wait(cu_sem *sem)
{
	if (WaitForSingleObject(*sem, INFINITE) == WAIT_OBJECT_0)
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}
int cu_sem_wait_try(cu_sem *sem)
{
	DWORD retval = WaitForSingleObject(*sem, 0);
	if (retval == WAIT_OBJECT_0)
		return CU_SYNC_OK;
	if (retval == WAIT_TIMEOUT)
		return CU_SYNC_BUSY;
	return CU_SYNC_ERR;
}
int cu_sem_delete(cu_sem *sem)
{
	if (CloseHandle(*sem))
		return CU_SYNC_OK;
	return CU_SYNC_OK;
}


static DWORD WINAPI win32_cb(LPVOID param)
{
	cu_thread *arg = param;
	arg->out_ptr = arg->cb(arg->arg_ptr);
	return 0;
}

int cu_thread_create(cu_thread *restrict thread, const struct cu_thread_attr *attr, cu_threadfunc func, void *restrict arg)
{
	SIZE_T stack_size = attr != NULL ? attr->stack_size : 0;
	thread->out_ptr = NULL;
	thread->arg_ptr = arg;
	thread->cb = func;
	thread->handle = CreateThread(NULL, stack_size, win32_cb, thread, 0, NULL);
	if (thread->handle == NULL)
		return CU_SYNC_ERR;
	return CU_SYNC_OK;
}
int cu_thread_join(cu_thread *thread, void **retval)
{
	if (thread->handle == NULL)
		return CU_SYNC_ERR;
	DWORD result = WaitForSingleObject(thread->handle, INFINITE);
	if (result != WAIT_OBJECT_0)
		return CU_SYNC_ERR;
	*retval = thread->out_ptr;
	if (CloseHandle(thread->handle))
		return CU_SYNC_OK;
	return CU_SYNC_ERR;
}
int cu_thread_detach(cu_thread *thread)
{
	if (CloseHandle(thread->handle)) {
		thread->handle = NULL;
		return CU_SYNC_OK;
	}
	return CU_SYNC_ERR;
}

[[noreturn]] void cu_thread_exit(void *retval)
{
}
int cu_thread_cancel(cu_thread thread);

#elif !defined(__STDC_NO_THREADS__)
#error "C11 thread implementation hasn't been implemented yet"
#else
#	error "Failed to find a suitable synchronization library"
#endif
