#pragma once
#include <stdint.h>

#define sizeof_LinkedPage_Header (sizeof(int64_t) * 2)
#define USED_PAGE_SIZE (PAGE_SIZE - sizeof_LinkedPage_Header)

typedef struct {
    int64_t next_page;
    int64_t page_index;
    char data[];
} LinkedPage;

enum LinkedPageStatuses{LP_SUCCESS = 0, LP_FAIL = -1};

int64_t lp_init();
LinkedPage* lp_load(int64_t page_index);
int lp_delete(int64_t page_index);
int lp_write_page(LinkedPage *lp, void* src, int64_t size, int64_t src_offset);
LinkedPage* lp_load_next(LinkedPage* lp);
LinkedPage* lp_go_to(int64_t start_page_index, int64_t start_idx, int64_t stop_idx);
int lp_write(int64_t page_index, void *src, int64_t size, int64_t src_offset);
int lp_read_copy_page(LinkedPage* lp, void* dest, int64_t size, int64_t src_offset);
int lp_read_copy(int64_t page_index, void* dest, int64_t size, int64_t src_offset);