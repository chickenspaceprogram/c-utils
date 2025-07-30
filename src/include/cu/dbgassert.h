// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once
#include <stdlib.h>
#include <stdio.h>

// Functions identically to the `assert` macro, except the assertion is always
// present regardless of whether `NDEBUG' is defined.
//
// If `arg == 0', the program is aborted, and a debugging message printed.
// Otherwise, nothing happens.
//
// This macro is intended for writing test cases.
//
// All other uses of assert() are best served by assert() itself; typically
// when just checking an invariant in normal code you don't want the check
// to happen in release builds.
#define dbgassert(ARG) do {\
	if (!(ARG)) {\
		fprintf(stderr, "%s:%d: %s: Assertion `%s' failed.\n", __FILE__, __LINE__, __func__, #ARG);\
		fflush(stderr);\
		abort();\
	}\
} while (0)
