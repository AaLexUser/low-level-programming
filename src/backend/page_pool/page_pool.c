#include "page_pool.h"
#include "backend/io/caching.h"
#include "backend/io/pager.h"
#include "backend/utils/parray64.h"
#include "utils/logger.h"

/**
 * \brief   Function to return fail chblix_t
 * \return  chblix_t
 */

chblix_t chblix_fail(){
    return (chblix_t){.chunk_idx = -1, .block_idx = -1};
}

/**
 * \brief       Compare chblix_t
 * \param[in]   chblix1: First chblix_t
 * \param[in]   chblix2: Second chblix_t
 * \return      0 if equal, 1 if chblix1 > chblix2, -1 if chblix1 < chblix2
 */

int chblix_cmp(const chblix_t* chblix1, const chblix_t* chblix2){
    if(chblix1->chunk_idx == chblix2->chunk_idx && chblix1->block_idx == chblix2->block_idx){
        return 0;
    }
    if(chblix1->chunk_idx > chblix2->chunk_idx)
        return 1;
    if(chblix1->chunk_idx < chblix2->chunk_idx)
        return -1;
    if(chblix1->block_idx > chblix2->block_idx)
        return 1;
    if(chblix1->block_idx < chblix2->block_idx)
        return -1;
    return -1;
}

/**
 * \brief       Initialize chunk
 * \param[in]   page_index: Chunk_t index
 * \return      chunk index on success, PPL_FAIL otherwise
 */

int64_t ppl_chunk_init(page_pool_t* ppl){
    logger(LL_INFO, __func__, "Initializing chunk");
    int64_t page_index = lp_init_m(sizeof(chunk_t));
    if(page_index == LP_FAIL){
        logger(LL_ERROR, __func__, "Unable to load chunk");
        return PPL_FAIL;
    }
    chunk_t* chunk = (chunk_t*) lp_load(page_index);
    chunk->page_index = page_index;
    if(ppl->block_size > lp_useful_space_size((linked_page_t*)chunk)){
        chunk->capacity = 1;
    }
    else{
        chunk->capacity = lp_useful_space_size((linked_page_t*)chunk) / ppl->block_size;
    }
    chunk->next = 0;
    chunk->num_of_free_blocks = chunk->capacity;
    chunk->num_of_used_blocks = 0;
    chunk->prev_page = -1;
    chunk->next_page = -1;
    return page_index;
}

/**
 * \brief       Create page
 * \param[in]   ppl: Chunk pool
 * \param[in]   page_index: Chunk_t index
 * \return      Pointer to page on success, `NULL` otherwise
 */

chunk_t* ppl_create_page(page_pool_t* ppl){
    logger(LL_INFO, __func__, "Creating page");

    // Allocate page
    // Initialize page
    int64_t page_idx = ppl_chunk_init(ppl);
    chunk_t* page = ppl_load_page(page_idx);
    if(!page){
        logger(LL_ERROR, __func__, "Unable to initialize page");
        return NULL;
    }

    return page;
}

/**
 * @brief Loads page
 * @param page_index
 * @return pointer to page or NULL
 */

chunk_t* ppl_load_page(int64_t page_index){
    logger(LL_INFO, __func__, "Loading page");

    if(page_index > get_max_page_index()){
        logger(LL_ERROR, __func__, "chunk_t index is out of range");
        return NULL;
    }

    chunk_t* page = (chunk_t*) lp_load(page_index);
    if(!page){
        logger(LL_ERROR, __func__, "Unable to load page");
        return NULL;
    }

    return page;
}

/**
 * @brief Delete page
 * @param page
 * @return PPL_SUCCESS or PPL_FAIL
 */

int ppl_delete_page(chunk_t* page){
    logger(LL_INFO, __func__, "Destroying page %ld", page->page_index);
    if(lp_delete(page->page_index) == PAGER_FAIL){
        logger(LL_ERROR, __func__, "Unable to delete page");
        return PPL_FAIL;
    }
    return PPL_SUCCESS;
}

/**
 * @brief       Writes to block
 * @param[in]   ppidx: Page pool index
 * @param[in]   chblix chunk_t and block index
 * @param[in]   src source to write from
 * @param[in]   size  size of source data to write
 * @param[in]   src_offset  offset in block to write to
 * @return      PPL_SUCCESS or PPL_FAIL
 */

int ppl_write_block(int64_t ppidx, const chblix_t* chblix, void* src, int64_t size, int64_t src_offset){
    logger(LL_INFO, __func__, "Writing to page %ld, block %ld", chblix->chunk_idx, chblix->block_idx);

    page_pool_t* ppl = ppl_load(ppidx);

    if(chblix->block_idx == -1){
        logger(LL_ERROR, __func__, "Unable to write to block");
        return PPL_FAIL;
    }
    // Check if size is greater than block size
    if(size + src_offset > ppl->block_size){
        logger(LL_ERROR, __func__, "Size + Offset is greater than block size");
        return PPL_FAIL;
    }

    // Declare parameters
    int64_t page_index = chblix->chunk_idx;
    off_t offset = chblix->block_idx * ppl->block_size + src_offset;

    if(lp_write(page_index, src, size, offset) == CachingFail){
        logger(LL_ERROR, __func__, "Unable to write to page");
        return PPL_FAIL;
    }
    return PPL_SUCCESS;



}

/**
 * \brief       Read from block
 * \note        Don't forget to free memory
 * @param[in]   ppidx: Page pool index
 * \param[in]   chblix: Chunk and block index
 * \param[out]  dest: Destination to read to
 * \param[in]   size: Size to read
 * \param[in]   src_offset: Offset in block to read from
 * \return      PP_SUCCESS or PP_FAIL
 */

int ppl_read_block(int64_t ppidx, const chblix_t* chblix, void* dest,  int64_t size, int64_t src_offset){
    logger(LL_INFO, __func__, "Reading from page %ld", chblix->chunk_idx);

    page_pool_t *ppl = ppl_load(ppidx);

    /* Check if size is greater than block size */
    if(src_offset + size > ppl->block_size){
        logger(LL_ERROR, __func__, "Size + Offset is greater than block size");
        return PPL_FAIL;
    }

    /* Declare parameters */
    int64_t page_index = chblix->chunk_idx;
    off_t offset = chblix->block_idx * ppl->block_size + src_offset;

    if(!dest){
        logger(LL_ERROR, __func__, "Unable to allocate memory");
        return PPL_FAIL;
    }

    if(lp_read_copy(page_index, dest, size, offset) == LP_FAIL){
        logger(LL_ERROR, __func__, "Unable to read from page");
        return PPL_FAIL;
    }
    return PPL_SUCCESS;
}


/**
 * \brief   Expand page pool
 * \param[in]   ppl: page pool
 * \return  PPL_SUCCESS or PPL_FAIL
 */

int ppl_pool_expand(page_pool_t* ppl){
    logger(LL_INFO, __func__, "Expanding page pool");

    // Load current page
    chunk_t* current = ppl_load_page(ppl->current_idx);
    if(!current){
        logger(LL_ERROR, __func__, "Unable to load current page");
        return PPL_FAIL;
    }

    chunk_t* new_page = NULL;
    int64_t npidx = -1;

    if(pa_exists64(ppl->wait, current->page_index)){
        logger(LL_ERROR, __func__, "Current page is already in wait");
        return PPL_FAIL;
    }

    /* Check if there is free page */

    int res = pa_pop64(ppl->wait, &npidx);
    switch (res) {
        case PA_SUCCESS: {
            new_page = ppl_load_page(npidx);
            if(!new_page){
                logger(LL_ERROR, __func__, "Unable to load new page");
                return PPL_FAIL;
            }
            break;
        }
        case PA_EMPTY: {
            new_page = ppl_create_page(ppl);
            if(!new_page){
                logger(LL_ERROR, __func__, "Unable to create new page");
                return PPL_FAIL;
            }
            break;
        }
        case PA_FAIL: {
            logger(LL_ERROR, __func__, "Unable to pop from wait");
            return PPL_FAIL;
        }
        default: {
            logger(LL_ERROR, __func__, "Unknown error");
            return PPL_FAIL;
        }
    }

    if(current->page_index == new_page->page_index){
        logger(LL_ERROR, __func__, "Error while expanding page pool, pages have same index");
        return PPL_FAIL;
    }

    current->next_page = new_page->page_index;
    new_page->prev_page = current->page_index;
    ppl->current_idx = new_page->page_index;
    return PPL_SUCCESS;
}


/**
 * @brief       Allocates page
 * @param[in]   ppidx: Page pool index
 * @return      chblix_t or PP_FAIL
 */

chblix_t ppl_alloc(int64_t ppidx) {
    logger(LL_INFO, __func__, "Allocating page");

    /* Load page pool */
    page_pool_t *ppl = ppl_load(ppidx);

    // Load current page
    chunk_t* current = ppl_load_page(ppl->current_idx);
    if(!current){
        logger(LL_ERROR, __func__, "Unable to load current page");
        return (chblix_t){.chunk_idx = PPL_FAIL, .block_idx = PPL_FAIL};
    }

    // Check if next block not already initialized
    if(current->num_of_used_blocks < current->capacity){
        chblix_t chblix = {.chunk_idx = current->page_index, .block_idx = current->num_of_used_blocks };
        current->num_of_used_blocks++;
        ppl_write_block(ppidx, &chblix, &current->num_of_used_blocks,
                        sizeof(int64_t), 0);
    }

    chblix_t chblixres;

    chblixres.block_idx = current->next;
    chblixres.chunk_idx = current->page_index;
    current->num_of_free_blocks--;

    if(current->num_of_free_blocks > 0){
        chblix_t templix = {.chunk_idx = current->page_index, .block_idx = current->next };
        int64_t next = -1;
        ppl_read_block(ppidx, &templix, &next, sizeof(int64_t), 0);
        if(next != -1) current->next = next;
    }
    else if (current->num_of_free_blocks == 0){
        current->next = -1;
        if(ppl_pool_expand(ppl) == PPL_FAIL){
            logger(LL_ERROR, __func__, "Unable to expand page pool");
            return chblix_fail();
        }
        if(current->page_index == ppl->current_idx){
            logger(LL_ERROR, __func__, "Unable to allocate page");
        }
    }
    else {
        logger(LL_ERROR, __func__, "Number of free blocks is less than 0");
        return chblix_fail();
    }

    return chblixres;
}

/**
 * @brief Reduces page pool
 * @param ppl  chunk_t pool
 * @param page  chunk_t to reduce
 * @return  PPL_SUCCESS or PPL_FAIL
 */

int ppl_pool_reduce(page_pool_t* ppl, chunk_t* page){
    logger(LL_INFO, __func__, "Reducing page pool");


    if(ppl->current_idx == page->page_index){
        ppl_pool_expand(ppl);
    }

    chunk_t* prev_page = NULL;
    chunk_t* next_page = NULL;

    if(page->prev_page != -1){
        prev_page = ppl_load_page(page->prev_page);

        if(!prev_page){
            logger(LL_ERROR, __func__, "Unable to load prev page");
            return PPL_FAIL;
        }
    }

    if(page->next_page != -1){
        next_page = ppl_load_page(page->next_page);

        if(!next_page){
            logger(LL_ERROR, __func__, "Unable to load next page");
            return PPL_FAIL;
        }
    }

    if(prev_page && next_page){
        prev_page->next_page = next_page->page_index;
        next_page->prev_page = prev_page->page_index;
    }
    else if(prev_page){
        prev_page->next_page = -1;
    }
    else if(next_page){
        next_page->prev_page = -1;
    }
    else{
        logger(LL_INFO, __func__, "Pool contains only one page");
        return PPL_SUCCESS;
    }

    if (pa_delete_unique64(ppl->wait, page->page_index) == PA_FAIL){
        logger(LL_ERROR, __func__, "Unable to delete page %ld from wait %ld",
               page->page_index, ppl->wait);
        return PPL_FAIL;
    }

    if(ppl_delete_page(page) == PPL_FAIL){
        logger(LL_ERROR, __func__, "Unable to delete page");
        return PPL_FAIL;
    }

    return PPL_SUCCESS;

}

/**
 * @brief       Deallocates page
 * @param[in]   ppidx: Page pool index
 * @param[in]   chblix: Chunk and block index
 * @return      PPL_SUCCESS or PPL_FAIL
 */

int ppl_dealloc(int64_t ppidx, chblix_t* chblix){
    logger(LL_INFO, __func__, "Deallocating page");

    /* Load page pool */
    page_pool_t *ppl = ppl_load(ppidx);

    // Load current page
    chunk_t* page = ppl_load_page(chblix->chunk_idx);
    if(!page){
        logger(LL_ERROR, __func__, "Unable to load page");
        return PPL_FAIL;
    }

    int64_t next_idx = -1;
    if(page->next != -1){
        next_idx = page->next;
    }
    else{
        next_idx = page->num_of_used_blocks;
    }

    ppl_write_block(ppidx, chblix, &next_idx, sizeof(int64_t), 0);
    page->next = chblix->block_idx;
    page->num_of_free_blocks++;


    if(page->num_of_free_blocks == page->capacity){
        ppl_pool_reduce(ppl, page);
        return PPL_SUCCESS;
    }
    if(ppl->current_idx != page->page_index) {
        pa_push_unique64(ppl->wait, chblix->chunk_idx);
    }
    return PPL_SUCCESS;
}



/**
 * @brief Initializes page_pool
 * @param block_size
 * @param start_page_index
 * @return ppl_index or PPL_FAIL
 */

int64_t ppl_init(int64_t block_size){
    logger(LL_INFO, __func__, "Initializing page pool");

    int64_t page_index = lp_init();

    // Load page pool
    page_pool_t* ppl = (page_pool_t*)lp_load(page_index);
    if(!ppl){
        logger(LL_ERROR, __func__, "Unable to load page");
        return PPL_FAIL;
    }
    // Initialize pool
    ppl->block_size = block_size;

    // Initialize first page pool
    int64_t chunk_idx = ppl_chunk_init(ppl);
    if(chunk_idx == PPL_FAIL){
        logger(LL_ERROR, __func__, "Unable to initialize page");
        return PPL_FAIL;
    }
    ppl->current_idx = chunk_idx;
    ppl->head = ppl->current_idx;

    // Initialize wait
    ppl->wait  = pa_init64(sizeof(int64_t), -1);
    if(ppl->wait  == PA_FAIL){
        logger(LL_ERROR, __func__, "Unable to initialize wait");
        return PPL_FAIL;
    }

    return page_index;
}

/**
 * Load existing page_pool_t from file
 * @param   start_page_index
 * @return  pointer to page_pool_t or NULL
 */

page_pool_t* ppl_load(int64_t start_page_index){
    logger(LL_INFO, __func__, "Loading chunk_t Pool %ld.", start_page_index);
    if(start_page_index > get_max_page_index()){
        logger(LL_ERROR, __func__, "You need to init page pool before");
        return NULL;
    }

    // Load page pool
    page_pool_t* ppl = (page_pool_t*)lp_load(start_page_index);
    if(!ppl){
        logger(LL_ERROR, __func__, "Unable to load page");
        return NULL;
    }

    return ppl;
}
