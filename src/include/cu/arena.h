#pragma once
#include <stddef.h>
#include <stdalign.h>
#include <cu/alloc.h>

struct cu_arena;
struct cu_arena_dyn;

struct cu_arena *cu_arena_new(size_t arena_size, struct cu_allocator *alloc);
// Allocates `amt` bytes of memory with an alignment of `align`.
// `align` must be a power of two.
void *cu_arena_aligned_alloc(size_t amt, size_t align, struct cu_arena *arena);

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
static inline void *cu_arena_alloc(size_t amt, struct cu_arena *arena)
{
	return cu_arena_aligned_alloc(amt, alignof(max_align_t), arena);
}
void cu_arena_free(struct cu_arena *arena, struct cu_allocator *alloc);

void cu_arena_cast(struct cu_allocator *alloc, struct cu_arena *arena);



struct cu_arena_dyn *cu_arena_dyn_new(size_t arena_size, struct cu_allocator *alloc);
void *cu_arena_dyn_aligned_alloc(size_t amt, size_t align, struct cu_arena_dyn *arena);
static inline void *cu_arena_dyn_alloc(size_t amt, struct cu_arena_dyn *arena)
{
	return cu_arena_dyn_aligned_alloc(amt, alignof(max_align_t), arena);
}
void cu_arena_dyn_free(struct cu_arena_dyn *arena);

void cu_arena_dyn_cast(struct cu_allocator *alloc, struct cu_arena_dyn *arena);

