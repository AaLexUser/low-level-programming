#pragma once

#include "../io/linked_pages.h"
#include <stdint.h>

#define sizeof_PArray_Header (sizeof(int64_t) * 4)

typedef struct {
    linked_page_t lp;
    int64_t page_idx;
    int64_t size;
    int64_t block_size;
} PArray;

enum PArrayStatuses{PA_SUCCESS = 0, PA_FAIL = -1, PA_EMPTY = -2};

int64_t pa_init(int64_t block_size);
int64_t pa_load(int64_t page_index);
int pa_destroy(int64_t page_index);
int pa_write(int64_t page_index, int64_t block_idx, void *src, int64_t size, int64_t src_offset);
int pa_read(int64_t page_index, int64_t block_idx, void *dest, int64_t size, int64_t src_offset);
int pa_append(int64_t page_index, void *src);
int pa_pop(int64_t pa_index, void *dest);
int pa_delete(int64_t page_index, int64_t block_idx);
int64_t pa_size(int64_t page_index);
int64_t pa_block_size(int64_t page_index);
int pa_at(int64_t page_index, int64_t block_idx, void *dest);
int pa_push_unique_int64(int64_t pa_index, int64_t value);
