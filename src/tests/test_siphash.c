// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#define SIPHASH_C 2
#define SIPHASH_D 4
#include <cu/dbgassert.h>
#include "../siphash.c"
#include "vectors.h"

static cu_siphash_key SIPHASH_KEY_DEFAULT = {
	.key = {
		UINT64_C(0x0706050403020100),
		UINT64_C(0x0f0e0d0c0b0a0908),
	}
};

int main(void)
{
	uint8_t inbuf[64] = {0};
	for (size_t i = 0; i < 64; ++i) {
		inbuf[i] = i;
		uint64_t hash = cu_siphash_hash(&SIPHASH_KEY_DEFAULT, inbuf, i);
		uint64_t expected = parse_little_endian(vectors_sip64[i]);
		assert(hash == expected);
	}
}
