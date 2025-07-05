#include <cu/arena.h>

#if !__has_c_attribute(gnu::may_alias)
#error "cu_arena requires gnu::may_alias attribute"
#endif


struct cu_arena {
	size_t bufsize;
	size_t index;
	[[gnu::may_alias]] max_align_t buf[];
};
struct cu_arena_dyn {
	struct cu_arena_elem *first;
	struct cu_allocator *alloc;
	size_t default_block_size;
	size_t index;
	[[gnu::may_alias]] max_align_t buf[];
};
struct cu_arena_elem {
	size_t bufsize;
	size_t index;
	struct cu_arena_elem *next;
	[[gnu::may_alias]] max_align_t buf[];
};


static size_t get_index_increment(size_t nbytes)
{
	size_t mod = nbytes % sizeof(max_align_t);
	if (mod != 0)
		mod = 1;
	return nbytes / sizeof(max_align_t) + mod;
}


struct cu_arena *cu_arena_new(size_t arena_size, struct cu_allocator *alloc)
{
	struct cu_arena *arena = cu_allocator_alloc(sizeof(struct cu_arena) + get_index_increment(arena_size) * sizeof(max_align_t), alloc);
	if (arena == NULL)
		return NULL;
	arena->bufsize = get_index_increment(arena_size);
	arena->index = 0;
	return arena;
}

void *cu_arena_alloc(size_t amt, struct cu_arena *arena)
{
	size_t index_inc = get_index_increment(amt);
	if (index_inc + arena->index > arena->bufsize) {
		return NULL;
	}
	void *elem = arena->buf + arena->index;
	arena->index += index_inc;
	return elem;
}

void cu_arena_free(struct cu_arena *arena, struct cu_allocator *alloc)
{
	cu_allocator_free(alloc, arena->bufsize * sizeof(max_align_t) + sizeof(struct cu_arena), alloc);
}

static void *arena_allocator_alloc(size_t amount, void *ctx)
{
	struct cu_arena *arena = ctx;
	return cu_arena_alloc(amount, arena);
}

void cu_arena_cast(struct cu_allocator *alloc, struct cu_arena *arena)
{
	alloc->alloc = arena_allocator_alloc;
	alloc->free = NULL;
	alloc->realloc = NULL;
	alloc->ctx = arena;
}

struct cu_arena_dyn *cu_arena_dyn_new(size_t block_size, struct cu_allocator *alloc)
{
	struct cu_arena_dyn *arena = cu_allocator_alloc(sizeof(struct cu_arena_dyn) + get_index_increment(block_size) * sizeof(max_align_t), alloc);
	if (arena == NULL)
		return NULL;
	arena->default_block_size = get_index_increment(block_size);
	arena->index = 0;
	arena->first = NULL;
	arena->alloc = alloc;
	return arena;
}
void *cu_arena_dyn_alloc(size_t amt, struct cu_arena_dyn *arena)
{
	size_t alloc_size = get_index_increment(amt);
	if (arena->index + alloc_size <= arena->default_block_size) {
		void *ptr = arena->buf + arena->index;
		arena->index += alloc_size;
		return ptr;
	}
	struct cu_arena_elem *elem = arena->first;
	while (elem != NULL) {
		if (elem->index + alloc_size <= elem->bufsize) {
			void *ptr = elem->buf + elem->index;
			elem->index += alloc_size;
			return ptr;
		}
		elem = elem->next;
	}
	size_t new_bufsize = alloc_size > arena->default_block_size ? alloc_size : arena->default_block_size;
	struct cu_arena_elem *new_first = cu_allocator_alloc(sizeof(struct cu_arena_elem) + new_bufsize, arena->alloc);
	if (new_first == NULL)
		return NULL;
	new_first->bufsize = new_bufsize;
	new_first->index = alloc_size;
	new_first->next = arena->first;
	arena->first = new_first;
	return new_first->buf;
}
void cu_arena_dyn_free(struct cu_arena_dyn *arena)
{
	struct cu_arena_elem *elem = arena->first;
	while (elem != NULL) {
		struct cu_arena_elem *tmp = elem;
		elem = elem->next;
		cu_allocator_free(tmp, sizeof(struct cu_arena_elem) + tmp->bufsize * sizeof(max_align_t), arena->alloc);
	}
	cu_allocator_free(arena, sizeof(struct cu_arena_dyn) + sizeof(max_align_t) * arena->default_block_size, arena->alloc);
}

static void *arena_dyn_alloc(size_t amount, void *ctx)
{
	struct cu_arena_dyn *arena = ctx;
	return cu_arena_dyn_alloc(amount, arena);
}

void cu_arena_dyn_cast(struct cu_allocator *alloc, struct cu_arena_dyn *arena)
{
	alloc->alloc = arena_dyn_alloc;
	alloc->free = NULL;
	alloc->realloc = NULL;
	alloc->ctx = arena;
}

