#pragma once
#include "backend/page_pool/page_pool.h"
#include <stdlib.h>

typedef struct linked_block{
    chblix_t next_block;
    chblix_t chblix;
    char flag;
    int64_t mem_start;
} linked_block_t;

typedef enum {LB_SUCCESS = 0, LB_FAIL = -1} linked_block_status_t;
typedef enum {LB_FREE = 0, LB_USED = 1} linked_block_flag_t;