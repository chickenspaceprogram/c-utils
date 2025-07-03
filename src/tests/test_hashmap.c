// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/hashmap.h>
#include <stdint.h>
#include <assert.h>

static int cmp_str(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}


static size_t hash_str(const char *cstr)
{
	if (sizeof(size_t) == 8) {
		const uint64_t offset_basis = 0xcbf29ce484222325;
		const uint64_t fnv_prime = 0x00000100000001b3;
		uint64_t hash = offset_basis;
		for (size_t i = 0; cstr[i] != '\0'; ++i) {
			hash ^= cstr[i];
			hash *= fnv_prime;
		}
		return hash;

	}
	else if (sizeof(size_t) == 4) {
		const uint32_t offset_basis = 0x811c9dc5;
		const uint32_t fnv_prime = 0x01000193;
		uint32_t hash = offset_basis;
		for (size_t i = 0; cstr[i] != '\0'; ++i) {
			hash ^= cstr[i];
			hash *= fnv_prime;
		}
		return hash;
	}
	return 0;
}

static void test_hashmap(struct cu_allocator *dummy_test_alloc)
{
	const char *keys[] = {
		"key1",
		"key2",
		"key3",
		"key4",
		"key5",
		"key6",
		"key7",
		"key8",
		"key9",
		"key10",
		"key11",
		"key12",
		"key13",
		"key14",
		"key15",
		"key16",
	};
	int vals[] = {
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		8,
		9,
		10,
		11,
		12,
		13,
		14,
		15,
		16,
	};

	CU_HASHMAP_TYPE(const char *, int) hm;
	assert(cu_hashmap_new(hm, dummy_test_alloc) == 0);
	for (int i = 0; i < 16; ++i) {
		assert(cu_hashmap_at(hm, keys[i], hash_str, cmp_str) == NULL);
	}
	for (int i = 0; i < 16; ++i) {
		assert(cu_hashmap_insert(hm, keys[i], vals[i], dummy_test_alloc, hash_str, cmp_str) == 0);
	}
	for (int i = 0; i < 16; ++i) {
		const int *retval = cu_hashmap_at(hm, keys[i], hash_str, cmp_str);
		assert(retval != NULL);
		assert(*retval == vals[i]);
	}
	cu_hashmap_remove(hm, "key1", hash_str, cmp_str);
	cu_hashmap_remove(hm, "key4", hash_str, cmp_str);
	cu_hashmap_remove(hm, "key10", hash_str, cmp_str);
	cu_hashmap_remove(hm, "key12", hash_str, cmp_str);
	cu_hashmap_remove(hm, "key15", hash_str, cmp_str);
	bool foundflags[16] = {0};
	bool targetflags[16] = {false, true, true, false, true, true, true, true, true, false, true, false, true, true, false, true};
	CU_HASHMAP_BUCKETTYPE(hm) *bucket = NULL;
	CU_HASHMAP_ITERTYPE(hm) iter;
	cu_hashmap_new_iter(iter, hm);
	do {
		bucket = cu_hashmap_iter_next(iter);
		if (bucket != NULL) {
			assert(bucket->val >= 1 && bucket->val <= 16);
			foundflags[bucket->val - 1] = true; 
			assert(strcmp(keys[bucket->val - 1], bucket->key) == 0);
		}
	} while (bucket != NULL);
	for (int i = 0; i < 16; ++i) {
		assert(foundflags[i] == targetflags[i]);
	}
	cu_hashmap_delete(hm, dummy_test_alloc);
}

int main(void)
{
	struct cu_allocator alloc = cu_get_dummy_test_alloc();
	test_hashmap(&alloc);
	cu_free_dummy_test_alloc(&alloc);
}
