// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#pragma once
#include <stddef.h>
#include <stdalign.h>
#include <cu/alloc.h>

typedef struct cu_arena_fixed cu_arena_fixed;
typedef struct cu_arena cu_arena;

cu_arena_fixed *cu_arena_fixed_new(size_t arena_size, cu_alloc *alloc);
// Allocates `amt` bytes of memory with an alignment of `align`.
// `align` must be a power of two.
void *cu_arena_fixed_aligned_alloc(size_t amt, size_t align, cu_arena_fixed *arena);

// Only valid when allocating space for types with "fundamental alignment" as
// defined by the C standard! (malloc also has this limitation)
//
// These are:
// - basic types
// - enumerated types
// - pointer types
// - arrays of types w/ fundamental alignment
// - structures/unions whose component types have fundamental alignment
//
// If a type doesn't have fundamental alignment (for example, you used _Alignas
// on it), use cu_arena_aligned_alloc.
//
// cu_arena_aligned_alloc is also beneficial if you need to allocate multiple
// types with small alignments consecutive to each other.
static inline void *cu_arena_fixed_alloc(size_t amt, cu_arena_fixed *arena)
{
	return cu_arena_fixed_aligned_alloc(amt, alignof(max_align_t), arena);
}
void cu_arena_fixed_free(cu_arena_fixed *arena, cu_alloc *alloc);

void cu_arena_fixed_cast(cu_alloc *alloc, cu_arena_fixed *arena);



cu_arena *cu_arena_new(size_t arena_size, cu_alloc *alloc);
void *cu_arena_aligned_alloc(size_t amt, size_t align, cu_arena *arena);
static inline void *cu_arena_alloc(size_t amt, cu_arena *arena)
{
	return cu_arena_aligned_alloc(amt, alignof(max_align_t), arena);
}
void cu_arena_free(cu_arena *arena);

void cu_arena_cast(cu_alloc *alloc, cu_arena *arena);

