// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

// SPDX-License-Identifier: MPL-2.0

#include <cu/string.h>
#include <cu/dbgassert.h>


/*
enum {
	CU_STR_OK = 0,			// Not an error, conversion successful

	CU_STR_EBADBASE = -1,		// Invalid base provided
	CU_STR_EOUTOFRANGE = -2,	   Integer is out of range, returned
					   value clamped to minimum or maximum
					   value for the type   
	CU_STR_ENOINT = -3,		   String doesn't start with a valid
					   integer   
	CU_STR_ESIGN = -4,		   Integer was negative, and was
					   attempted to be parsed as unsigned.   
};
*/

// int cu_str_parse_signed(cu_str str, int base, cu_str *rest, intmax_t *num);
// int cu_str_parse_unsigned(cu_str str, int base, cu_str *rest, uintmax_t *num);

static void signed_tests(void)
{
	cu_str rest_ptr;
	intmax_t val;
	int retval;

	retval = cu_str_parse_signed(
		cu_str_from_cstr("12345"),
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 12345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	retval = cu_str_parse_signed(
		cu_str_from_cstr("12345"),
		10,
		NULL,
		&val
	);
	dbgassert(val == 12345);
	dbgassert(retval == CU_STR_OK);

	retval = cu_str_parse_signed(
		cu_str_from_cstr(" \n\r\t\v\f12345"),
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 12345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	retval = cu_str_parse_signed(
		cu_str_from_cstr("+12345"),
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 12345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	retval = cu_str_parse_signed(
		cu_str_from_cstr("-12345"),
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == -12345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	cu_str asdf = cu_str_from_cstr("12345   ");
	retval = cu_str_parse_signed(
		asdf,
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 12345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.buf == asdf.buf + 5);
	dbgassert(rest_ptr.len == asdf.len - 5);

	retval = cu_str_parse_signed(
		cu_str_from_cstr("0xDEADbeef"),
		0,
		&rest_ptr,
		&val
	);
	dbgassert(val == 0xDEADbeef);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	retval = cu_str_parse_signed(
		cu_str_from_cstr("012345"),
		0,
		&rest_ptr,
		&val
	);
	dbgassert(val == 012345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	retval = cu_str_parse_signed(
		cu_str_from_cstr("12345"),
		8,
		&rest_ptr,
		&val
	);
	dbgassert(val == 012345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);


	val = 69420;
	asdf = cu_str_from_cstr("!@#$%^&");
	retval = cu_str_parse_signed(
		asdf,
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 0);
	dbgassert(retval == CU_STR_ENOINT);
	dbgassert(rest_ptr.buf == asdf.buf);
	dbgassert(rest_ptr.len == asdf.len);

	val = 69420;
	asdf = cu_str_from_cstr("12345");
	retval = cu_str_parse_signed(
		asdf,
		123,
		&rest_ptr,
		&val
	);
	dbgassert(val == 0);
	dbgassert(retval == CU_STR_EBADBASE);
	dbgassert(asdf.buf == rest_ptr.buf);
	dbgassert(asdf.len == rest_ptr.len);

	retval = cu_str_parse_signed(
		cu_str_from_cstr("99999999999999999999"),
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == INTMAX_MAX);
	dbgassert(retval == CU_STR_EOUTOFRANGE);
	dbgassert(rest_ptr.len == 0);

	retval = cu_str_parse_signed(
		cu_str_from_cstr("-99999999999999999999"),
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == INTMAX_MIN);
	dbgassert(retval == CU_STR_EOUTOFRANGE);
	dbgassert(rest_ptr.len == 0);
}

static void unsigned_tests(void)
{
	cu_str rest_ptr;
	uintmax_t val;
	int retval;

	retval = cu_str_parse_unsigned(
		cu_str_from_cstr("12345"),
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 12345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	retval = cu_str_parse_unsigned(
		cu_str_from_cstr("12345"),
		10,
		NULL,
		&val
	);
	dbgassert(val == 12345);
	dbgassert(retval == CU_STR_OK);

	retval = cu_str_parse_unsigned(
		cu_str_from_cstr(" \n\r\t\v\f12345"),
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 12345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	retval = cu_str_parse_unsigned(
		cu_str_from_cstr("+12345"),
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 12345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	cu_str asdf = cu_str_from_cstr("-12345");
	retval = cu_str_parse_unsigned(
		asdf,
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 0);
	dbgassert(retval == CU_STR_ENOINT);
	dbgassert(rest_ptr.buf == asdf.buf);
	dbgassert(rest_ptr.len == asdf.len);

	asdf = cu_str_from_cstr("12345   ");
	retval = cu_str_parse_unsigned(
		asdf,
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 12345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.buf == (uint8_t *)asdf.buf + 5);
	dbgassert(rest_ptr.len == asdf.len - 5);

	retval = cu_str_parse_unsigned(
		cu_str_from_cstr("0xDEADbeef"),
		0,
		&rest_ptr,
		&val
	);
	dbgassert(val == 0xDEADbeef);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	retval = cu_str_parse_unsigned(
		cu_str_from_cstr("012345"),
		0,
		&rest_ptr,
		&val
	);
	dbgassert(val == 012345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);

	retval = cu_str_parse_unsigned(
		cu_str_from_cstr("12345"),
		8,
		&rest_ptr,
		&val
	);
	dbgassert(val == 012345);
	dbgassert(retval == CU_STR_OK);
	dbgassert(rest_ptr.len == 0);


	val = 69420;
	asdf = cu_str_from_cstr("!@#$%^&");
	retval = cu_str_parse_unsigned(
		asdf,
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == 0);
	dbgassert(retval == CU_STR_ENOINT);
	dbgassert(rest_ptr.buf == asdf.buf);
	dbgassert(rest_ptr.len == asdf.len);

	val = 69420;
	asdf = cu_str_from_cstr("12345");
	retval = cu_str_parse_unsigned(
		asdf,
		123,
		&rest_ptr,
		&val
	);
	dbgassert(val == 0);
	dbgassert(retval == CU_STR_EBADBASE);
	dbgassert(asdf.buf == rest_ptr.buf);
	dbgassert(asdf.len == rest_ptr.len);

	retval = cu_str_parse_unsigned(
		cu_str_from_cstr("99999999999999999999"),
		10,
		&rest_ptr,
		&val
	);
	dbgassert(val == UINTMAX_MAX);
	dbgassert(retval == CU_STR_EOUTOFRANGE);
	dbgassert(rest_ptr.len == 0);
}

int main(void)
{
	signed_tests();
	unsigned_tests();
}
