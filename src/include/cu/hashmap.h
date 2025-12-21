#pragma once
#include <cu/string.h>
#include <cu/arena.h>

// This hashmap is primarily intended to map string keys to arbitrary values.
// Other usecases are uncommon for me.

// It's assumed that you'll store values in an arena or something.
// It should be secure even for maliciously-chosen keys, and uses quadratic
// probing.

typedef struct {
	cu_str key;
	void *value;
} cu_hm_bucket;

typedef struct {
	cu_hm_bucket *arr;
	uint64_t capacity;
	uint64_t nel;
	cu_alloc *alloc;
	cu_siphash_key key;
} cu_hm;

typedef struct {
	cu_hm *hmap;
	uint64_t index;
} cu_hm_iter;

int cu_hm_new(cu_hm *map, cu_alloc *alloc);
void cu_hm_free(cu_hm *map);

void *cu_hm_at(cu_hm *map, cu_str key);
static inline bool cu_hm_contains(cu_hm *map, cu_str key)
{
	return cu_hm_at(map, key) != NULL;
}
int cu_hm_insert(cu_hm *map, cu_str key, void *value);
int cu_hm_reserve(cu_hm *map, uint64_t nel);

static inline cu_hm_iter cu_hm_begin(cu_hm *map)
{
	return (cu_hm_iter){
		.hmap = map,
		.index = 0,
	};
}

cu_hm_bucket *cu_hm_next(cu_hm_iter *iter);

// copies the keys of the hashmap
typedef struct {
	cu_hm hm;
	cu_arena *name_arena;
} cu_hm_cpy;

static inline int
cu_hm_cpy_new(cu_hm_cpy *map, cu_alloc *alloc, size_t arena_blocksize)
{
	int retval = cu_hm_new(&map->hm, alloc);
	if (retval != 0)
		return retval;
	map->name_arena = cu_arena_new(arena_blocksize, alloc);
	if (map->name_arena == NULL)
		return -1;
	return 0;
}
static inline void cu_hm_cpy_free(cu_hm_cpy *map)
{
	cu_hm_free(&map->hm);
	cu_arena_free(map->name_arena);
}
static inline void *cu_hm_cpy_at(cu_hm_cpy *map, cu_str key)
{
	return cu_hm_at(&map->hm, key);
}
static inline bool cu_hm_cpy_contains(cu_hm_cpy *map, cu_str key)
{
	return cu_hm_contains(&map->hm, key);
}
static inline int cu_hm_cpy_insert(cu_hm_cpy *map, cu_str key, void *value)
{
	// allocating key separately
	uint8_t *new_key = cu_arena_alloc(key.len, map->name_arena);
	if (new_key == NULL)
		return -1;
	memcpy(new_key, key.buf, key.len);
	key.buf = new_key;
	return cu_hm_insert(&map->hm, key, value);
	
	
}
static inline int cu_hm_cpy_reserve(cu_hm_cpy *map, uint64_t nel)
{
	return cu_hm_reserve(&map->hm, nel);
}
static inline cu_hm_iter cu_hm_cpy_begin(cu_hm_cpy *map)
{
	return cu_hm_begin(&map->hm);
}


