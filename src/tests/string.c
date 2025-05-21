// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#ifdef NDEBUG
#undef NDEBUG
#endif

#include <assert.h>
#include <string.h>
#include <c-utils/string.h>

#include <stdio.h>

static void test_new_empty(void) {
    cu_arena arena;
    cu_arena_new(&arena, 4096);
    cu_str new = cu_str_new_empty(&arena, 1234); // checking that creation of new arena doesn't cause a segfault
    for (int i = 0; i < 1234; ++i) {
        new.string[i] = i % 10; // just filling with values so valgrind gets pissed if the memory is bad
    }
    for (int i = 0; i < 1234; ++i) {
        assert(new.string[i] == i % 10);
    }
    cu_arena_free(&arena);
}

static void test_new_copy(void) {
    char str[] = {'t', 'h', 'i', 's', 'i', 's', 'a', 's', 't', 'r', 'i', 'n', 'g'};
    cu_arena arena;
    cu_arena_new(&arena, 4096);
    cu_str new = cu_str_new_copy(&arena, str, 13);
    assert(memcmp(new.string, str, 13) == 0);
    cu_arena_free(&arena);
}

static void test_new_move(void) {
    char str[] = {'t', 'h', 'i', 's', 'i', 's', 'a', 's', 't', 'r', 'i', 'n', 'g'};
    cu_str new = cu_str_new_move(str, 13);
    assert(new.string == str);
}

static void test_new_copy_cstr(void) {
    cu_arena arena;
    cu_arena_new(&arena, 4096);
    char str[] = "look, another test string!";
    cu_str new = cu_str_new_copy_cstr(&arena, str);
    assert(memcmp(new.string, str, strlen(str)) == 0);
    cu_arena_free(&arena);
}

static void test_new_move_cstr(void) {
    char str[] = "look, another test string!";
    cu_str new = cu_str_new_move_cstr(str);
    assert(new.string == str);
}

static void test_copy(void) {
    char str[] = "wow it's another test string";
    cu_str new = cu_str_new_move_cstr(str); // yeah i'm lazy deal with it
    cu_arena arena;
    cu_arena_new(&arena, 4096);
    cu_str copy = cu_str_copy(&arena, &new);
    assert(new.len == copy.len);
    assert(memcmp(new.string, copy.string, new.len) == 0);
    assert(new.string != copy.string);
    cu_arena_free(&arena);
}

static void test_view(void) {
    char str[] = "im running out of funny things to put as strings tbh";
    cu_str new = cu_str_new_move_cstr(str);

    cu_str view1 = cu_str_view(&new, 0, 10);
    assert(view1.len == 10);
    assert(memcmp(new.string, view1.string, view1.len) == 0);

    cu_str view2 = cu_str_view(&new, 18, 23);
    assert(view2.len == 5);
    assert(memcmp(new.string + 18, view2.string, 5) == 0);

    cu_str view3 = cu_str_view(&new, new.len - 3, new.len);
    assert(view3.len == 3);
    assert(memcmp(new.string + new.len - 3, view3.string, 3) == 0);
}

static void test_cat(void) {
    char str1[] = "this is the first string\n";
    char str2[] = "this is the second string\n";
    char str3[] = "thi is the third string\n";
    char str4[] = "i hate writing test cases so much >:(\n";
    cu_str cu_str1 = cu_str_new_move_cstr(str1);
    cu_str cu_str2 = cu_str_new_move_cstr(str2);
    cu_str cu_str3 = cu_str_new_move_cstr(str3);
    cu_str cu_str4 = cu_str_new_move_cstr(str4);
    cu_str array[] = { cu_str1, cu_str2, cu_str3, cu_str4 }; // copying a cu_str does a shallow copy
    cu_arena allocator;
    cu_arena_new(&allocator, 4096);
    cu_arena_alloc(&allocator, 4096);

    cu_str cat = cu_str_cat(&allocator, array, 4);
    assert(memcmp(str1, cat.string, strlen(str1)) == 0);
    assert(memcmp(str2, cat.string + strlen(str1), strlen(str2)) == 0);
    assert(memcmp(str3, cat.string + strlen(str1) + strlen(str2), strlen(str3)) == 0);
    assert(memcmp(str4, cat.string + strlen(str1) + strlen(str2) + strlen(str3), strlen(str4)) == 0);

    cu_arena_free(&allocator);
}

static void test_cmp(void) {
    char lowstr[] = "this is the alower string";
    char highstr[] = "this is the zhigher string";
    cu_str cu_lowstr = cu_str_new_move_cstr(lowstr);
    cu_str cu_highstr = cu_str_new_move_cstr(highstr);

    assert(cu_str_cmp(&cu_lowstr, &cu_highstr) < 0);
    assert(cu_str_cmp(&cu_highstr, &cu_lowstr) > 0);
    assert(cu_str_cmp(&cu_lowstr, &cu_lowstr) == 0);
    assert(cu_str_cmp(&cu_highstr, &cu_highstr) == 0);
}

static void test_tok(void) {
    char str1[] = "must,break,up,string";
    char str2[] = "must,handle,delimiter,at,end,";
    char str3[] = ",must,handle,delimiter,at,start";
    char str4[] = "must,handle,,double,delimiters";
    char delims[] = ",";

    cu_str cu_str1 = cu_str_new_move_cstr(str1);
    cu_str cu_str2 = cu_str_new_move_cstr(str2);
    cu_str cu_str3 = cu_str_new_move_cstr(str3);
    cu_str cu_str4 = cu_str_new_move_cstr(str4);
    cu_str cu_delims = cu_str_new_move_cstr(delims);

    cu_arena arena;
    cu_arena_new(&arena, 4096);

    cu_str *ptr;
    cu_str tok;

    assert(cu_str_tok(&arena, &cu_str1, &cu_delims, &ptr) == 4);

    tok = cu_str_new_move_cstr("must");
    assert(cu_str_cmp(&(ptr[0]), &tok) == 0);
    tok = cu_str_new_move_cstr("break");
    assert(cu_str_cmp(&(ptr[1]), &tok) == 0);
    tok = cu_str_new_move_cstr("up");
    assert(cu_str_cmp(&(ptr[2]), &tok) == 0);
    tok = cu_str_new_move_cstr("string");
    assert(cu_str_cmp(&(ptr[3]), &tok) == 0);

    assert(cu_str_tok(&arena, &cu_str2, &cu_delims, &ptr) == 6);
    tok = cu_str_new_move_cstr("must");
    assert(cu_str_cmp(ptr, &tok) == 0);
    tok = cu_str_new_move_cstr("handle");
    assert(cu_str_cmp(ptr + 1, &tok) == 0);
    tok = cu_str_new_move_cstr("delimiter");
    assert(cu_str_cmp(ptr + 2, &tok) == 0);
    tok = cu_str_new_move_cstr("at");
    assert(cu_str_cmp(ptr + 3, &tok) == 0);
    tok = cu_str_new_move_cstr("end");
    assert(cu_str_cmp(ptr + 4, &tok) == 0);
    assert(cu_str_isempty(ptr + 5));

    assert(cu_str_tok(&arena, &cu_str3, &cu_delims, &ptr) == 6);
    assert(cu_str_isempty(ptr));
    tok = cu_str_new_move_cstr("must");
    assert(cu_str_cmp(ptr + 1, &tok) == 0);
    tok = cu_str_new_move_cstr("handle");
    assert(cu_str_cmp(ptr + 2, &tok) == 0);
    tok = cu_str_new_move_cstr("delimiter");
    assert(cu_str_cmp(ptr + 3, &tok) == 0);
    tok = cu_str_new_move_cstr("at");
    assert(cu_str_cmp(ptr + 4, &tok) == 0);
    tok = cu_str_new_move_cstr("start");
    assert(cu_str_cmp(ptr + 5, &tok) == 0);

    assert(cu_str_tok(&arena, &cu_str4, &cu_delims, &ptr) == 5);
    tok = cu_str_new_move_cstr("must");
    assert(cu_str_cmp(ptr, &tok) == 0);
    tok = cu_str_new_move_cstr("handle");
    assert(cu_str_cmp(ptr + 1, &tok) == 0);
    assert(cu_str_isempty(ptr + 2));
    tok = cu_str_new_move_cstr("double");
    assert(cu_str_cmp(ptr + 3, &tok) == 0);
    tok = cu_str_new_move_cstr("delimiters");
    assert(cu_str_cmp(ptr + 4, &tok) == 0);

    cu_arena_free(&arena);
}

void test_getchr(void) {
    cu_str str = cu_str_new_move_cstr("this is a string that contains an 'x' at index 35");
    assert(cu_str_getchr(&str, 'x') == 35);
}

void test_getstr(void) {
    cu_str str = cu_str_new_move_cstr("this shows the shiny substring shit at index 31. it also only has the first instance of 'shit'");
    cu_str target = cu_str_new_move_cstr("shit");
    assert(cu_str_getstr(&str, &target) == 31);
}

void test_rev(void) {
    char cstr[] = "idk this is a random string";
    char rev_cstr[] = "gnirts modnar a si siht kdi";
    cu_str str = cu_str_new_move_cstr(cstr);
    cu_str rev_str = cu_str_new_move_cstr(rev_cstr);
    cu_str_rev(&str);
    assert(cu_str_cmp(&str, &rev_str) == 0);
}

// still need:
// replace
// toupper
// tolower
// swapcase
// is*
// testing all fns with empty strings
// add some more edge cases

int main(void) {
    test_new_empty();
    test_new_copy();
    test_new_move();
    test_new_copy_cstr();
    test_new_move_cstr();
    test_copy();
    test_view();
    test_cat();
    test_cmp();
    test_tok();
    test_getchr();
    test_getstr();
    test_rev();
}
