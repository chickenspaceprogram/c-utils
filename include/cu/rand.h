// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once
#include <stddef.h>
#include <stdint.h>

// Writes `nbytes` cryptographically-secure random bytes to `buf`.
//
// Returns 0 on success, -1 on error.
//
// Writing more than CU_RAND_MAX bytes will result in the program
// aborting.

#define CU_RAND_MAX 256

int cu_rand_bytes(uint8_t *buf, size_t nbytes);
