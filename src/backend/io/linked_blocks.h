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

chblix_t lb_alloc(int64_t page_pool_idx);
int lb_load(int64_t page_pool_index, const chblix_t* chblix, linked_block_t* lb);
int lb_update(int64_t ppidx, const chblix_t* chblix, linked_block_t* lb);
int lb_dealloc(int64_t pplidx, const chblix_t* chblix);
chblix_t lb_get_next(int64_t page_pool_index, const chblix_t* chblix);
chblix_t lb_go_to(int64_t pplidx,
                  chblix_t* chblix,
                  int64_t current_block_idx,
                  int64_t block_idx);
int lb_write(int64_t pplidx,
             chblix_t* chblix,
             void *src,
             int64_t size,
             int64_t src_offset);
int lb_read(int64_t pplidx,
            chblix_t* chblix,
            void *dest,
            int64_t size,
            int64_t src_offset);
int64_t lb_useful_space_size(int64_t ppidx, chblix_t* chblix);
int64_t lb_ppl_init(int64_t block_size);

