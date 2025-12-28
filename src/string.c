// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <ctype.h>
#include <limits.h>
#include <cu/string.h>
#include <cu/bitmanip.h>

// i thought for a bit about how to write the logic for this and decided
// fuck it, you get a LUT and you'd damned well better be happy with it
//
// any number >= 36 is treated as not a digit
// i did that to make a conditional simpler
static const uint8_t DIGIT_LUT[0x100] = {
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 36, 36, 36, 36, 36, 36, 
36, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 36, 36, 36, 36, 
36, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 36, 36, 36, 36, 
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 
};
// str is guaranteed to start with a number with no sign, have a nonzero length
// base is guaranteed to be from 2-36
// *num == 0
static inline int
parse_internal (
	cu_str str,
	int base,
	cu_str *rest,
	uintmax_t *num
) {
	assert(*num == 0 && "num is invalid");
	assert(base >= 2 && base <= 36 && "base is invalid");
	size_t i = 0;
	*num = DIGIT_LUT[str.buf[i++]];
	if (*num >= base) {
		*num = 0;
		*rest = str;
		return CU_STR_ENOINT; // invalid dig
	}
	int retval = 0;
	for (; i < str.len; ++i) {
		uint8_t digit_val = DIGIT_LUT[str.buf[i]];
		if (digit_val >= base)
			break;

		uintmax_t cur_num = *num;
		if (cu_ckd_mul(num, cur_num, base)) {
			// overflow!
			*num = UINTMAX_MAX;
			retval = CU_STR_EOUTOFRANGE;
			// skip main loop
			for (; i < str.len; ++i) {
				if (digit_val >= base)
					break;
			}
			break;
		}
		*num += digit_val;
	}
	if (rest != NULL) {
		rest->buf = str.buf + i;
		rest->len = str.len - i;
	}
	return retval;
}

int cu_str_parse_signed(cu_str str, int base, cu_str *rest, intmax_t *num)
{
	*num = 0;
	if (base != 0 && (base < 2 || base > 36)) {
		if (rest != NULL)
			*rest = str;
		return CU_STR_EBADBASE;
	}
	// consume spaces
	size_t nws = 0;
	for (; nws < str.len; ++nws) {
		if (!isspace(str.buf[nws])) {
			break;
		}
	}
	str.buf += nws;
	str.len -= nws;
	
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
	if (str.len == 0) {
		if (rest != NULL)
			*rest = str;
		return CU_STR_ENOINT;
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

	uintmax_t res_int = 0;
	int result = parse_internal(str, base, rest, &res_int);
	if (res_int > INTMAX_MAX)
		*num = (sign > 0) ? INTMAX_MAX : INTMAX_MIN;
	else
		*num = sign * res_int;
	return result;
}
int cu_str_parse_unsigned(cu_str str, int base, cu_str *rest, uintmax_t *num)
{
	*num = 0;
	if (base != 0 && (base < 2 || base > 36)) {
		if (rest != NULL)
			*rest = str;
		return CU_STR_EBADBASE;
	}
	// consume spaces
	size_t nws = 0;
	for (; nws < str.len; ++nws) {
		if (!isspace(str.buf[nws])) {
			break;
		}
	}
	str.buf += nws;
	str.len -= nws;
	
	// sign finding
	if (str.buf[0] == '+') {
		++str.buf;
		--str.len;
	}
	if (str.len == 0) {
		if (rest != NULL)
			*rest = str;
		return CU_STR_ENOINT;
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

	return parse_internal(str, base, rest, num);
}
