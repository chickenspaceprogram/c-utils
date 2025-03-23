#include <string.h>
#include <ctype.h>
#include <c-utils/string.h>

cu_str cu_str_new_empty(cu_arena *allocator, size_t len) {
    cu_str new_string = {
        .string = cu_arena_alloc(allocator, len),
        .len = len,
    };
    return new_string; // other functions will have to check that malloc didn't shit itself
}

cu_str cu_str_new_copy(cu_arena *allocator, char *string, size_t len) {
    cu_str new_cu_str = cu_str_new_empty(allocator, len);
    if (new_cu_str.string == NULL) {
        return new_cu_str;
    }
    memcpy(new_cu_str.string, string, len);
    return new_cu_str;
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
    return cu_str_new_copy(allocator, cstring, len);
}

cu_str cu_str_new_move_cstr(char *cstring) {
    size_t len = strlen(cstring);
    return cu_str_new_move(cstring, len);
}

cu_str cu_str_copy(cu_arena *allocator, const cu_str *string) {
    return cu_str_new_copy(allocator, string->string, string->len);
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
    cu_str new_cu_str = cu_str_new_empty(allocator, concatenated_size);
    if (new_cu_str.string != NULL) {
        for (size_t i = 0, current_index = 0; i < num_strings; ++i) {
            memcpy(new_cu_str.string + current_index, strings[i].string, strings[i].len);
            current_index += strings[i].len;
        }
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

size_t cu_str_tok(cu_arena *allocator, cu_str *string, const cu_str *delims, cu_str **const out) {
    // this function kinda sucks because it loops through the string twice, but idk if making a new arena would actually be faster for most usecases
    size_t num_delims = 0;
    for (size_t i = 0; i < string->len; ++i) {
        for (size_t j = 0; j < delims->len; ++j) {
            if (string->string[i] == delims->string[j]) {
                ++num_delims;
                break;
            }
        }
    }

    *out = cu_arena_alloc(allocator, sizeof(cu_str) * (num_delims + 1));
    if (*out == NULL) {
        return 0;
    }

    if (num_delims == 0) {
        *(out)[0] = cu_str_view(string, 0, string->len); // can avoid looping back through the string
        return 1;
    }

    size_t start_index = 0;
    size_t num_toks = 0;
    for (size_t i = 0; i < string->len; ++i) {
        for (size_t j = 0; j < delims->len; ++j) {
            if (string->string[i] == delims->string[j]) {
                (*out)[num_toks] = cu_str_view(string, start_index, i);
                start_index = ++i;
                ++num_toks;
                break;
            }
        }
    }
    if (start_index < string->len) {
        (*out)[num_toks] = cu_str_view(string, start_index, string->len);
    }
    return num_toks;
}

size_t cu_str_getchr(const cu_str *string, char character) {
    void *memchr_result = memchr(string->string, character, string->len);
    if (memchr_result == NULL) {
        return string->len;
    }
    return (char *)memchr_result - string->string;
}

size_t cu_str_getstr(cu_str *string, const cu_str *search_string) {
    size_t index = 0;
    while ((index = cu_str_getchr(string, search_string->string[0])) != string->len) {
        if (index + search_string->len > string->len) {
            break;
        }
        cu_str view = cu_str_view(string, index, search_string->len);
        if (cu_str_cmp(&view, search_string) == 0) {
            return index;
        }
    }
    return string->len;
}

cu_str cu_str_replace(cu_arena *allocator, cu_str *string, const cu_str *target, const cu_str *replacement) {
    size_t num_occurrences = 0;
    size_t index = 0;
    cu_str string_view = cu_str_view(string, 0, string->len);
    while ((index = cu_str_getstr(&string_view, target)) != string->len) {
        ++num_occurrences;
        string_view = cu_str_view(string, index + target->len, string->len);
    }

    cu_str new_str = cu_str_new_empty(allocator, string->len + (replacement->len - target->len) * num_occurrences);
    string_view = cu_str_view(string, 0, string->len);
    index = 0;
    size_t copy_start = 0;
    while ((index = cu_str_getstr(&string_view, target)) != string->len) {
        memcpy(new_str.string + copy_start, string_view.string, index);
        memcpy(new_str.string + copy_start + index, replacement->string, replacement->len);
        copy_start += index + replacement->len;
        string_view = cu_str_view(string, index + target->len, string->len);
    }
    memcpy(new_str.string + copy_start, string_view.string, string_view.len);
    return new_str;
}

void cu_str_rev(cu_str *string) {
    for (size_t i = 0; i < string->len / 2; ++i) {
        char temp = string->string[i];
        string->string[i] = string->string[string->len - i - 1];
        string->string[string->len - i - 1] = temp;
    }
}

void cu_str_toupper(cu_str *string) {
    for (size_t i = 0; i < string->len; ++i) {
        string->string[i] = toupper(string->string[i]);
    }
}

void cu_str_tolower(cu_str *string) {
    for (size_t i = 0; i < string->len; ++i) {
        string->string[i] = tolower(string->string[i]);
    }
}

void cu_str_swapcase(cu_str *string) {
    for (size_t i = 0; i < string->len; ++i) {
        if (islower(string->string[i])) {
            string->string[i] = toupper(string->string[i]);
        }
        else if (isupper(string->string[i])) {
            string->string[i] = tolower(string->string[i]);
        }
    }
}

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
