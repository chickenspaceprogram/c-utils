#pragma once
#include <stddef.h>
#include <c-utils/arena.h>

typedef struct cu_str {
    char *buf;
    size_t len;
} cu_str;

cu_str cu_str_new(struct arena *allocator, size_t size);

