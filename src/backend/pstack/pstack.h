#pragma once

#include <stdint.h>
#include <stdlib.h>

#define sizeof_PStack_Header (sizeof(int64_t) + sizeof(uint32_t) + \
sizeof(int64_t) + sizeof(uint32_t) + sizeof(uint32_t) + sizeof(int64_t))

typedef struct  __attribute__((packed)){
    int64_t next_page;
    uint32_t block_size;
    int64_t page_index;
    uint32_t capacity;
    uint32_t size;
    int64_t prev_page;
    char data[];
} PStack;


enum PStackStatuses{PSTACK_SUCCESS = 0, PSTACK_FAIL = -1, PSTACK_EMPTY = 1};

typedef struct{
    int64_t current_idx;
} PStack_List;

PStack* pst_load(size_t block_size, int64_t page_index);
int pst_destroy(PStack* pst);
int pst_list_expand(PStack_List* pstl);
int pst_list_reduce(PStack_List* pstl);
int pst_push(PStack_List* pstl, void* data);
int pst_pop(PStack_List* pstl, void* data);
PStack_List* pst_list_init(size_t block_size, int64_t start_page_index);
int pst_list_destroy(PStack_List* pStackList);
