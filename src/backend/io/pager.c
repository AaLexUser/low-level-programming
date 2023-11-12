#include "pager.h"
#include "../../utils/logger.h"
#include "../utils/parray64.h"
#include "caching.h"

Pager pg;

#ifndef DELETED_PAGES_START_INDEX
#define DELETED_PAGES_START_INDEX 0
#endif

/**
 *  Initializes pager
 *  @breif Initializes pager
 * @return PAGE_SUCCESS or PAGE_FAIL
 */

int pg_init(){
    logger(LL_INFO, __func__, "Initializing pager");
    pg.deleted_pages = -1;
    pg.deleted_pages = pa_init64(sizeof(int64_t), -1);
    return PAGER_SUCCESS;
}

/**
 *  Destroys pager
 *  @breif Destroys pager
 * @return PAGE_SUCCESS or PAGE_FAIL
 */

int pg_destroy() {
    logger(LL_INFO, __func__, "Destroying pager");
    return PAGER_SUCCESS;
}

/**
 * Allocates page
 * @brief Loads free pages from file or allocates new page
 * @return index of page or PAGER_FAIL
 */

int64_t pg_alloc(){
    logger(LL_INFO, __func__, "Allocating page");
    int64_t page_idx = -1;

    int64_t del_pag_idx = -1;

    if(pg.deleted_pages != -1){
        if((pa_pop64(pg.deleted_pages, &del_pag_idx)) == PA_SUCCESS){
            page_idx = del_pag_idx;
        }
    }

    if(del_pag_idx == -1){
        logger(LL_INFO, __func__, "Unable to pop page from deleted pages, allocating new page");
        if((page_idx = ch_new_page()) == CachingFail){
            logger(LL_ERROR, __func__, "Unable to load new page");
            return PAGER_FAIL;
        }
    }
    return page_idx;
}

/**
 * Allocates page
 * @return pointer to page or NULL
 */

void* pg_alloc_page(){
    logger(LL_INFO, __func__, "Allocating page");
    int64_t page_idx = -1;
    if ((page_idx = pg_alloc()) == PAGER_FAIL) {
        logger(LL_ERROR, __func__, "Unable to allocate page");
        return NULL;
    }

    void* page_ptr = NULL;
    if((page_ptr = ch_load_page(page_idx)) == NULL){
        logger(LL_ERROR, __func__, "Unable to load page");
        return NULL;
    }
    return page_ptr;
}


/**
 * Deallocates page
 * @warning double deleting same page may cause program crash in future
 * @brief Deallocates page
 * @param page_index
 * @return PAGER_SUCCESS or PAGER_FAIL
 */

int pg_dealloc(int64_t page_index) {
    logger(LL_INFO, __func__, "Deallocating page");
    pa_push_unique64(pg.deleted_pages, page_index);
    ch_delete_page(page_index);
    return PAGER_SUCCESS;
}

/**
 * Loads page
 * @brief Loads page
 * @param page_index
 * @return pointer to page or NULL
 */

void* pg_load_page(int64_t page_index){
    logger(LL_INFO, __func__, "Loading page");
    void* page_ptr = NULL;
    if((page_ptr = ch_load_page(page_index)) == NULL){
        logger(LL_ERROR, __func__, "Unable to load page");
        return NULL;
    }
    return page_ptr;
}

