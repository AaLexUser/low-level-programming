#include "linked_blocks.h"
#include "backend/page_pool/page_pool.h"
#include "utils/logger.h"

#include <math.h>

/**
 * \brief Allocates new linked block
 * \param[in] page_pool_idx
 * \return LB_SUCCESS or LB_FAIL
 */

chblix_t lb_alloc(const int64_t page_pool_idx) {
    logger(LL_INFO, __func__, "Linked_block allocating start.");

    page_pool_t *ppl = ppl_load(page_pool_idx);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return chblix_fail();
    }

    chblix_t chblix = ppl_alloc(ppl);
    if (chblix.block_idx == -1) {
        logger(LL_ERROR, __func__, "Unable to allocate block");
        return chblix_fail();
    }

    linked_block_t *lb = (linked_block_t*)ppl_read_block(ppl, &chblix, 0);
    if (lb == NULL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        return chblix_fail();
    }

    lb->next_block = chblix_fail();
    lb->chblix = chblix;
    lb->flag = LB_USED;
    lb->mem_start = sizeof(linked_block_t);

    logger(LL_INFO, __func__,
           "Linked_block allocating finished. Linked_block chunk_index: %ld, block_index: %ld",
           chblix.chunk_idx, chblix.block_idx);

    return chblix;
}

/**
 * \brief       Loads linked block
 * \param[in]   page_pool_idx: Fist page index of page pool
 * \param[in]   chblix: Chunk Block Index
 * \return      pointer to linked_block_t on success, `NULL` otherwise
 */

linked_block_t* lb_load(const int64_t page_pool_index, const chblix_t chblix) {
    page_pool_t *ppl = ppl_load(page_pool_index);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return NULL;
    }

    linked_block_t *lb = (linked_block_t *) ppl_read_block(ppl, &chblix, 0);
    if (lb == NULL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        return NULL;
    }

    return lb;
}

/**
 * \brief       Dealloc linked block
 * \param[in]   page_pool_idx: Fist page index of page pool
 * \param[in]   chblix: Chunk Block Index
 * \return      LB_SUCCESS or LB_FAIL
 */

int lb_dealloc(const int64_t page_pool_index, const chblix_t chblix) {

    /* Loading Page Pool*/
    page_pool_t *ppl = ppl_load(page_pool_index);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }

    /* Loading Linked Block */
    linked_block_t *lb = (linked_block_t *) ppl_read_block(ppl, &chblix, 0);
    if (lb == NULL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        return LB_FAIL;
    }

    chblix_t fail = chblix_fail();
    while (chblix_cmp(&lb->next_block, &fail) != 0) {
        chblix_t next_block_idx = lb->next_block;

        /* Setting flag to free */
        lb->flag = LB_FREE;

        /* Deallocating block */
        if (ppl_dealloc(ppl, &lb->chblix) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to deallocate block");
            return LB_FAIL;
        }

        /* Loading next block */
        lb = (linked_block_t *) ppl_read_block(ppl, &next_block_idx, 0);
        if (lb == NULL) {
            logger(LL_ERROR, __func__, "Unable to read block");
            return LB_FAIL;
        }

    }

    /* Setting flag to free */
    lb->flag = LB_FREE;

    /* Deallocating block */
    if (ppl_dealloc(ppl, &lb->chblix) == PPL_FAIL) {
        logger(LL_ERROR, __func__, "Unable to deallocate block");
        return LB_FAIL;
    }

    return LB_SUCCESS;
}

/**
 * \brief       Loads next linked block
 * \param[in]   page_pool_idx: Fist page index of page pool
 * \param[in]   chblix: chblix of Linked Block
 * \return      pointer to linked_block_t on success, `NULL` otherwise
 */

chblix_t lb_get_next(const int64_t page_pool_index,
                             const chblix_t* chblix){

    /* Loading Page Pool*/
    page_pool_t *ppl = ppl_load(page_pool_index);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return chblix_fail();
    }

    /* Loading Linked Block */
    linked_block_t *lb = (linked_block_t *) ppl_read_block(ppl, chblix, 0);

    chblix_t fail = chblix_fail();
    chblix_t next_block_idx = lb->next_block;
    if (chblix_cmp(&next_block_idx, &fail)) {
        /* Allocating new block */
        next_block_idx = lb_alloc(page_pool_index);
        if (next_block_idx.block_idx == -1) {
            logger(LL_ERROR, __func__, "Unable to allocate block");
            return chblix_fail();
        }
    }

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

chblix_t lb_go_to(const int64_t page_pool_index,
                     const chblix_t chblix,
                     const int64_t current_block_idx,
                     const int64_t block_idx) {

    int64_t counter = current_block_idx;
    chblix_t res = chblix;
    /* Go to block */
    while (counter != block_idx) {
        res = lb_get_next(page_pool_index, &chblix);
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

int lb_write(const int64_t page_pool_index,
             const chblix_t chblix,
             void *src,
             const int64_t size,
             const int64_t src_offset) {

    /* Loading Page Pool*/
    page_pool_t *ppl = ppl_load(page_pool_index);
    if (ppl == NULL) {
        logger(LL_ERROR, __func__, "Unable to load page pool");
        return LB_FAIL;
    }

    /* Loading Linked Block */
    linked_block_t *lb = (linked_block_t *) ppl_read_block(ppl, &chblix, 0);
    if (lb == NULL) {
        logger(LL_ERROR, __func__, "Unable to read block");
        return LB_FAIL;
    }

    /* Initializing variables */
    int64_t useful_space_size = ppl->block_size - lb->mem_start;
    int64_t start_block = floor((double) src_offset / (double) useful_space_size);
    int64_t start_offset = src_offset % useful_space_size;
    int64_t blocks_needed = ceil((double)(size + start_offset) / (double) useful_space_size);
    int64_t total_size = size;
    int64_t current_block_idx = 0;

    /* Go to start block of write and allocate new blocks if needed */
    chblix_t start_point = chblix_fail();
    start_point = lb_go_to(page_pool_index, chblix, current_block_idx, start_block);

    /* Write to blocks until all data is written */
    while (blocks_needed > 0){
        /* Calculate size to write */
        int64_t size_to_write = size > useful_space_size - start_offset
                ? useful_space_size - start_offset : size;

        /* Write to block */
        if (ppl_write_block(ppl, &start_point, src, size_to_write, start_offset) == PPL_FAIL) {
            logger(LL_ERROR, __func__, "Unable to write to block");
            return LB_FAIL;
        }

        /* Update variables */
        blocks_needed--;

        if(blocks_needed > 0){
            total_size -= useful_space_size - start_offset;
            src += useful_space_size - start_offset;
            start_offset = 0;

            /* Go to next block */
            start_point = lb_get_next(page_pool_index, &lb->chblix);

        }

    }
    return LB_SUCCESS;
}



