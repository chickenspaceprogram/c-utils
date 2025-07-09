// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once
#include <cu/alloc.h>

static const struct cu_allocator *STDALLOC = NULL;


// Creates a dummy allocator.
//
// If you're looking to use a cu_allocator, this is likely not what you want.
// You can just pass a NULL cu_allocator to any function that asks for one;
// this will make functions default to the libc allocator.
//
// The dummy allocator created by this function itself uses the libc allocator.
// It's effectively libc malloc that makes sure the correct sizes are passed to
// malloc and free.
//
// This dummy allocator is not itself thread-safe; calling alloc() and free()
// from multiple threads is likely to result in race conditions.
// However, different dummy allocators do not interfere with each other;
// so, if you want to run multiple threads, call this function multiple times
// and pass each dummy allocator you get to a different thread.
[[nodiscard("Discarding allocated dummy allocator")]]
struct cu_allocator cu_get_dummy_test_alloc(void);

// Frees any memory associated with a dummy test allocator.
//
// It's invalid to call this on any non-dummy test allocator.
void cu_free_dummy_test_alloc(struct cu_allocator *alloc);

// Creates an allocator that zeroes an allocated region prior to freeing it.
//
// The new allocator is only valid as long as the pointer to the original allocator is valid.
struct cu_allocator cu_make_zalloc(struct cu_allocator *alloc);
