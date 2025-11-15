// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once
#include <stddef.h>
#include <cu/alloc.h>
#include <cu/string.h>
#include <time.h>

// cu/cache.h
//
// A simple cache, intended mostly to be paired with an HTTP server but generic
// enough to be used in other places too.

typedef struct cu_cache_elem {
	cu_str key;
	cu_str val;
	struct timespec init_time;
	struct timespec max_alive_time;
} cu_cache_elem;

typedef struct {
	cu_cache_elem *elems_minheap;
	uint64_t nel;
	uint64_t max_size;
	cu_cache_elem **ptr_hashlist;
	cu_alloc *alloc;
	cu_siphash_key hashkey;
} cu_cache;

// Allocates the whole cache upfront
int cu_cache_new(cu_cache *cache, size_t max_nel, cu_alloc *alloc);
const cu_str *cu_cache_search(cu_cache *cache, cu_str key);
int cu_cache_add(cu_cache *cache, cu_str key, cu_str val, struct timespec max_alive_time);
void cu_cache_clear(cu_cache *cache);
void cu_cache_free(cu_cache *cache);
