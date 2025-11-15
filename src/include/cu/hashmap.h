#pragma once
#include <cu/string.h>

// This hashmap is primarily intended to map string keys to arbitrary values.
// Other usecases are uncommon for me.

// It's assumed that you'll store values in an arena or something.
// It should be secure even for maliciously-chosen keys, and uses quadratic
// probing.

typedef struct {
	cu_str key;
	void *value;
} cu_hashmap_bucket;

typedef struct {
	cu_hashmap_bucket *arr;
	uint64_t capacity;
	uint64_t nel;
	cu_alloc *alloc;
	cu_siphash_key key;
} cu_hashmap;

typedef struct {
	cu_hashmap *hmap;
	uint64_t index;
} cu_hashmap_iter;

int cu_hashmap_new(cu_hashmap *map, cu_alloc *alloc);
void cu_hashmap_free(cu_hashmap *map);

void *cu_hashmap_at(cu_hashmap *map, cu_str key);
static inline bool cu_hashmap_contains(cu_hashmap *map, cu_str key)
{
	return cu_hashmap_at(map, key) != NULL;
}
int cu_hashmap_insert(cu_hashmap *map, cu_str key, void *value);
int cu_hashmap_reserve(cu_hashmap *map, uint64_t nel);

static inline cu_hashmap_iter cu_hashmap_begin(cu_hashmap *map)
{
	return (cu_hashmap_iter){
		.hmap = map,
		.index = 0,
	};
}

cu_hashmap_bucket *cu_hashmap_next(cu_hashmap_iter *iter);

