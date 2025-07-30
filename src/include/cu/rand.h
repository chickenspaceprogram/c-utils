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
