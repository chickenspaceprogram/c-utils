// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/bitmanip.h>

#ifndef CU_HAVE_STDBIT

#if (SIZE_MAX == 0xFFFFUL)
#	define CU_SIZET_NBITS 16
#elif (SIZE_MAX == 0xFFFFFFFFUL)
#	define CU_SIZET_NBITS 32
#elif (SIZE_MAX == 0xFFFFFFFFFFFFFFFFUL)
#	define CU_SIZET_NBITS 64
#else
#	error "size_t on your platform is defined to be a weird size"
#endif


size_t cu_bit_ceil(size_t val)
{
	--val;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
#if CU_SIZET_NBITS >= 32
	val |= val >> 16;
#endif
#if CU_SIZET_NBITS == 64
	val |= val >> 32;
#endif
	++val;
	return val;
}


#endif

