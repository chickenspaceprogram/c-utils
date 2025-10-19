// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once
#include <stdint.h>
#include <stddef.h>


// An implementation of the SipHash-1-3 algorithm.
//
// This is primarily intended for usage in hashmaps which allow user-submitted
// keys.
//
// Usage of non-cryptographically-secure hash functions (such as FNV) in
// hashmaps can lead to HashDoS attacks if attackers can submit arbitrary keys.
//
// CUtils reserves the right to switch from SipHash-1-3 to another algorithm
// suitable for usage in hashmaps in future.


typedef struct {
	uint64_t key[2];
} cu_siphash_key;

#define CU_SIPHASH_KEYSIZE sizeof(cu_siphash_key)

// Initializes a `cu_siphash_key` with cryptographically-secure random bytes.
//
// The bytes are retrieved from your system's randomness source.
//
// This function is potentially not the most efficient on some randomness
// backends as it does not attempt to buffer random bits or seed a global
// random number generator.
//
// As such, it is also valid to initialize a `cu_siphash_key` yourself with
// cryptographically-secure random bytes from another source.
// Use the `cu_siphash_init_from_bytes' function to do this.
//
// Returns 0 on success, -1 on failure
int cu_siphash_init(cu_siphash_key *key);


// Initializes a `cu_siphash_key` with cryptographically-secure random bytes.
//
// The bytes are provided by the user. There must be at least
// `CU_SIPHASH_KEYSIZE' bytes.
//
// If you don't wish to provide random bytes, `cu_siphash_init' will obtain
// them from the system randomness source automatically.
//
// This API is provided in cases where it is convenient or more performant
// to initialize the key from a different randomness source, such as a
// userspace CSPRNG, or a buffer of already-obtained random bytes from the
// system.
void cu_siphash_init_from_bytes(cu_siphash_key *key, uint8_t *bytes);

// Always succeeds
uint64_t cu_siphash_hash(
	const cu_siphash_key *key,
	const uint8_t *buf,
	size_t size
);

