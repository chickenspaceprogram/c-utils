// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <ctype.h>
#include <cu/string.h>

int cu_str_parse_signed(cu_str str, int base, cu_str *rest, intmax_t *num)
{
	// consume spaces
	size_t nws = 0;
	for (; nws < str.len; ++nws) {
		if (!isspace(str.buf[nws])) {
			break;
		}
	}
	str.buf += nws;
	str.len -= nws;
}
int cu_str_parse_unsigned(cu_str str, int base, cu_str *rest, uintmax_t *num);
