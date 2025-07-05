#pragma once
#include <stddef.h>

enum {
	CU_SYNC_BUSY = 1,
	CU_SYNC_OK = 0,
	CU_SYNC_ERR = -1,
};

typedef void *(*cu_threadfunc)(void *);

struct cu_thread_attr {
	size_t stack_size; // set to 0 -> default stack
};


#if defined(USE_PTHREAD)

#	define _XOPEN_SOURCE 500
#	include <semaphore.h>
#	include <pthread.h>
#	undef _XOPEN_SOURCE

	typedef pthread_mutex_t cu_mutex;
	typedef pthread_rwlock_t cu_rwmutex;
	typedef sem_t cu_sem;
	typedef pthread_t cu_thread;

#elif defined(USE_WIN32)
#	include <Windows.h>

// handles are technically pointer-sized so passing a pointer to them is pointless
// this will be a bit inefficient on windows. but that's a skill issue. just don't use windows
	typedef CRITICAL_SECTION cu_mutex;
	typedef SRWLOCK cu_rwmutex;
	typedef HANDLE cu_sem;
	typedef struct {
		HANDLE handle;
		void *out_ptr;
		void *arg_ptr;
		cu_threadfunc cb;
	} cu_thread;
#else
#	error "Failed to find a library providing synchronization primitives."
#endif


void cu_mutex_new(cu_mutex *mut);
int cu_mutex_delete(cu_mutex *mut);

int cu_mutex_lock(cu_mutex *mut);
int cu_mutex_lock_try(cu_mutex *mut);
int cu_mutex_unlock(cu_mutex *mut);


int cu_rwmutex_new(cu_rwmutex *mut);
int cu_rwmutex_delete(cu_rwmutex *mut);

int cu_rwmutex_rlock(cu_rwmutex *mut);
int cu_rwmutex_rlock_try(cu_rwmutex *mut);
int cu_rwmutex_runlock(cu_rwmutex *mut);

int cu_rwmutex_wlock(cu_rwmutex *mut);
int cu_rwmutex_wlock_try(cu_rwmutex *mut);
int cu_rwmutex_wunlock(cu_rwmutex *mut);

int cu_sem_new(cu_sem *sem, unsigned int initval);
int cu_sem_post(cu_sem *sem);
int cu_sem_wait(cu_sem *sem);
int cu_sem_wait_try(cu_sem *sem);
int cu_sem_delete(cu_sem *sem);

// attr is nullable
int cu_thread_create(cu_thread *restrict thread, const struct cu_thread_attr *attr, cu_threadfunc func, void *restrict arg);
int cu_thread_join(cu_thread *thread, void **retval);
int cu_thread_detach(cu_thread *thread);
[[noreturn]] void cu_thread_exit(void *retval);
int cu_thread_cancel(cu_thread *thread);
