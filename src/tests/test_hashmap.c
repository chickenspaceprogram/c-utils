// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <stdint.h>
#include <assert.h>
#include <cu/hashmap.h>
#include <cu/dbgassert.h>


static void test_hashmap(void)
{
	char *keys[] = {
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
	char *vals[] = {
		"val1",
		"val2",
		"val3",
		"val4",
		"val5",
		"val6",
		"val7",
		"val8",
		"val9",
		"val10",
		"val11",
		"val12",
		"val13",
		"val14",
		"val15",
		"val16",
	};
	cu_string_view keystr[16];
	for (size_t i = 0; i < 16; ++i) {
		keystr[i] = cu_cstr_cast(keys[i]);
	}
	cu_hashmap hm;
	int retval = cu_hashmap_new(&hm, NULL);
	dbgassert(retval == 0);
	for (int i = 0; i < 16; ++i) {
		dbgassert(cu_hashmap_at(&hm, keystr[i]) == NULL);
	}
	for (int i = 0; i < 16; ++i) {
		dbgassert(cu_hashmap_insert(&hm, keystr[i], vals[i]) == 0);
	}
	for (int i = 0; i < 16; ++i) {
		char *retstr = cu_hashmap_at(&hm, keystr[i]);
		dbgassert(retstr != NULL);
		dbgassert(strcmp(retstr, vals[i]) == 0);

	}
	dbgassert(cu_hashmap_at(&hm, cu_cstr_cast("asdf")) == NULL);
	cu_hashmap_free(&hm);
}

int main(void)
{
	test_hashmap();
}
