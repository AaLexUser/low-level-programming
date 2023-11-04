#pragma once
#include "../pstack/pstack.h"
#include "file_manager.h"
#include <stdint.h>
#include <stdlib.h>

#ifndef PAGE_POOL_SIZE
#define PAGE_POOL_SIZE 100
#endif

typedef struct {
    PStack_List* deleted_pages;

} Pager;

enum PagerStatuses{PAGER_SUCCESS = 0, PAGER_FAIL = -1};

int pg_init();
int pg_destroy();
int64_t pg_alloc();
void* pg_alloc_page();
int pg_dealloc(int64_t page_index);

