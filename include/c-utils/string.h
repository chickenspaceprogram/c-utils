// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <c-utils/mem.h>

struct cu_str {
	char *string;
	size_t len;
};

// Moves the C-string pointed to by `string` into a cu_str.
// The cu_str does not take ownership of the c-string; for a version that takes ownership, try cu_str_clone.
struct cu_str cu_str_new(char *string)
{
	return (struct cu_str) { .string = string, .len = strlen(string) };
}

#define cu_str_clone(STR, ALLOC) _Generic ((STR),					\
	const char *: _Generic ((ALLOC),						\
		struct cu_arena: cu_str_clone_cstr_arena(STR, ALLOC),			\
		const struct cu_allocator: cu_str_clone_cstr_alloc(STR, ALLOC)),	\
	const struct cu_str *: _Generic ((ALLOC),					\
		struct cu_arena: cu_str_clone_cu_str_arena(STR, ALLOC),			\
		const struct cu_allocator: cu_str_clone_cu_str_alloc(STR, ALLOC))	\
)

struct cu_str cu_str_clone_cstr_arena(const char *string, struct cu_arena *arena);
struct cu_str cu_str_clone_cstr_alloc(const char *string, const struct cu_allocator *alloc);
struct cu_str cu_str_clone_cu_str_arena(const struct cu_str *string, struct cu_arena *arena);
struct cu_str cu_str_clone_cu_str_alloc(const struct cu_str *string, const struct cu_allocator *alloc);

// Only valid to call if you have created the cu_str with the given allocator.
static inline void cu_str_free(struct cu_str *string, const struct cu_allocator *alloc)
{
	cu_allocator_free(string->string, string->len, alloc);
}

// Takes a non-owning view of a cu_str in O(1) time.
// [start, end)
static inline struct cu_str cu_str_view(struct cu_str *string, size_t start, size_t end)
{
	return (struct cu_str) { .string = string->string + start, .len = end - start };
}

static inline size_t cu_strlen(const struct cu_str *string)
{
	return string->len;
}


// Concatenates the strings in `strings`, and returns the concatenated result.
// If passing in a `const struct cu_allocator` or `const struct cu_arena`, the third argument will be used to allocate the returned string
// if passing in a `void *`, the third parameter will be used as a buffer to store the returned string.
// It is assumed that the buffer is sized correctly to hold the concatenated string. 
// To see how much space the buffer will need, use cu_str_gather_size.

#define cu_str_gather(STRS, NSTRS, STORAGE) 							\
	_Generic ((STRS), const struct cu_str *: _Generic((NSTRS), size_t: _Generic((STORAGE),	\
		const struct cu_allocator *: cu_str_gather_alloc(STRS, NSTRS, STORAGE),		\
		struct cu_arena *: cu_str_gather_arena(STRS, NSTRS, STORAGE),			\
		void *: cu_str_gather_buf(STRS, NSTRS, STORAGE)					\
)))

struct cu_str cu_str_gather_alloc(const struct cu_str *strings, size_t nstrings, const struct cu_allocator *alloc);
struct cu_str cu_str_gather_arena(const struct cu_str *strings, size_t nstrings, struct cu_arena *arena);
struct cu_str cu_str_gather_buf(const struct cu_str *strings, size_t nstrings, void *buf); // assumes buf has enough space :)

size_t cu_str_gather_size(const struct cu_str *strings, size_t nstrings);

// Compares `string1` and `string2`, starting from the first character.
// If string1 < string2, a number less than 0 is returned.
// If string1 == string2, 0 is returned.
// If string1 > string2, a number greater than 0 is returned.
static inline int cu_str_cmp(const struct cu_str *string1, const struct cu_str *string2)
{
	size_t len = (string1->len > string2->len) ? string2->len : string1->len;
	return memcmp(string1->string, string2->string, len);
}

// Checks if the string is empty.
static inline bool cu_str_empty(const struct cu_str *string)
{
	return (cu_strlen(string) == 0);
}

// Searches `string` for an instance of `character`.
// If the character is found, its index will be returned.
// If not, the length of `string` will be returned.
static inline size_t cu_strchr(const struct cu_str *string, unsigned char character)
{
	char *item = memchr(string->string, character, string->len);
	if (item == NULL) {
		return string->len;
	}
	return item - string->string;
}


#define cu_strstr(HAYSTACK, NEEDLE) _Generic((HAYSTACK), const struct cu_str: _Generic((NEEDLE),	\
	const char *: cu_strstr_cstr(HAYSTACK, NEEDLE),							\
	const struct cu_str *: cu_strstr_cu_str(HAYSTACK, NEEDLE)					\
))

// Searches `string` for the substring `search_string`.
// If the substring is found, the index of the start of the substring will be returned.
// If not, the length of `string` will be returned.
size_t cu_strstr_cstr(const struct cu_str *string, const char *search_string);
size_t cu_strstr_cu_str(const struct cu_str *string, const struct cu_str *search_string);

static inline void cu_str_rev(struct cu_str *string)
{
	for (size_t i = 0; i < string->len / 2; ++i) {
		char temp = string->string[i];
		string->string[i] = string->string[string->len - i];
		string->string[string->len - i] = temp;
	}
}

// these just apply the toupper/tolower functions in ctype.h to every character
static inline void cu_str_toupper(struct cu_str *string)
{
	for (size_t i = 0; i < string->len; ++i) {
		string->string[i] = toupper(string->string[i]);
	}
}
static inline void cu_str_tolower(struct cu_str *string)
{
	for (size_t i = 0; i < string->len; ++i) {
		string->string[i] = tolower(string->string[i]);
	}
}

// see the manpage for ctype.h
// the following just apply the appropriately-named function there to every character in the string; if every character is as desired, 1 is returned, otherwise, 0 is returned.

#define CU_STR_MAKE_FN(NAME) static inline bool cu_str_##NAME(const struct cu_str *string)\
{\
	for (size_t i = 0; i < string->len; ++i) {\
		if (!NAME(string->string[i]))\
			return false;\
	}\
	return true;\
}

// all of format static inline bool cu_str_isalnum(const struct cu_str *string);
CU_STR_MAKE_FN(isalnum)
CU_STR_MAKE_FN(isalpha)
CU_STR_MAKE_FN(iscntrl)
CU_STR_MAKE_FN(isgraph)
CU_STR_MAKE_FN(islower)
CU_STR_MAKE_FN(isprint)
CU_STR_MAKE_FN(ispunct)
CU_STR_MAKE_FN(isspace)
CU_STR_MAKE_FN(isupper)
CU_STR_MAKE_FN(isxdigit)
CU_STR_MAKE_FN(isascii)
CU_STR_MAKE_FN(isblank)

#undef CU_STR_MAKE_FN
