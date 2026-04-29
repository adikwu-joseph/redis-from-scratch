#pragma once

#include <stddef.h>
#include <stdint.h>


struct HeapItem {
    uint64_t val = 0;
    size_t *ref = NULL;
};
inline size_t heap_left(size_t i) { return 2 * i + 1; }
inline size_t heap_right(size_t i) { return 2 * i + 2; }
void heap_update(HeapItem *a, size_t pos, size_t len);