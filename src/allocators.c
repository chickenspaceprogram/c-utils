#include <cu/allocators.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <cu/hashmap.h>
#include <string.h>

#ifdef CU_HAVE_MEMSET_EXPLICIT
#	define CU_ZEROSET(PTR, LEN) (void)memset_explicit(PTR, 0, LEN)
#elif defined(CU_HAVE_EXPLICIT_BZERO)
#	define CU_ZEROSET explicit_bzero
#elif defined(CU_HAVE_SECURE_ZERO_MEMORY)
#	include <Windows.h>
#	define CU_ZEROSET (void)SecureZeroMemory
#elif defined(CU_HAVE_MEMSET_S)
#	define CU_ZEROSET(PTR, LEN) (void)memset_s(PTR, LEN, 0, LEN)
#else
#	error "Need one of bzero_explicit, memset_explicit, memset_s, SecureZeroMemory for zeroing memory allocator"
#endif

#undef NDEBUG
#include <assert.h>

union voidptrunion {
	void *ptr;
	char bytes[sizeof(void *)];
};

struct hmap {
	CU_HASHMAP_TYPE(void *, size_t) map;
};

static inline size_t ptrhash(void *ptr)
{
	static_assert(1, "sizeof(size_t) on your platform is neither 4 nor 8.");
	union voidptrunion un = {.ptr = ptr};
	if (sizeof(size_t) == 8) {
		const uint64_t offset_basis = 0xcbf29ce484222325;
		const uint64_t fnv_prime = 0x00000100000001b3;
		uint64_t hash = offset_basis;
		for (size_t i = 0; i < sizeof(void *); ++i) {
			hash ^= un.bytes[i];
			hash *= fnv_prime;
		}
		return hash;
	}
	else if (sizeof(size_t) == 4) {
		const uint32_t offset_basis = 0x811c9dc5;
		const uint32_t fnv_prime = 0x01000193;
		uint32_t hash = offset_basis;
		for (size_t i = 0; i < sizeof(void *); ++i) {
			hash ^= un.bytes[i];
			hash *= fnv_prime;
		}
		return hash;
	}
	abort();
}

static inline int ptrcmp(const void *p1, const void *p2)
{
	if (p1 > p2) {
		return 1;
	}
	else if (p1 < p2) {
		return -1;
	}
	return 0;
}

static inline void *dummy_test_allocfn(size_t amount, void *ctx)
{
	void *newptr = malloc(amount);
	if (newptr == NULL)
		return NULL;
	struct hmap *map = ctx;
	int retval = cu_hashmap_insert(map->map, newptr, amount, NULL, ptrhash, ptrcmp);
	assert(retval == 0);
	return newptr;
}
static inline void dummy_test_free(void *mem, size_t amount, void *ctx)
{
	struct hmap *map = ctx;
	size_t *ptr = cu_hashmap_at(map->map, mem, ptrhash, ptrcmp);
	assert(ptr != NULL);
	assert(*ptr == amount);
	free(mem);
}

struct cu_allocator cu_get_dummy_test_alloc(void)
{
	struct hmap *map = malloc(sizeof(struct hmap));

	assert(map != NULL && "Failed to allocate dummy test alloc hashmap");
	int retval = cu_hashmap_new(map->map, NULL);
	assert(retval == 0 && "Failed to initialize dummy test alloc hashmap");
	struct cu_allocator dummy = {
		.alloc = dummy_test_allocfn,
		.free = dummy_test_free,
		.realloc = NULL,
		.ctx = map,
	};
	return dummy;
}
void cu_free_dummy_test_alloc(struct cu_allocator *alloc)
{
	cu_hashmap_delete(*(CU_HASHMAP_TYPE(void *, size_t) *)alloc->ctx, NULL);
	free(alloc->ctx);
}



static void zalloc_free(void *mem, size_t amount, void *ctx)
{
	CU_ZEROSET(mem, amount);
	cu_allocator_free(mem, amount, ctx);
}

struct cu_allocator cu_make_zalloc(struct cu_allocator *alloc)
{
	struct cu_allocator al = {
		.alloc = alloc->alloc,
		.free = zalloc_free,
		.realloc = NULL,
		.ctx = alloc,
	};
	return al;
}
