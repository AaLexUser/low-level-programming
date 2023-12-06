#include "linked_blocks.h"
#include "page_pool.h"
#include "core/io/file.h"
#include "utils/logger.h"

#include <math.h>

/**
 * \brief       Allocates new linked block, with custom memory start
 * \param[in]   page_pool_idx
 * \return      chblix or chblix_fail
 */

chblix_t lb_alloc_m(int64_t page_pool_idx, int64_t mem_start){
    logger(LL_INFO, __func__, "Linked_block allocating start.");

    page_pool_t *ppl = ppl_load(page_pool_idx);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return chblix_fail();
    }

    chblix_t chblix = ppl_alloc(page_pool_idx);
    if (chblix.block_idx == -1) {
        logger(LL_ERROR, __func__, "Unable to allocate block");
        return chblix_fail();
    }

    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    lb_load(page_pool_idx, &chblix, lb);

    lb->next_block = chblix_fail();
    lb->prev_block = chblix_fail();
    lb->chblix = chblix;
    lb->flag = LB_USED;
    lb->mem_start = mem_start;

    lb_update(page_pool_idx, &chblix, lb);

    logger(LL_INFO, __func__,
           "Linked_block allocating finished. Linked_block chunk_index: %ld, block_index: %ld",
           chblix.chunk_idx, chblix.block_idx);

    free(lb);
    return chblix;
}


/**
 * \brief Allocates new linked block
 * \param[in] page_pool_idx
 * \return chblix or chblix_fail
 */

chblix_t lb_alloc(int64_t page_pool_idx) {
    return lb_alloc_m(page_pool_idx, sizeof(linked_block_t));
}

/**
 * \brief       Loads linked block
 * \param[in]   page_pool_idx: Fist page index of page pool
 * \param[in]   chblix: Chunk Block Index
 * \param[out]  lb: Linked Block pointer
 * \return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_load(int64_t page_pool_index, const chblix_t* chblix, linked_block_t* lb) {
    page_pool_t *ppl = ppl_load(page_pool_index);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }

    if(ppl_read_block(page_pool_index, chblix, lb, ppl->block_size, 0) != PPL_SUCCESS){
        logger(LL_ERROR, __func__, "Unable to read block");
        return LB_FAIL;
    }

    return LB_SUCCESS;
}

/**
 * @brief       Update linked block
 * @param[in]   ppl: Page pool pointer
 * @param[in]   chblix: Chunk Block Index
 * @param[in]   lb: Linked Block pointer
 * @return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_update_nova(page_pool_t* ppl, const chblix_t* chblix, linked_block_t* lb){
    if(ppl_write_block_nova(ppl, chblix, lb, ppl->block_size, 0) != PPL_SUCCESS){
        logger(LL_ERROR, __func__, "Unable to read block");
        return LB_FAIL;
    }
    return LB_SUCCESS;
}

/**
 * @brief       Update linked block
 * @param[in]   ppidx: Fist page index of page pool
 * @param[in]   chblix: Chunk Block Index
 * @param[in]   lb: Linked Block pointer
 * @return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_update(int64_t ppidx, const chblix_t* chblix, linked_block_t* lb) {
    page_pool_t *ppl = ppl_load(ppidx);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }

    if(ppl_write_block(ppidx, chblix, lb, ppl->block_size, 0) != PPL_SUCCESS){
        logger(LL_ERROR, __func__, "Unable to read block");
        return LB_FAIL;
    }

    return LB_SUCCESS;
}

int lb_dealloc_nova(page_pool_t* ppl, linked_block_t* lb, const chblix_t* chblix){
    chblix_t fail = chblix_fail();
    while (chblix_cmp(&lb->next_block, &fail) != 0) {
        chblix_t next_block_idx = lb->next_block;

        /* Setting flag to free */
        lb->flag = LB_FREE;
        lb_update_nova(ppl, &lb->chblix, lb);

        /* Deallocating block */
        if (ppl_dealloc_nova(ppl, &lb->chblix) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to deallocate block");
            free(lb);
            return LB_FAIL;
        }

        /* Loading next block */
        if (lb_load_nova_ppp(ppl, &next_block_idx, lb) == LB_FAIL) {
            logger(LL_ERROR, __func__, "Unable to read block");
            free(lb);
            return LB_FAIL;
        }

    }

    /* Setting flag to free */
    lb->flag = LB_FREE;
    lb_update_nova(ppl, &lb->chblix, lb);

    /* Deallocating block */
    if (ppl_dealloc_nova(ppl, &lb->chblix) == PPL_FAIL) {
        logger(LL_ERROR, __func__, "Unable to deallocate block");
        free(lb);
        return LB_FAIL;
    }
    return LB_SUCCESS;
}

/**
 * \brief       Dealloc linked block
 * \param[in]   pplidx: Fist page index of page pool
 * \param[in]   chblix: Chunk Block Index
 * \return      LB_SUCCESS or LB_FAIL
 */


int lb_dealloc(int64_t pplidx, const chblix_t* chblix){

    /* Loading Page Pool*/
    page_pool_t *ppl = ppl_load(pplidx);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }

    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    if (lb_load(pplidx, chblix, lb) == LB_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        free(lb);
        return LB_FAIL;
    }

    chblix_t fail = chblix_fail();
    while (chblix_cmp(&lb->next_block, &fail) != 0) {
        chblix_t next_block_idx = lb->next_block;

        /* Setting flag to free */
        lb->flag = LB_FREE;
        lb_update(pplidx, &lb->chblix, lb);

        /* Deallocating block */
        if (ppl_dealloc(pplidx, &lb->chblix) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to deallocate block");
            free(lb);
            return LB_FAIL;
        }

        /* Loading next block */
        if (lb_load(pplidx, &next_block_idx, lb) == LB_FAIL) {
            logger(LL_ERROR, __func__, "Unable to read block");
            free(lb);
            return LB_FAIL;
        }

    }

    /* Setting flag to free */
    lb->flag = LB_FREE;
    lb_update(pplidx, &lb->chblix, lb);

    /* Deallocating block */
    if (ppl_dealloc(pplidx, &lb->chblix) == PPL_FAIL) {
        logger(LL_ERROR, __func__, "Unable to deallocate block");
        free(lb);
        return LB_FAIL;
    }

    free(lb);
    return LB_SUCCESS;
}

/**
 * \brief       Loads next linked block
 * \param[in]   ppl: Page pool pointer
 * \param[in]   chblix: chblix of Linked Block
 * \return      pointer to linked_block_t on success, `NULL` otherwise
 */

chblix_t lb_get_next_nova(page_pool_t* ppl, const chblix_t* chblix){
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return chblix_fail();
    }
    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    lb_load_nova_ppp(ppl, chblix, lb);

    chblix_t fail = chblix_fail();
    chblix_t next_block_idx = lb->next_block;
    if (chblix_cmp(&next_block_idx, &fail) == 0) {
        /* Allocating new block */
        next_block_idx = lb_alloc(ppl->lp_header.page_index);
        if (next_block_idx.block_idx == -1) {
            logger(LL_ERROR, __func__, "Unable to allocate block");
            free(lb);
            return chblix_fail();
        }
        linked_block_t* next_lb = malloc(ppl->block_size); /* Don't forget to free it */
        lb_load_nova_ppp(ppl, &next_block_idx, next_lb);
        next_lb->prev_block = lb->chblix;
        lb_update_nova(ppl, &next_block_idx, next_lb);
        free(next_lb);
        lb->next_block = next_block_idx;
        lb_update_nova(ppl, chblix, lb);
    }

    free(lb);
    return next_block_idx;

}


/**
 * \brief       Go to block
 * \param[in]   ppl: Page pool pointer
 * \param[in]   chblix: Chunk Block Index
 * \param[in]   current_block_idx: Current block index
 * \param[in]   block_idx: Block index to go to
 * \return      chblix_t on success, `chblix_fail()` otherwise
 */

chblix_t lb_go_to_nova(page_pool_t* ppl,
                       chblix_t* chblix,
                       int64_t current_block_idx,
                       int64_t block_idx){
    int64_t counter = current_block_idx;
    chblix_t res = *chblix;
    /* Go to block */
    while (counter != block_idx) {
        res = lb_get_next_nova(ppl, chblix);
        counter++;
    }

    return res;

}

/**
 * \brief       Loads next linked block
 * \param[in]   page_pool_idx: Fist page index of page pool
 * \param[in]   chblix: chblix of Linked Block
 * \return      pointer to linked_block_t on success, `NULL` otherwise
 */

chblix_t lb_get_next(int64_t page_pool_index,
                     const chblix_t* chblix){

    /* Loading Page Pool*/
    page_pool_t *ppl = ppl_load(page_pool_index);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return chblix_fail();
    }

    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    lb_load(page_pool_index, chblix, lb);

    chblix_t fail = chblix_fail();
    chblix_t next_block_idx = lb->next_block;
    if (chblix_cmp(&next_block_idx, &fail) == 0) {
        /* Allocating new block */
        next_block_idx = lb_alloc(page_pool_index);
        if (next_block_idx.block_idx == -1) {
            logger(LL_ERROR, __func__, "Unable to allocate block");
            free(lb);
            return chblix_fail();
        }
        linked_block_t* next_lb = malloc(ppl->block_size); /* Don't forget to free it */
        lb_load(page_pool_index, &next_block_idx, next_lb);
        next_lb->prev_block = lb->chblix;
        lb_update(page_pool_index, &next_block_idx, next_lb);
        free(next_lb);
        lb->next_block = next_block_idx;
        lb_update(page_pool_index, chblix, lb);
    }

    free(lb);
    return next_block_idx;
}

/**
 * \brief       Go to block
 * \param[in]   page_pool_idx: Fist page index of page pool
 * \param[in]   chblix: Chunk Block Index
 * \param[in]   current_block_idx: Current block index
 * \param[in]   block_idx: Block index to go to
 * \return      chblix_t on success, `chblix_fail()` otherwise
 */

chblix_t lb_go_to(int64_t pplidx,
                  chblix_t* chblix,
                  int64_t current_block_idx,
                  int64_t block_idx) {

    int64_t counter = current_block_idx;
    chblix_t res = *chblix;
    /* Go to block */
    while (counter != block_idx) {
        res = lb_get_next(pplidx, chblix);
        counter++;
    }

    return res;
}


/**
 * \brief       Read from linked block
 * \param[in]   page_pool_index: Fist page index of page pool
 * \param[in]   chblix: Chunk Block Index
 * \param[in]   src: Source to write
 * \param[in]   size: Size to read
 * \param[in]   src_offset: Offset in block to write
 * \return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_write(int64_t pplidx,
             chblix_t* chblix,
             void *src,
             int64_t size,
             int64_t src_offset) {

    logger(LL_INFO, __func__, "Write to Linked Block %ld %ld size: %ld, offset: %ld"
            , chblix->block_idx, chblix->chunk_idx, size, src_offset);

    /* Loading Page Pool*/
    page_pool_t *ppl = ppl_load(pplidx);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }

    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    if (lb_load(pplidx, chblix, lb) == LB_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        free(lb);
        return LB_FAIL;
    }

    /* Initializing variables */
    int64_t useful_space_size = ppl->block_size - lb->mem_start;
    int64_t start_block = floor((double) src_offset / (double) useful_space_size);
    int64_t start_offset = src_offset % useful_space_size;
    int64_t blocks_needed = ceil((double)(size + start_offset) / (double) useful_space_size);
    int64_t total_size = size;
    int64_t current_block_idx = 0;
    int64_t header_offset =  lb->mem_start;

    /* Go to start block of write and allocate new blocks if needed */
    chblix_t start_point = chblix_fail();
    start_point = lb_go_to(pplidx, chblix, current_block_idx, start_block);

    /* Write to blocks until all data is written */
    while (blocks_needed > 0){
        /* Calculate size to write */
        int64_t size_to_write = total_size > useful_space_size - start_offset
                                ? useful_space_size - start_offset : total_size;

        /* Write to block */
        if (ppl_write_block(pplidx, &start_point, src, size_to_write, header_offset + start_offset) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to write to block");
            free(lb);
            return LB_FAIL;
        }

        /* Update variables */
        blocks_needed--;

        if(blocks_needed > 0){
            total_size -= useful_space_size - start_offset;
            src += useful_space_size - start_offset;
            start_offset = 0;

            /* Go to next block */
            start_point = lb_get_next(pplidx, &lb->chblix);
            lb_load(pplidx, &start_point, lb);

        }

    }

    free(lb);
    return LB_SUCCESS;
}

/**
 * @brief       Read from linked block
 * @param[in]   ppl: Page Pool pointer
 * @param[in]   chunk: pointer to chunk
 * @param[in]   chblix: Chunk Block Index
 * @param[out]  dest: Destination to read
 * @param[in]   size: Size to read
 * @param[in]   src_offset: Offset in block to read
 * @return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_read_nova(page_pool_t* ppl,
                 chunk_t* chunk,
                 chblix_t* chblix,
                 void *dest,
                 int64_t size,
                 int64_t src_offset){
    logger(LL_INFO, __func__, "Reading Linked Block %ld %ld size: %ld, offset: %ld"
            , chblix->block_idx, chblix->chunk_idx, size, src_offset);
    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    if (lb_load_nova_pppp(ppl, chunk, chblix, lb) == LB_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        free(lb);
        return LB_FAIL;
    }
    /* Initializing variables */
    int64_t useful_space_size = ppl->block_size - lb->mem_start;
    int64_t start_block = floor((double) src_offset / (double) useful_space_size);
    int64_t start_offset = src_offset % useful_space_size;
    int64_t blocks_needed = ceil((double)(size + start_offset) / (double) useful_space_size);
    int64_t total_size = size;
    int64_t current_block_idx = 0;
    int64_t header_offset = lb->mem_start;

    /* Go to start block of write and allocate new blocks if needed */
    chblix_t start_point = chblix_fail();
    start_point = lb_go_to_nova(ppl, chblix, current_block_idx, start_block);
    linked_page_t* start_chunk = lp_load(start_point.chunk_idx);

    /* Write to blocks until all data is written */
    while (blocks_needed > 0){
        /* Calculate size to read */
        int64_t size_to_read = total_size > useful_space_size - start_offset
                               ? useful_space_size - start_offset : total_size;

        /* Write to block */
        if (ppl_read_block_nova(ppl, start_chunk, &start_point, dest, size_to_read, header_offset + start_offset) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to write to block");
            free(lb);
            return LB_FAIL;
        }

        /* Update variables */
        blocks_needed--;

        if(blocks_needed > 0){
            total_size -= (useful_space_size - start_offset);
            dest += useful_space_size - start_offset;
            start_offset = 0;

            /* Go to next block */
            start_point = lb_get_next_nova(ppl, &lb->chblix);
            lb_load_nova_ppp(ppl, &start_point, lb);

        }

    }
    return LB_SUCCESS;

}

int lb_read_nova_5(page_pool_t* ppl,
                   chblix_t* chblix,
                   void *dest,
                   int64_t size,
                   int64_t src_offset){
    chunk_t* chunk = ppl_load_chunk(chblix->chunk_idx);
    return lb_read_nova(ppl, chunk, chblix, dest, size, src_offset);

}

/**
 * @brief       Read from linked block
 * @param[in]   pplidx: Fist page index of page pool
 * @param[in]   chblix: Chunk Block Index
 * @param[out]  dest: Destination to read
 * @param[in]   size: Size to read
 * @param[in]   src_offset: Offset in block to read
 * @return      LB_SUCCESS on success, LB_FAIL otherwise
 */

int lb_read(int64_t pplidx,
            chblix_t* chblix,
            void *dest,
            int64_t size,
            int64_t src_offset){

    logger(LL_INFO, __func__, "Reading Linked Block %ld %ld size: %ld, offset: %ld"
            , chblix->block_idx, chblix->chunk_idx, size, src_offset);

    /* Loading Page Pool*/
    page_pool_t *ppl = ppl_load(pplidx);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }

    /* Loading Linked Block */
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    if (lb_load(pplidx, chblix, lb) == LB_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        free(lb);
        return LB_FAIL;
    }

    /* Initializing variables */
    int64_t useful_space_size = ppl->block_size - lb->mem_start;
    int64_t start_block = floor((double) src_offset / (double) useful_space_size);
    int64_t start_offset = src_offset % useful_space_size;
    int64_t blocks_needed = ceil((double)(size + start_offset) / (double) useful_space_size);
    int64_t total_size = size;
    int64_t current_block_idx = 0;
    int64_t header_offset = lb->mem_start;

    /* Go to start block of write and allocate new blocks if needed */
    chblix_t start_point = chblix_fail();
    start_point = lb_go_to(pplidx, chblix, current_block_idx, start_block);

    /* Write to blocks until all data is written */
    while (blocks_needed > 0){
        /* Calculate size to read */
        int64_t size_to_read = total_size > useful_space_size - start_offset
                               ? useful_space_size - start_offset : total_size;

        /* Write to block */
        if (ppl_read_block(pplidx, &start_point, dest, size_to_read, header_offset + start_offset) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to write to block");
            free(lb);
            return LB_FAIL;
        }

        /* Update variables */
        blocks_needed--;

        if(blocks_needed > 0){
            total_size -= (useful_space_size - start_offset);
            dest += useful_space_size - start_offset;
            start_offset = 0;

            /* Go to next block */
            start_point = lb_get_next(pplidx, &lb->chblix);
            lb_load(pplidx, &start_point, lb);

        }

    }
    return LB_SUCCESS;
}




/**
 * @brief       Get useful space size of linked block
 * @param[in]   ppidx: Fist page index of page pool
 * @param[in]   chblix: Chunk Block Index
 * @return      useful space size on success, LB_FAIL otherwise
 */

int64_t lb_useful_space_size(int64_t ppidx, chblix_t* chblix){
    page_pool_t *ppl = ppl_load(ppidx);
    linked_block_t* lb = malloc(ppl->block_size); /* Don't forget to free it */
    if(lb_load(ppidx, chblix, lb) == LB_FAIL){
        logger(LL_ERROR, __func__, "Unable to load linked block");
        free(lb);
        return LB_FAIL;
    }
    int64_t res = PAGE_SIZE - lb->mem_start;
    free(lb);
    return res;
}

/**
 * @brief       Initialize page pool for Linked Blocks
 * @param[in]   block_size: Size of block
 * @return      page pool index on success, LB_FAIL otherwise
 */

int64_t lb_ppl_init(int64_t block_size){
    int64_t ppidx = ppl_init(block_size + (int64_t)sizeof(linked_block_t));
    if(ppidx == PPL_FAIL){
        logger(LL_ERROR, __func__, "Unable to initialize page pool block size: %ld"
               , block_size);
        return LB_FAIL;
    }

    page_pool_t *ppl = ppl_load(ppidx);
    if(ppl == NULL){
        logger(LL_ERROR, __func__, "Unable to load page pool with index: %ld"
               , ppidx);
        return LB_FAIL;
    }
    return ppidx;
}

/**
 * @brief       Load existing page pool for Linked Blocks
 * @param[in]   ppidx: Fist page index of page pool
 * @return      pointer to page pool on success, `NULL` otherwise
 */

page_pool_t* lb_ppl_load(int64_t ppidx){
    return ppl_load(ppidx);
}

static chblix_t lb_nearest_valid_chblix_chunk(page_pool_t* ppl, chunk_t* chunk, int64_t block_idx){
    chblix_t chblix = {.block_idx = block_idx, .chunk_idx = chunk->page_index};
    linked_block_t* temp = malloc(ppl->block_size);
    while (chblix.block_idx != chunk->capacity){
        if(lb_valid(ppl, chunk, chblix)){
            free(temp);
            return chblix;
        }
        chblix.block_idx++;
    }

    free(temp);
    return chblix_fail();
}

chblix_t lb_nearest_valid_chblix(page_pool_t* ppl, chblix_t chblix, chunk_t** chunk){
    if(chunk == NULL){
        return chblix_fail();
    }
    do{
        chblix_t chblix_res = lb_nearest_valid_chblix_chunk(ppl, *chunk, chblix.block_idx);
        if(chblix_cmp(&chblix_res, &CHBLIX_FAIL) != 0){
            return chblix_res;
        }
        if((*chunk)->next_page != -1){
            (*chunk) = ppl_load_chunk((*chunk)->next_page);
            chblix.block_idx = 0;
        }
    } while((*chunk)->next_page != -1);
    return chblix_fail();
}


chblix_t lb_pool_start(page_pool_t* ppl, chunk_t* chunk){
    if(ppl->head == -1){
        return chblix_fail();
    }
    if(chunk == NULL){
        return chblix_fail();
    }
    return lb_nearest_valid_chblix_chunk(ppl, chunk, 0);
}

bool lb_valid(page_pool_t* ppl, chunk_t* chunk, chblix_t chblix){
    if(chblix_cmp(&chblix, &CHBLIX_FAIL) == 0){
        return false;
    }
    linked_block_t* linked_block = malloc(ppl->block_size);
    if(lb_load_nova_pppp(ppl, chunk,  &chblix, linked_block) == LB_FAIL){
        logger(LL_ERROR, __func__, "Unable to load linked block");
        free(linked_block);
        return false;
    }
    if(linked_block->flag == LB_FREE){
        free(linked_block);
        return false;
    }
    if(chblix_cmp(&linked_block->prev_block, &CHBLIX_FAIL) != 0){
        free(linked_block);
        return false;
    }
    return chblix_cmp(&chblix, &CHBLIX_FAIL) != 0;
}

int lb_load_nova_pppp(page_pool_t* ppl, chunk_t* chunk, chblix_t* chblix, linked_block_t* linked_block){
    return ppl_read_block_nova(ppl,
                               (linked_page_t*)chunk,
                               chblix,
                               linked_block,
                               ppl->block_size,
                               0) == PPL_FAIL ? LB_FAIL : LB_SUCCESS;
}

int lb_load_nova_ppp(page_pool_t* ppl, chblix_t* chblix, linked_block_t* linked_block){
    chunk_t* chunk = ppl_load_chunk(chblix->block_idx);
    return ppl_read_block_nova(ppl,
                               (linked_page_t*)chunk,
                               chblix,
                               linked_block,
                               ppl->block_size,
                               0) == PPL_FAIL ? LB_FAIL : LB_SUCCESS;
}




