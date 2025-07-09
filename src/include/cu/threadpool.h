// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

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
