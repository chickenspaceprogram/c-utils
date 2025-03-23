
// Copyright 2024, 2025 Athena Boose

// This file is part of c-utils.

// c-utils is free software: you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation, either version 3 of the License, or (at your option)
// any later version.

// c-utils is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
// for more details.

// You should have received a copy of the GNU General Public License
// along with c-utils. If not, see <https://www.gnu.org/licenses/>. 

#pragma once
#include <stddef.h>
#include <c-utils/arena.h>

typedef struct cu_str {
    unsigned char *string;
    size_t len;
} cu_str;

typedef struct cu_str_pair {
    cu_str fst;
    cu_str snd;
} cu_str_pair;

// Copies the array of `char` pointed to by `string` into a `cu_str`.
// The `cu_str` is allocated on the allocator `allocator`.
//
// If the allocation fails, cu_str.string will be equal to NULL.
cu_str cu_str_new_copy(cu_arena *allocator, char *string, size_t len);

// Moves the array of `char` pointed to by `string` into a `cu_str`.
// This operation runs in O(1) time, so if you have a string which is guaranteed to be valid as long as the `cu_str` will be valid, consider moving it, rather than copying it.
//
// The array `string` must have a length of at least `len`.
cu_str cu_str_new_move(char *string, size_t len);

// Copies the C-string pointed to by `cstring` into a `cu_str` allocated on the allocator `allocator`. 
//
// If the allocation fails, cu_str.string will be equal to NULL.
//
// The null character of `cstring` will be truncated and not included in the `cu_str`.
// If you wish to retain this null character, consider using cu_str_new_copy instead.
cu_str cu_str_new_copy_cstr(cu_arena *allocator, char *cstring);

// Moves the C-string pointed to by `cstring` into a cu_str.
// This operation runs in O(1) time, so if you have a string which is guaranteed to be valid as long as the `cu_str` will be valid, consider moving it, rather than copying it.
// `string` is not modified, however, it is not marked const so that non-const `cu_str`s can be returned.
// 
// Do note, however, that the null character of `cstring` will be truncated and not included in the `cu_str`.
// If you wish to retain this null character, consider using cu_str_new_move instead.
cu_str cu_str_new_move_cstr(char *cstring);


// Returns a copy of `string`, allocated on the allocator `allocator`.
// As with all copies, this runs in O(n) time, so try to avoid copying unless absolutely necessary.
//
// If the allocation fails, cu_str.string will be equal to NULL.
cu_str cu_str_copy(cu_arena *allocator, const cu_str *string);


// Returns a view on `string`.
//
// A view is a shallow copy of a string; it does not hold ownership of it, and will be invalid once `string` is invalid.
// This shallow copy can be performed in O(1) time, so when a deep copy is not needed it's recommended to just get a view.
// Nevertheless, a view can be used for all the same functions that the regular string can, and its creation is guaranteed not to change any of the data in the string.
// (The string is not marked const only so that non-const strings can be returned from the function.)
//
// The view will start at index `start`, and will not include the character at index `end`. Essentially you get the substring [start, end).
//
// This function doesn't validate its inputs! If you pass in a `start` or `end` that is greater than the string's length, it is undefined behavior, as it is if you pass in a `start` that is greater than the `end`.
cu_str cu_str_view(cu_str *string, size_t start, size_t end);

// Concatenates the strings in `strings`, and returns the concatenated result.
//
// The strings are each copied into the result string, which is allocated on the allocator `allocator`.
// If the allocation fails, cu_str.string will be equal to NULL.
cu_str cu_str_cat(cu_arena *allocator, const cu_str *strings, size_t num_strings);

// Compares `string1` and `string2`, starting from the first character.
// If string1 < string2, a number less than 0 is returned.
// If string1 == string2, 0 is returned.
// If string1 > string2, a number greater than 0 is returned.
int cu_str_cmp(const cu_str *string1, const cu_str *string2);

// Breaks `string` up into tokens, delimited by any of the characters in `delims`.
// The number of tokens is returned, and the tokens are stored in an array allocated with `allocator` and stored at `out`.
// If the allocation fails, *out will be equal to NULL.
// 
// Each token does not contain the delimiter. However, as the tokens are each views of `string`, it is valid to access all but the last token one past their length to see the delimiter character.
size_t cu_str_tok(cu_arena *allocator, cu_str *string, const cu_str *delims, cu_str **out);

// Searches `string` for an instance of `character`.
// If the character is found, its index will be returned.
// If not, the length of `string` will be returned.
size_t cu_str_getchr(const cu_str *string, char character);

// Searches `string` for the substring `search_string`.
// If the substring is found, its index will be returned.
// If not, the length of `string` will be returned.
size_t cu_str_getstr(const cu_str *string, const cu_str *search_string);

void cu_str_rev(cu_str *string);

// makes a copy of `string` with every instance of `target` replaced by `replacement`
cu_str cu_str_replace(cu_arena *allocator, const cu_str *string, const cu_str *target, const cu_str *replacement);


// these just apply the toupper/tolower functions in ctype.h to every character
void cu_str_toupper(cu_str *string);
void cu_str_tolower(cu_str *string);

void cu_str_swapcase(cu_str *string);

// see the manpage for ctype.h
// the following just apply the appropriately-named function there to every character in the string; if every character is as desired, 1 is returned, otherwise, 0 is returned.
int cu_str_isalnum(const cu_str *string);
int cu_str_isalpha(const cu_str *string);
int cu_str_iscntrl(const cu_str *string);
int cu_str_isdigit(const cu_str *string);
int cu_str_isgraph(const cu_str *string);
int cu_str_islower(const cu_str *string);
int cu_str_isprint(const cu_str *string);
int cu_str_ispunct(const cu_str *string);
int cu_str_isspace(const cu_str *string);
int cu_str_isupper(const cu_str *string);
int cu_str_isxdigit(const cu_str *string);

int cu_str_isascii(const cu_str *string);
int cu_str_isblank(const cu_str *string);

