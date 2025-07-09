#include <cu/sync.h>
#include <stdlib.h>
#undef NDEBUG
#include <assert.h>
#include <errno.h>

#define NUM_EINTR 128

static void test_mtx_recursive(void)
{
	mtx_t mtx;
	int retval = mtx_init(&mtx, mtx_plain | mtx_recursive);
	assert(retval == thrd_success);
	retval = mtx_lock(&mtx);
	assert(retval == thrd_success);
	retval = mtx_lock(&mtx);
	assert(retval == thrd_success);
	retval = mtx_unlock(&mtx);
	assert(retval == thrd_success);
	retval = mtx_unlock(&mtx);
	assert(retval == thrd_success);
	mtx_destroy(&mtx);
}

struct {
	int flag;
	mtx_t mutex;
} targetvar;
once_flag flag = ONCE_FLAG_INIT;

void inc_targetvar(void)
{
	int retval = mtx_lock(&(targetvar.mutex));
	assert(retval == thrd_success);
	++(targetvar.flag);
	retval = mtx_unlock(&(targetvar.mutex));
	assert(retval == thrd_success);
}

int callonce_targetvar(void *asdf)
{
	call_once(&flag, inc_targetvar);
	return 0;
}


void test_call_once(void)
{
	thrd_t thrd1;
	thrd_t thrd2;
	thrd_t thrd3;
	int retval = mtx_init(&(targetvar.mutex), mtx_plain);
	assert(retval == thrd_success);
	retval = thrd_create(&thrd1, callonce_targetvar, NULL);
	assert(retval == thrd_success);
	retval = thrd_create(&thrd2, callonce_targetvar, NULL);
	assert(retval == thrd_success);
	retval = thrd_create(&thrd3, callonce_targetvar, NULL);
	assert(retval == thrd_success);
	int thrd_retval = 1234;
	retval = thrd_join(thrd1, &thrd_retval);
	assert(retval == thrd_success);
	assert(thrd_retval == 0);
	retval = thrd_join(thrd2, &thrd_retval);
	assert(retval == thrd_success);
	assert(thrd_retval == 0);
	retval = thrd_join(thrd3, &thrd_retval);
	assert(retval == thrd_success);
	assert(thrd_retval == 0);
	mtx_destroy(&(targetvar.mutex));
}

int main(void) {
	test_mtx_recursive();
	test_call_once();
}
