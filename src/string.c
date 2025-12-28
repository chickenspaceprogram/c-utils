// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <ctype.h>
#include <cu/string.h>

int cu_str_parse_signed(cu_str str, int base, cu_str *rest, intmax_t *num)
{
	if (base != 0 && (base < 2 || base > 36))
		return CU_STR_EBADBASE;
	// consume spaces
	size_t nws = 0;
	for (; nws < str.len; ++nws) {
		if (!isspace(str.buf[nws])) {
			break;
		}
	}
	str.buf += nws;
	str.len -= nws;
	if (str.len == 0)
		return CU_STR_ENOINT;
	
	// sign finding
	int sign = 1;
	if (str.len > 1) { // str has sign and digits
		if (str.buf[0] == '-') {
			sign = -1;
			++str.buf;
			--str.len;
		}
		else if (str.buf[0] == '+') {
			++str.buf;
			--str.len;
		}
	}
	// base guessing
	if (base == 0) {
		if (str.len >= 2 && str.buf[0] == '0' && str.buf[1] == 'x') {
			base = 16;
			str.buf += 2;
			str.len -= 2;
		}
		else if (str.buf[0] == '0') {
			base = 8;
			str.buf += 1;
			str.len -= 1;
		}
		else {
			base = 10;
		}
	}

	intmax_t res_int = 0;
	if (base == 2) {
		// special binary handling
	}
	else if (base == 4) {
		// special quaternary handling
	}
	else if (base == 8) {
		// special oct handling
	}
	else if (base == 16) {
		// special hex handling
	}
	else if (base == 32) {
		// special base-32 handling
	}
	else {
		// general handling
	}
	return 0;
}
int cu_str_parse_unsigned(cu_str str, int base, cu_str *rest, uintmax_t *num)
{
	if (base != 0 && (base < 2 || base > 36))
		return CU_STR_EBADBASE;
	// consume spaces
	size_t nws = 0;
	for (; nws < str.len; ++nws) {
		if (!isspace(str.buf[nws])) {
			break;
		}
	}
	str.buf += nws;
	str.len -= nws;
	if (str.len == 0)
		return CU_STR_ENOINT;
	
	// sign finding
	int sign = 1;
	if (str.len > 1 && str.buf[0] == '+') { // str has sign and digits
		++str.buf;
		--str.len;
	}
	// base guessing
	if (base == 0) {
		if (str.len >= 2 && str.buf[0] == '0' && str.buf[1] == 'x') {
			base = 16;
			str.buf += 2;
			str.len -= 2;
		}
		else if (str.buf[0] == '0') {
			base = 8;
			str.buf += 1;
			str.len -= 1;
		}
		else {
			base = 10;
		}
	}

	intmax_t res_int = 0;
	if (base == 2) {
		// special binary handling
	}
	else if (base == 4) {
		// special quaternary handling
	}
	else if (base == 8) {
		// special oct handling
	}
	else if (base == 16) {
		// special hex handling
	}
	else if (base == 32) {
		// special base-32 handling
	}
	else {
		// general handling
	}
	return 0;
}
