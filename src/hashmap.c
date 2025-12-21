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

int cu_hm_new(cu_hm *map, cu_alloc *alloc)
{
	map->arr = NULL;
	map->capacity = 0;
	map->nel = 0;
	map->alloc = alloc;
	return cu_siphash_init(&map->key);

}
void cu_hm_free(cu_hm *map)
{
	cu_freearray(map->arr, map->nel, sizeof(cu_hm_bucket), map->alloc);
}

static cu_hm_bucket *cu_hm_find_bucket(cu_hm *map, cu_str str)
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

void *cu_hm_at(cu_hm *map, cu_str key)
{
	cu_hm_bucket *el = cu_hm_find_bucket(map, key);
	if (el == NULL)
		return NULL;
	
	if (el->key.buf == NULL) {
		return NULL;
	}
	
	return el->value;
}


void cu_hm_insert_unsafe(cu_hm *map, cu_str key, void *value)
{
	cu_hm_bucket *bucket = cu_hm_find_bucket(map, key);
	bucket->key = key;
	bucket->value = value;
	++map->nel;
}

int cu_hm_reserve(cu_hm *map, uint64_t nel)
{
	if (nel <= map->capacity / FILL_FACTOR) {
		return 0;
	}
	uint64_t new_capacity = next_pwr_2(nel * FILL_FACTOR);
	if (map->capacity == 0 && new_capacity < MIN_CAPACITY)
		new_capacity = MIN_CAPACITY;
	
	cu_hm new_map = {
		.arr = cu_allocarray(new_capacity, sizeof(cu_hm_bucket), map->alloc),
		.capacity = new_capacity,
		.nel = 0,
		.alloc = map->alloc,
		.key = map->key,
	};
	if (new_map.arr == NULL)
		return -1;
	memset(new_map.arr, 0, new_capacity * sizeof(cu_hm_bucket));
	
	cu_hm_iter it = cu_hm_begin(map);
	cu_hm_bucket *cur = NULL;
	while ((cur = cu_hm_next(&it)) != NULL) {
		cu_hm_insert_unsafe(&new_map, cur->key, cur->value);
	}

	cu_freearray(map->arr, map->capacity, sizeof(cu_hm_bucket), map->alloc);
	*map = new_map;
	return 0;
}

int cu_hm_insert(cu_hm *map, cu_str key, void *value)
{
	if (cu_hm_reserve(map, map->nel + 1) != 0)
		return -1;
	cu_hm_insert_unsafe(map, key, value);
	return 0;

}

cu_hm_bucket *cu_hm_next(cu_hm_iter *iter)
{
	if (iter->hmap->nel == 0) {
		return NULL;
	}
	while (iter->index < iter->hmap->capacity) {
		cu_hm_bucket *elem = iter->hmap->arr + iter->index;
		++iter->index;
		if (elem->key.buf != NULL)
			return elem;
	}
	return NULL;
}
