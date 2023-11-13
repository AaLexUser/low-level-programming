#include "pager.h"
#include "../../utils/logger.h"
#include "../utils/parray64.h"
#include "caching.h"

Pager pg;

#ifndef DELETED_PAGES_START_INDEX
#define DELETED_PAGES_START_INDEX 0
#endif

/**
 * @breif       Initializes pager
 * @param[in]   file_name: name of file to store data
 * @return      PAGE_SUCCESS on success, PAGE_FAIL otherwise
 */

int pg_init(const char* file_name){
    logger(LL_INFO, __func__, "Initializing pager");
    if (ch_init(file_name, &pg.ch) == CH_FAIL) {
        logger(LL_ERROR, __func__, "Unable to initialize caching");
        return PAGER_FAIL;
    }
    pg.deleted_pages = -1;
    pg.deleted_pages = pa_init64(sizeof(int64_t), -1);
    return PAGER_SUCCESS;
}

/**
 * @brief       Delete file and destroy caching
 * @return      PAGE_SUCCESS on success, PAGE_FAIL otherwise
 */

int pg_delete(){
    logger(LL_INFO, __func__, "Deleting file");
    if(ch_delete(&pg.ch) == CH_FAIL){
        logger(LL_ERROR, __func__, "Unable to delete caching");
        return PAGER_FAIL;
    }
    return PAGER_SUCCESS;
}

/**
 * @brief       Close file and destroy caching
 * @return      PAGE_SUCCESS on success, PAGE_FAIL otherwise
 */

int pg_close(){
    logger(LL_INFO, __func__, "Closing file");
    if(ch_close(&pg.ch) == CH_FAIL){
        logger(LL_ERROR, __func__, "Unable to delete caching");
        return PAGER_FAIL;
    }
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
        if((page_idx = ch_new_page(&pg.ch)) == CH_FAIL){
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
    if((page_ptr = ch_load_page(&pg.ch, page_idx)) == NULL){
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
    ch_delete_page(&pg.ch, page_index);
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
    if((page_ptr = ch_load_page(&pg.ch, page_index)) == NULL){
        logger(LL_ERROR, __func__, "Unable to load page");
        return NULL;
    }
    return page_ptr;
}

/**
 * @brief       Write to page
 * @param[in]   page_index: page index
 * @param[in]   src: source
 * @param[in]   size: size to write
 * @param[in]   offset: offset in page to write to
 * @return      PAGER_SUCCESS on success, PAGER_FAIL otherwise
 */

int pg_write(uint64_t page_index, void* src, size_t size, off_t offset){
    logger(LL_INFO, __func__, "Writing to page");
    if(ch_write(&pg.ch, page_index, src, size, offset) == CH_FAIL){
        logger(LL_ERROR, __func__, "Unable to write to page");
        return PAGER_FAIL;
    }
    return PAGER_SUCCESS;
}

/**
 * @brief       Read from page
 * @param[in]   page_index: page index
 * @param[out]  dest: destination
 * @param[in]   size: size to read
 * @param[in]   offset: offset in page to read from
 * @return      PAGER_SUCCESS on success, PAGER_FAIL otherwise
 */

int pg_copy_read(uint64_t page_index, void* dest, size_t size, off_t offset){
    logger(LL_INFO, __func__, "Reading from page");
    if(ch_copy_read(&pg.ch, page_index, dest, size, offset) == CH_FAIL){
        logger(LL_ERROR, __func__, "Unable to read from page");
        return PAGER_FAIL;
    }
    return PAGER_SUCCESS;
}

/**
 * @brief   Get current max page index
 * @return  max page index
 */

int64_t pg_max_page_index(){
    return ch_max_page_index(&pg.ch);
}

/**
 * @brief   Get current file size
 * @return  file size
 */

off_t pg_file_size(){
    return ch_file_size(&pg.ch);
}

