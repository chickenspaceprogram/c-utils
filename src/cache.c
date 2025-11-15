#include <cu/cache.h>
#include <assert.h>
#define HASHLIST_SIZE_SHIFT 1

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

static inline uint64_t get_hashlist_sz(uint64_t max_size)
{
	return next_pwr_2(max_size) << HASHLIST_SIZE_SHIFT;
}

static cu_cache_elem **find_elem(cu_cache *cache, cu_str key)
{
	uint64_t hashlist_sz = get_hashlist_sz(cache->nel);
	uint64_t hashval = cu_siphash_hash(&cache->hashkey, key.buf, key.len);
	uint64_t index = hashval & (hashlist_sz - 1);
	uint64_t inc = 1;
	while (true) {
		assert(inc < hashlist_sz);
		if (cache->ptr_hashlist[index] == NULL)
			return cache->ptr_hashlist + index;

	if (cu_streq(cache->ptr_hashlist[index]->key, key))
			return cache->ptr_hashlist + index;

		index += inc++;
		index &= (hashlist_sz - 1);
	}
}

static inline void delete_cache_entry(cu_cache_elem **entry);
static struct timespec add_timespecs(struct timespec tm1, struct timespec tm2);
static int cmp_timespecs(struct timespec tm1, struct timespec tm2);

int cu_cache_new(cu_cache *cache, size_t max_nel, cu_alloc *alloc)
{
	// generate key
	int res = cu_siphash_init(&cache->hashkey);
	if (res == -1)
		return -1;

	// alloc space for minheap, bufs
	cache->elems_minheap = cu_allocarray(max_nel, sizeof(cu_cache_elem), alloc);
	if (cache->elems_minheap == NULL)
		return -1;
	cache->ptr_hashlist = cu_allocarray(get_hashlist_sz(max_nel), sizeof(cu_cache_elem *), alloc);
	if (cache->ptr_hashlist == NULL) {
		cu_freearray(cache->elems_minheap, max_nel, sizeof(cu_cache_elem), alloc);
		return -1;
	}
	// need to make a safe version of memset
	memset(cache->ptr_hashlist, 0, sizeof(cu_cache_elem *) * max_nel);
	cache->nel = 0;
	cache->max_size = max_nel;
	cache->alloc = alloc;
	return 0;
}

const cu_str *cu_cache_search(cu_cache *cache, cu_str key)
{
	cu_cache_elem **el = find_elem(cache, key);
	if (*el == NULL)
		return NULL;
	struct timespec cur_time;
	int res = timespec_get(&cur_time, TIME_UTC);
	if (res == 0)
		return NULL;
	struct timespec end_tm = add_timespecs((*el)->init_time, (*el)->max_alive_time);
	if (cmp_timespecs(cur_time, end_tm) < 0) {
		delete_cache_entry(el);
		return NULL;
	}
	return &(*el)->val;
}
