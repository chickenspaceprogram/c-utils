#pragma once
#include <cu/deque.h>
#include <semaphore.h>
#include <pthread.h>

struct cu_task {
	
};

struct cu_threadpool {
	CU_DEQUE_TYPE(int) in_queue;
	CU_DEQUE_TYPE(int) out_queue;
	sem_t tasks_left;
	pthread_mutex_t in_mutex;
	pthread_mutex_t out_mutex;
};
