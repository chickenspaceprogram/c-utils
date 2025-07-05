#pragma once
#include <stddef.h>
#include <cu/alloc.h>

struct cu_arena;
struct cu_arena_dyn;

struct cu_arena *cu_arena_new(size_t arena_size, struct cu_allocator *alloc);
void *cu_arena_alloc(size_t amt, struct cu_arena *arena);
void cu_arena_free(struct cu_arena *arena, struct cu_allocator *alloc);

void cu_arena_cast(struct cu_allocator *alloc, struct cu_arena *arena);



struct cu_arena_dyn *cu_arena_dyn_new(size_t arena_size, struct cu_allocator *alloc);
void *cu_arena_dyn_alloc(size_t amt, struct cu_arena_dyn *arena);
void cu_arena_dyn_free(struct cu_arena_dyn *arena);

void cu_arena_dyn_cast(struct cu_allocator *alloc, struct cu_arena_dyn *arena);

