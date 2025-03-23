#include <string.h>
#include <ctype.h>
#include <c-utils/string.h>

cu_str cu_str_new_copy(cu_arena *allocator, char *string, size_t len) {
    cu_str new_string = {
        .string = cu_arena_alloc(allocator, len),
        .len = len,
    };

    return new_string; // other functions will have to check that malloc didn't shit itself
}

cu_str cu_str_new_move(char *string, size_t len) {
    cu_str new_string = {
        .string = string,
        .len = len,
    };
    return new_string;
}

cu_str cu_str_new_copy_cstr(cu_arena *allocator, char *cstring) {
    size_t len = strlen(cstring);
    cu_str new_cu_str = {
        .string = cu_arena_alloc(allocator, len),
        .len = len,
    };
    if (new_cu_str.string == NULL) {
        return new_cu_str;
    }
    memcpy(new_cu_str.string, cstring, len);
    return new_cu_str;
}

cu_str cu_str_new_move_cstr(char *cstring) {
    size_t len = strlen(cstring);
    cu_str new_cu_str = {
        .string = cstring,
        .len = len,
    };
    return new_cu_str;
}

cu_str cu_str_copy(cu_arena *allocator, const cu_str *string) {
    cu_str new_cu_str = {
        .string = cu_arena_alloc(allocator, string->len),
        .len = string->len,
    };
    if (new_cu_str.string == NULL) {
        return new_cu_str;
    }
    memcpy(new_cu_str.string, string->string, string->len);
    return new_cu_str;
}

cu_str cu_str_view(cu_str *string, size_t start, size_t end) {
    cu_str new_view = {
        .string = string->string + start,
        .len = end - start,
    };
    return new_view;
}

cu_str cu_str_cat(cu_arena *allocator, const cu_str *strings, size_t num_strings) {
    size_t concatenated_size = 0;
    for (size_t i = 0; i < num_strings; ++i) {
        concatenated_size += strings[i].len;
    }
    cu_str new_cu_str = {
        .string = cu_arena_alloc(allocator, concatenated_size),
        .len = concatenated_size,
    };
    if (new_cu_str.string == NULL) {
        return new_cu_str;
    }
    for (size_t i = 0, current_index = 0; i < num_strings; ++i) {
        memcpy(new_cu_str.string + current_index, strings[i].string, strings[i].len);
        current_index += strings[i].len;
    }
    return new_cu_str;
}

int cu_str_cmp(const cu_str *string1, const cu_str *string2) {
    if (string1->len > string2->len) {
        return 1;
    }
    if (string1->len < string2->len) {
        return -1;
    }
    return memcmp(string1, string2, string1->len);
}

// need strtok, getchr, getstr, rev, toupper, tlower, swapcase, replace

#define APPLY_CTYPE_FN(FUNC)\
int cu_str_##FUNC(const cu_str *string) {\
    for (size_t i = 0; i < string->len; ++i) {\
        if (!FUNC(string->string[i])) {\
            return 0;\
        }\
    }\
    return 1;\
}

// vile but i hate typing
APPLY_CTYPE_FN(isalnum)
APPLY_CTYPE_FN(isalpha)
APPLY_CTYPE_FN(iscntrl)
APPLY_CTYPE_FN(isdigit)
APPLY_CTYPE_FN(isgraph)
APPLY_CTYPE_FN(islower)
APPLY_CTYPE_FN(isprint)
APPLY_CTYPE_FN(ispunct)
APPLY_CTYPE_FN(isspace)
APPLY_CTYPE_FN(isupper)
APPLY_CTYPE_FN(isxdigit)
APPLY_CTYPE_FN(isascii)
APPLY_CTYPE_FN(isblank)

#undef APPLY_CTYPE_FN // incase someone includes this they won't hate me
