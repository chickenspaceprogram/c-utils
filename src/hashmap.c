#include <cu/hashmap.h>
#include <assert.h>
#define FILL_FACTOR 2
#define MIN_CAPACITY 16

static inline uint64_t next_pwr_2(uint64_t val)
{
	--val;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	val |= val >> 32;
	++val;
	return val;
}

int cu_hashmap_new(cu_hashmap *map, cu_alloc *alloc)
{
	map->arr = NULL;
	map->capacity = 0;
	map->nel = 0;
	map->alloc = alloc;
	return cu_siphash_init(&map->key);

}
void cu_hashmap_free(cu_hashmap *map)
{
	cu_freearray(map->arr, map->nel, sizeof(cu_hashmap_bucket), map->alloc);
}

static cu_hashmap_bucket *cu_hashmap_find_bucket(cu_hashmap *map, cu_str str)
{
	if (map->capacity == 0)
		return NULL;

	uint64_t index = cu_strhash(str, &map->key) & (map->capacity - 1);
	uint64_t inc = 0;
	for (; ; index = (index + ++inc) & (map->capacity - 1)) {
		assert(inc < map->capacity);
		if (map->arr[index].key.buf == NULL)
			return map->arr + index;

		if (cu_streq(map->arr[index].key, str))
			return map->arr + index;
	}
}

void *cu_hashmap_at(cu_hashmap *map, cu_str key)
{
	cu_hashmap_bucket *el = cu_hashmap_find_bucket(map, key);
	if (el == NULL)
		return NULL;
	
	if (el->key.buf == NULL) {
		return NULL;
	}
	
	return el->value;
}


void cu_hashmap_insert_unsafe(cu_hashmap *map, cu_str key, void *value)
{
	cu_hashmap_bucket *bucket = cu_hashmap_find_bucket(map, key);
	bucket->key = key;
	bucket->value = value;
	++map->nel;
}

int cu_hashmap_reserve(cu_hashmap *map, uint64_t nel)
{
	if (nel <= map->capacity / FILL_FACTOR) {
		return 0;
	}
	uint64_t new_capacity = next_pwr_2(nel * FILL_FACTOR);
	if (map->capacity == 0 && new_capacity < MIN_CAPACITY)
		new_capacity = MIN_CAPACITY;
	
	cu_hashmap new_map = {
		.arr = cu_allocarray(new_capacity, sizeof(cu_hashmap_bucket), map->alloc),
		.capacity = new_capacity,
		.nel = 0,
		.alloc = map->alloc,
		.key = map->key,
	};
	if (new_map.arr == NULL)
		return -1;
	memset(new_map.arr, 0, new_capacity * sizeof(cu_hashmap_bucket));
	
	cu_hashmap_iter it = cu_hashmap_begin(map);
	cu_hashmap_bucket *cur = NULL;
	while ((cur = cu_hashmap_next(&it)) != NULL) {
		cu_hashmap_insert_unsafe(&new_map, cur->key, cur->value);
	}

	cu_freearray(map->arr, map->capacity, sizeof(cu_hashmap_bucket), map->alloc);
	*map = new_map;
	return 0;
}

int cu_hashmap_insert(cu_hashmap *map, cu_str key, void *value)
{
	if (cu_hashmap_reserve(map, map->nel + 1) != 0)
		return -1;
	cu_hashmap_insert_unsafe(map, key, value);
	return 0;

}

cu_hashmap_bucket *cu_hashmap_next(cu_hashmap_iter *iter)
{
	if (iter->hmap->nel == 0) {
		return NULL;
	}
	while (iter->index < iter->hmap->capacity) {
		cu_hashmap_bucket *elem = iter->hmap->arr + iter->index;
		++iter->index;
		if (elem->key.buf != NULL)
			return elem;
	}
	return NULL;
}
