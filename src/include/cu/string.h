#pragma once
#include <stdint.h>
#include <stddef.h>
#include <cu/alloc.h>
#include <string.h>
#include <stdbool.h>
#include <cu/siphash.h>


// The most barebones string library possible.
//
// Unfortunately most C string functions require null-terminators.

typedef struct {
	uint8_t *buf;
	size_t len;
} cu_string_view;

static inline cu_string_view cu_cstr_cast(char *cstr)
{
	return (cu_string_view){
		.buf = (uint8_t *)cstr,
		.len = strlen(cstr),
	};
}
static inline uint8_t *cu_strchr(cu_string_view str, uint8_t chr)
{
	return memchr(str.buf, chr, str.len);
}
static inline int cu_strcmp(cu_string_view s1, cu_string_view s2)
{
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
static inline bool cu_streq(cu_string_view s1, cu_string_view s2)
{
	if (s1.len != s2.len) {
		return false;
	}
	if (memcmp(s1.buf, s2.buf, s1.len) == 0) {
		return true;
	}
	return false;
}

static inline uint64_t cu_strhash(cu_string_view str, const cu_siphash_key *key)
{
	return cu_siphash_hash(key, str.buf, str.len);
}
