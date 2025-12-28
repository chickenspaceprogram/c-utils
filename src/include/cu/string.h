#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <cu/alloc.h>
#include <cu/siphash.h>


// The most barebones string library possible.
//
// Unfortunately most C string functions require null-terminators.

typedef struct {
	uint8_t *buf;
	size_t len;
} cu_str;

#define CU_NIL_STR (cu_str){.buf = NULL, .len = 0}

static inline cu_str cu_str_from_cstr(char *cstr)
{
	return (cu_str){
		.buf = (uint8_t *)cstr,
		.len = strlen(cstr),
	};
}
static inline uint8_t *cu_str_find(cu_str str, uint8_t chr)
{
	return memchr(str.buf, chr, str.len);
}
static inline int cu_str_cmp(cu_str s1, cu_str s2)
{
	assert(s1.buf != NULL && s2.buf != NULL && "Cannot compare a null string");
	size_t num_to_cmp = s1.len > s2.len ? s2.len : s1.len;
	int res = memcmp(s1.buf, s2.buf, num_to_cmp);
	if (res == 0) {
		if (s1.len > s2.len) {
			res = 1;
		}
		if (s1.len < s2.len) {
			res = -1;
		}
	}
	return res;
}
static inline bool cu_str_eq(cu_str s1, cu_str s2)
{
	assert(s1.buf != NULL && s2.buf != NULL && "Cannot compare a null string");
	if (s1.len != s2.len) {
		return false;
	}
	if (memcmp(s1.buf, s2.buf, s1.len) == 0) {
		return true;
	}
	return false;
}

static inline uint64_t cu_str_hash(cu_str str, const cu_siphash_key *key)
{
	assert(str.buf != NULL && "Cannot hash a null string");
	return cu_siphash_hash(key, str.buf, str.len);
}

static inline cu_str cu_str_substr(cu_str str, size_t start, size_t end)
{
	assert(str.buf != NULL && "Cannot get a substring from a null string");
	assert(end <= str.len && start < end && "Invalid bounds");
	str.buf += start;
	str.len = end - start;
	return str;
}
static inline cu_str cu_str_rmprefix(cu_str str, size_t n_to_remove)
{
	assert(str.buf != NULL && "Cannot get a substring from a null string");
	assert(n_to_remove <= str.len && "Removed too many characters from string");
	str.buf += n_to_remove;
	str.len -= n_to_remove;
	return str;
}
static inline cu_str cu_str_rmsuffix(cu_str str, size_t n_to_remove)
{
	assert(str.buf != NULL && "Cannot get a substring from a null string");
	assert(n_to_remove <= str.len && "Removed too many characters from string");
	str.len -= n_to_remove;
	return str;
}
