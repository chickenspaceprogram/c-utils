// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/rand.h>


#ifdef CU_HAVE_ARC4RANDOM

#define _GNU_SOURCE
#include <stdlib.h>
#include <assert.h>

int cu_rand_bytes(uint8_t *buf, size_t nbytes)
{
	assert(nbytes <= CU_RAND_MAX);
	arc4random_buf(buf, nbytes);
	return 0;
}


#elif defined(CU_HAVE_GETENTROPY)
#define _DEFAULT_SOURCE
#include <unistd.h>
#include <assert.h>

int cu_rand_bytes(uint8_t *buf, size_t nbytes)
{
	assert(nbytes <= CU_RAND_MAX);
	return getentropy(buf, nbytes);
}

#elif defined(CU_HAVE_URANDOM)
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define EINTR_MAX 128
#warning "Legacy /dev/urandom interface used"
int cu_rand_bytes(uint8_t *buf, size_t nbytes)
{
	assert(nbytes <= CU_RAND_MAX);
	int fd = -1;
	int i = EINTR_MAX;
	errno = 0;
	do {
		fd = open("/dev/urandom", 0);
		if (fd != -1) {
			break;
		}
		else if (errno != EINTR) {
			return -1;
		}
	} while (i-- > 0);
	if (fd == -1) {
		return -1;
	}
	i = EINTR_MAX;
	size_t nread = 0;
	do {
		int retval = read(fd, buf + nread, nbytes - nread);
		if (retval != -1) {
			nread += retval;
		}
		else if (errno != EINTR) {
			close(fd);
			return -1;
		}
	} while (i-- > 0 && nbytes != nread);
	close(fd);
	if (nbytes == nread) {
		return 0;
	}
	return -1;
}
#elif defined(CU_HAVE_BCRYPT)

#include <assert.h>

// win32 nonsense
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <winternl.h>
#include <ntstatus.h>

int cu_rand_bytes(uint8_t *buf, size_t nbytes)
{
	assert(nbytes <= CU_RAND_MAX);
	NTSTATUS res = BCryptGenRandom(NULL, buf, nbytes,
		BCRYPT_USE_SYSTEM_PREFERRED_RNG);
	if (NT_SUCCESS(res)) {
		return 0;
	}
	return -1;
}


#else
#error "No suitable random number generation facilities found."
#endif
