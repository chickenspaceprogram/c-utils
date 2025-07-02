// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#if (CHAR_BIT != 8)
#error "CHAR_BIT != 8 on your platform."
#endif

static inline size_t logbase2(size_t num)
{
	size_t counter = 0;
	while ((num >>= 1) > 0)
		++counter;
	return counter;
}

static inline size_t rudp_next_pwr_2(size_t num)
{
	if (sizeof(size_t) != 1 << logbase2(sizeof(size_t))) {
		fputs("Fatal error: The sizeof(size_t) on your computer is not a power of 2.\n", stderr);
		exit(EXIT_FAILURE); // runtime panic, compiletime would be better but this is C and we can't have nice things
	}
	--num;
	num |= num >> 1;
	num |= num >> 2;
	num |= num >> 4;
	for (size_t i = 0; i < logbase2(sizeof(size_t)); ++i) {
		num |= num >> (1 << (i + 3));
	}
	return ++num;
}
