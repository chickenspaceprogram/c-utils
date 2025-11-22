#include <cu/cache.h>
#include <assert.h>
#define HASHLIST_SIZE_SHIFT 1

// NEVER MIND FUCK MY LIFE I GUESS
// I NEED TO MAKE THIS A LINKED LIST HASHMAP
// BECAUSE IM GONNA BE ADDING AND REMOVING SHIT CONSTANTLY AND NEVER RESIZING
// FJDSALFJSDL

static cu_cache_elem DELETED_ELEM_OBJ;

// When elements are deleted, the pointer in the spot they occupied in the hashlist is set to this value
// this guarantees a nonnull pointer that is still a testable value
static cu_cache_elem *const DELETED_ELEM_FLAG = &DELETED_ELEM_OBJ;

#define BILLION 1000000000

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

typedef struct cu_cache_searchres {
	cu_cache_elem **item;
	cu_cache_elem **fst_empty_spot;
} cu_cache_searchres;

cu_cache_elem **search_elem(cu_cache *cache, cu_str key)
{
	uint64_t hashlist_sz = get_hashlist_sz(cache->nel);
	uint64_t hashval = cu_siphash_hash(&cache->hashkey, key.buf, key.len);
	uint64_t index = hashval & (hashlist_sz - 1);
	uint64_t inc = 1;
	cu_cache_elem **fst_empty_spot = NULL;
	while (true) {
		assert(inc < hashlist_sz);
		if (cache->ptr_hashlist[index] == NULL)
			return cache->ptr_hashlist + index;

		if (cache->ptr_hashlist[index] == DELETED_ELEM_FLAG) {
			if (fst_empty_spot == NULL)
				fst_empty_spot = cache->ptr_hashlist + index;

			goto loopend;
		}

		if (cu_streq(cache->ptr_hashlist[index]->key, key)) {
			if (fst_empty_spot == NULL)
				return cache->ptr_hashlist + index;

			*fst_empty_spot = cache->ptr_hashlist[index];
			cache->ptr_hashlist[index] = DELETED_ELEM_FLAG;
			return fst_empty_spot;
		}

		loopend:
		index += inc++;
		index &= (hashlist_sz - 1);
	}
}

static inline uint64_t get_lchild(uint64_t parent)
{
	return parent * 2 + 1;
}

static inline uint64_t get_rchild(uint64_t parent)
{
	return parent * 2 + 2;
}

static inline uint64_t get_parent(uint64_t child)
{
	--child;
	child -= child % 2;
	return child / 2;
}

static inline void swap_with_parent(cu_cache *cache, uint64_t index)
{
	struct cu_cache_elem tmp = cache->elems_minheap[index];
	uint64_t parent_ind = get_parent(index);
	cache->elems_minheap[index] = cache->elems_minheap[parent_ind];
	cache->elems_minheap[parent_ind] = tmp;

	*(cache->elems_minheap[parent_ind].hashlist_entry) = cache->elems_minheap + parent_ind;
	*(cache->elems_minheap[index].hashlist_entry) = cache->elems_minheap + index;
}

static inline void delete_top_entry(cu_cache *cache);

static inline void delete_cache_entry(cu_cache *cache, uint64_t index)
{
	while (index > 0) {
		swap_with_parent(cache, index);
		index = get_parent(index);
	}
	delete_top_entry(cache);
}

static struct timespec add_timespecs(struct timespec tm1, struct timespec tm2)
{
	tm1.tv_sec += tm2.tv_sec;
	uint32_t tm1_nsec = tm1.tv_nsec;
	uint32_t tm2_nsec = tm1.tv_nsec;
	uint32_t res_nsec = tm1_nsec + tm2_nsec;
	if (res_nsec >= BILLION) {
		res_nsec -= BILLION;
		++tm1.tv_sec;
	}
	tm1.tv_nsec = res_nsec;
	return tm1;
}

static int cmp_timespecs(struct timespec tm1, struct timespec tm2)
{
	if (tm1.tv_sec > tm2.tv_sec)
		return 1;
	if (tm1.tv_sec < tm2.tv_sec)
		return -1;
	
	if (tm1.tv_nsec > tm2.tv_nsec)
		return 1;
	if (tm1.tv_nsec < tm2.tv_nsec)
		return -1;
	return 0;
}

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
	cu_cache_searchres res = find_elem(cache, key);
	cu_cache_elem *elptr = NULL;
	if (*res.item == NULL)
		return NULL;
	if (res.fst_empty_spot != NULL) {
		*res.fst_empty_spot = *res.item;
		*res.item = DELETED_ELEM_FLAG;
		elptr = *res.fst_empty_spot;
	}
	if (*el == NULL || *el == DELETED_ELEM_FLAG)
		return NULL;
	struct timespec cur_time;
	int res = timespec_get(&cur_time, TIME_UTC);
	if (res == 0)
		return NULL;
	struct timespec end_tm = add_timespecs((*el)->init_time, (*el)->max_alive_time);
	if (cmp_timespecs(cur_time, end_tm) < 0) {
		delete_cache_entry(cache, *el - cache->elems_minheap);
		return NULL;
	}
	return &(*el)->val;
}
