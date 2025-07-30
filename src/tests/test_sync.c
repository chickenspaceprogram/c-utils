// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/sync.h>
#include <stdlib.h>
#include <cu/dbgassert.h>
#include <stdbool.h>

#define NUM_EINTR 128

struct thread_exit_info {
	struct timespec sleepamt;
	int exitcode;
	bool free_exitinfo;
};

static int thread_exit(void *info)
{
	struct thread_exit_info *exinf = info;
	struct thread_exit_info exinf_copy = *exinf;
	if (exinf_copy.free_exitinfo)
		free(info);
	struct timespec sleeptime = exinf_copy.sleepamt;
	struct timespec timeleft;
	int retval = 0;
	int eintr_left = NUM_EINTR;
	do {
		retval = thrd_sleep(&sleeptime, &timeleft);
		sleeptime = timeleft;
	} while (retval == -1 && eintr_left-- > 0);
	dbgassert(retval == 0);
	return exinf_copy.exitcode;
}

struct thread_id {
	mtx_t idmut;
	thrd_t thread_id;
};

static int thread_cmpid(void *id)
{
	struct thread_id *thread = id;
	int retval = mtx_lock(&(thread->idmut));
	dbgassert(retval == thrd_success);
	
	thrd_t arg_thrd = thread->thread_id;
	thrd_t this_thrd = thrd_current();

	retval = mtx_unlock(&(thread->idmut));
	dbgassert(retval == thrd_success);
	if (thrd_equal(arg_thrd, this_thrd))
		return 0;
	return -1;
}

static void test_thread_detach(void)
{
	thrd_t thread;
	struct thread_exit_info *exinf = malloc(sizeof(struct thread_exit_info));
	dbgassert(exinf != NULL);
	exinf->sleepamt.tv_nsec = 0;
	exinf->sleepamt.tv_sec = 1;
	exinf->free_exitinfo = true;
	exinf->exitcode = 0;
	int retval = thrd_create(&thread, thread_exit, exinf);
	dbgassert(retval == thrd_success);
	retval = thrd_detach(thread);
	dbgassert(retval == thrd_success);
}

static void test_thread_current(void)
{
	struct thread_id id;
	int result = mtx_init(&(id.idmut), mtx_plain);
	dbgassert(result == thrd_success);
	result = mtx_lock(&(id.idmut));
	dbgassert(result == thrd_success);
	result = thrd_create(&(id.thread_id), thread_cmpid, &id);
	dbgassert(result == thrd_success);
	result = mtx_unlock(&(id.idmut));
	dbgassert(result == thrd_success);
	int out = 1234;
	result = thrd_join(id.thread_id, &out);
	dbgassert(out == 0);
	dbgassert(result == 0);
	mtx_destroy(&(id.idmut));
}

struct {
	int flag;
	mtx_t mutex;
} targetvar;
once_flag flag = ONCE_FLAG_INIT;

static void inc_targetvar(void)
{
	int retval = mtx_lock(&(targetvar.mutex));
	dbgassert(retval == thrd_success);
	++(targetvar.flag);
	retval = mtx_unlock(&(targetvar.mutex));
	dbgassert(retval == thrd_success);
}

static int callonce_targetvar(void *asdf)
{
	call_once(&flag, inc_targetvar);
	return 0;
}


static int thread_exit_cb(void *asdf)
{
	thrd_exit(1234);
	return -1;
}

static void test_thread_exit(void)
{
	thrd_t thread;
	int retval = thrd_create(&thread, thread_exit_cb, NULL);
	dbgassert(retval == thrd_success);
	int thrd_retval = 0;
	retval = thrd_join(thread, &thrd_retval);
	dbgassert(retval == thrd_success);
	dbgassert(thrd_retval == 1234);
}

struct mtx_testing {
	mtx_t mtx;
	volatile uintmax_t counter;
};

static void inc_mtx_testing(struct mtx_testing *test)
{
	int retval = mtx_lock(&(test->mtx));
	dbgassert(retval == thrd_success);
	++(test->counter);
	retval = mtx_unlock(&(test->mtx));
	dbgassert(retval == thrd_success);
}

#define NINCREMENTS 1000000
static int run_mtx_test(void *arg)
{
	struct mtx_testing *test = arg;
	for (uintmax_t i = 0; i < NINCREMENTS; ++i) {
		inc_mtx_testing(test);
	}
	return 0;
}

static void test_mtx_plain(void)
{
	struct mtx_testing test = {
		.counter = 0,
	};
	int retval = mtx_init(&(test.mtx), mtx_plain);
	dbgassert(retval == thrd_success);

	thrd_t thread;
	thrd_t thread2;
	retval = thrd_create(&thread, run_mtx_test, &test);
	dbgassert(retval == thrd_success);
	retval = thrd_create(&thread2, run_mtx_test, &test);
	dbgassert(retval == thrd_success);
	run_mtx_test(&test);
	retval = thrd_join(thread, NULL);
	dbgassert(retval == thrd_success);
	retval = thrd_join(thread2, NULL);
	dbgassert(retval == thrd_success);
	dbgassert(test.counter == NINCREMENTS * 3);

	mtx_destroy(&(test.mtx));
}

static void test_mtx_recursive(void)
{
	mtx_t mtx;
	int retval = mtx_init(&mtx, mtx_plain | mtx_recursive);
	dbgassert(retval == thrd_success);
	retval = mtx_lock(&mtx);
	dbgassert(retval == thrd_success);
	retval = mtx_lock(&mtx);
	dbgassert(retval == thrd_success);
	retval = mtx_unlock(&mtx);
	dbgassert(retval == thrd_success);
	retval = mtx_unlock(&mtx);
	dbgassert(retval == thrd_success);
	mtx_destroy(&mtx);
}

static void test_call_once(void)
{
	thrd_t thrd1;
	thrd_t thrd2;
	thrd_t thrd3;
	int retval = mtx_init(&(targetvar.mutex), mtx_plain);
	dbgassert(retval == thrd_success);
	retval = thrd_create(&thrd1, callonce_targetvar, NULL);
	dbgassert(retval == thrd_success);
	retval = thrd_create(&thrd2, callonce_targetvar, NULL);
	dbgassert(retval == thrd_success);
	retval = thrd_create(&thrd3, callonce_targetvar, NULL);
	dbgassert(retval == thrd_success);
	int thrd_retval = 1234;
	retval = thrd_join(thrd1, &thrd_retval);
	dbgassert(retval == thrd_success);
	dbgassert(thrd_retval == 0);
	retval = thrd_join(thrd2, &thrd_retval);
	dbgassert(retval == thrd_success);
	dbgassert(thrd_retval == 0);
	retval = thrd_join(thrd3, &thrd_retval);
	dbgassert(retval == thrd_success);
	dbgassert(thrd_retval == 0);
	mtx_destroy(&(targetvar.mutex));
}

int main(void) {
	test_thread_current();
	thrd_yield(); // ensuring call doesn't panic
	test_thread_exit();
	test_thread_detach();

	test_mtx_plain();
	test_mtx_recursive();
	test_call_once();
}
