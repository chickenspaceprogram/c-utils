// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0


// This file contains some redefinitions of a few C23 features that I found
// useful. They are not available on all compilers so more limited fallback
// versions are provided here.

#pragma once
#include <stddef.h>
#include <stdint.h>

// cu_bit_ceil is analogous to stdc_bit_ceil, except it only takes size_t
// arguments
#ifdef CU_HAVE_STDBIT
#	include <stdbit.h>

#	define cu_bit_ceil stdc_bit_ceil

#else

size_t cu_bit_ceil(size_t val);

#endif

// cu_ckd_mul is identical to ckd_mul in C23, except RESULT cannot point to A
// or B.
#ifdef CU_HAVE_CKDINT
#include <stdckdint.h>

#define cu_ckd_mul ckd_mul

#else

#define cu_ckd_mul(RESULT, A, B) (\
	*(RESULT) = (A) * (B),\
	((A) != 0 && ((A) * (B)) / (A) != (B))\
)

#endif
