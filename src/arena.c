#include <cu/arena.h>
#include <stdint.h>
#include <assert.h>

#define IS_PWR_2(V) ((V) && !((V) & ((V) - 1)))

struct cu_arena {
	size_t bufsize;
	size_t index;
	alignas(alignof(max_align_t)) uint8_t buf[];
};
struct cu_arena_dyn {
	struct cu_arena_elem *first;
	struct cu_allocator *alloc;
	size_t default_block_size;
	size_t index;
	alignas(alignof(max_align_t)) uint8_t buf[];
};
struct cu_arena_elem {
	size_t bufsize;
	size_t index;
	struct cu_arena_elem *next;
	alignas(alignof(max_align_t)) uint8_t buf[];
};

static_assert(alignof(struct cu_arena) <= alignof(max_align_t), "Your platform aligns structs weirdly.");
static_assert(alignof(struct cu_arena_dyn) <= alignof(max_align_t), "Your platform aligns structs weirdly.");
static_assert(alignof(struct cu_arena_elem) <= alignof(max_align_t), "Your platform aligns structs weirdly.");

static size_t align_index(uintptr_t baseptr, size_t cur_index, size_t align)
{
	assert(IS_PWR_2(align) && "Alignment must be a power of 2");
	size_t mod = (baseptr + cur_index) & (align - 1);
	if (mod == 0)
		return cur_index;
	size_t extra = align - mod;
	assert(((baseptr + cur_index + extra) & (align - 1)) == 0 && "Something is misaligned...");
	return cur_index + extra;
}

struct cu_arena *cu_arena_new(size_t arena_size, struct cu_allocator *alloc)
{
	struct cu_arena *arena = cu_allocator_alloc(sizeof(struct cu_arena) + arena_size, alloc);
	if (arena == NULL)
		return NULL;
	arena->bufsize = arena_size;
	arena->index = 0;
	return arena;
}

void *cu_arena_aligned_alloc(size_t amt, size_t align, struct cu_arena *arena)
{
	assert(IS_PWR_2(align));
	size_t base_index = align_index((uintptr_t)arena->buf, arena->index, align);
	if (base_index + amt > arena->bufsize) {
		return NULL;
	}
	void *elem = arena->buf + base_index;
	arena->index = base_index + amt;
	return elem;
}

void cu_arena_free(struct cu_arena *arena, struct cu_allocator *alloc)
{
	cu_allocator_free(arena, arena->bufsize + sizeof(struct cu_arena), alloc);
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
	if (block_size < alignof(max_align_t))
		block_size = alignof(max_align_t);
	struct cu_arena_dyn *arena = cu_allocator_alloc(sizeof(struct cu_arena_dyn) + block_size, alloc);
	if (arena == NULL)
		return NULL;
	arena->default_block_size = block_size;
	arena->index = 0;
	arena->first = NULL;
	arena->alloc = alloc;
	return arena;
}

static size_t max_reqd_size(size_t amt, size_t align)
{
	if (align <= alignof(max_align_t))
		return amt;
	return amt + align - alignof(max_align_t);
}
void *cu_arena_dyn_aligned_alloc(size_t amt, size_t align, struct cu_arena_dyn *arena)
{
	size_t reqd_size = max_reqd_size(amt, align);
	if (reqd_size > arena->default_block_size) {
		struct cu_arena_elem *new_block = cu_allocator_alloc(reqd_size + sizeof(struct cu_arena_elem), arena->alloc);
		if (new_block == NULL)
			return NULL;

		size_t start_index = align_index((uintptr_t)new_block->buf, 0, align);

		new_block->index = start_index + amt;
		new_block->bufsize = reqd_size;
		new_block->next = arena->first;
		arena->first = new_block;
		return new_block->buf + start_index;
	}
	size_t base_index = align_index((uintptr_t)arena->buf, arena->index, align);
	if (base_index + amt <= arena->default_block_size) {
		arena->index = base_index + amt;
		return arena->buf + base_index;
	}

	struct cu_arena_elem *elem = arena->first;
	while (elem != NULL) {
		base_index = align_index((uintptr_t)elem->buf, elem->index, align);
		if (base_index + amt <= elem->bufsize) {
			elem->index = base_index + amt;
			return elem->buf + base_index;
		}
		elem = elem->next;
	}
	struct cu_arena_elem *new_first = cu_allocator_alloc(sizeof(struct cu_arena_elem) + arena->default_block_size, arena->alloc);
	if (new_first == NULL)
		return NULL;
	new_first->bufsize = arena->default_block_size;
	new_first->index = align_index((uintptr_t)elem->buf, 0, align);
	void *retval = new_first->buf + new_first->index;
	new_first->index += amt;
	new_first->next = arena->first;
	arena->first = new_first;
	return retval;
}
void cu_arena_dyn_free(struct cu_arena_dyn *arena)
{
	struct cu_arena_elem *elem = arena->first;
	while (elem != NULL) {
		struct cu_arena_elem *tmp = elem;
		elem = elem->next;
		cu_allocator_free(tmp, sizeof(struct cu_arena_elem) + tmp->bufsize, arena->alloc);
	}
	cu_allocator_free(arena, sizeof(struct cu_arena_dyn) + arena->default_block_size, arena->alloc);
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

