// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef CU_HAVE_STDBIT
#include <stdbit.h>


// use C23 if possible
#define cu_bit_ceil stdc_bit_ceil
#else

// otherwise, fallback to a function that uses the preprocessor to find the max value of size_t in order to validly bit ceil
size_t cu_bit_ceil(size_t val);
#endif
