// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <string.h>
#include <cu/alloc.h>


void *cu_realloc(void *mem, size_t newsize, size_t oldsize, cu_alloc *alloc)
{
	if (alloc == NULL)
		return realloc(mem, newsize);
	
	if (alloc->realloc == NULL) {
		void *newbuf = alloc->alloc(newsize, alloc->ctx);
		if (newbuf == NULL) {
			return NULL;
		}
		memcpy(newbuf, mem, oldsize);
		cu_free(mem, oldsize, alloc);
		return newbuf;
	}
	return alloc->realloc(mem, newsize, oldsize, alloc->ctx);
}

