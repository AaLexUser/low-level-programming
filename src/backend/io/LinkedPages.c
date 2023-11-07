#include "LinkedPages.h"
#include "caching.h"
#include "pager.h"
#include "utils/logger.h"
#include <math.h>

/**
 *  Initializes LinkedPage
 *  @breif Initializes LinkedPage
 * @return page_index or LP_FAIL
 */

int64_t lp_init(){
    int64_t page_index = pg_alloc();
    if(page_index == PAGER_FAIL){
        logger(LL_ERROR, __func__, "Unable to allocate page");
        return LP_FAIL;
    }
    LinkedPage *lp = (LinkedPage *) ch_load_page(page_index);
    if(lp == NULL){
        logger(LL_ERROR, __func__, "Unable to allocate page");
        return LP_FAIL;
    }
    lp->next_page = -1;
    lp->page_index = page_index;
    return page_index;
}

/**
 * Loads LinkedPage
 * @param page_index
 * @return pointer to LinkedPage or NULL
 */

LinkedPage* lp_load(int64_t page_index){
    LinkedPage *lp = (LinkedPage *) pg_load_page(page_index);
    if(lp == NULL){
        logger(LL_ERROR, __func__, "Unable to allocate page");
        return NULL;
    }
    return lp;
}

/**
 *  Delete LinkedPage
 *  @breif Destroys LinkedPage
 *  @param page_index index of page to delete
 * @return LP_SUCCESS or LP_FAIL
 */

int lp_delete(int64_t page_index) {
    logger(LL_INFO, __func__, "Deleting LinkedPage");

    LinkedPage *current = (LinkedPage *) ch_load_page(page_index);

    while (current->next_page != -1) {
        LinkedPage *next = (LinkedPage *) ch_load_page(current->next_page);
        if (pg_dealloc(current->page_index) == PAGER_FAIL) {
            logger(LL_ERROR, __func__, "Unable to deallocate page");
            return LP_FAIL;
        }
        current = next;
    }

    if (pg_dealloc(current->page_index) == PAGER_FAIL) {
        logger(LL_ERROR, __func__, "Unable to deallocate page");
        return LP_FAIL;
    }
    return LP_SUCCESS;
}



#define lp_for_each(index) for ( \
LinkedPage* index = lp;\
(index)->next_page != -1;                                  \
(index) = ch_load_page((index)->next_page) \
)                                \


/**
 * Writes data to ONE LinkedPage
 * @param lp  LinkedPage to write to
 * @param src  data to write
 * @param size  size of data to write
 * @param src_offset offset in LinkedPage to write to
 * @return LP_SUCCESS or LP_FAIL
 */

int lp_write_page(LinkedPage *lp, void* src, int64_t size, int64_t src_offset){
    logger(LL_INFO, __func__, "Writing to LinkedPage %ld", lp->page_index);
    if(size + src_offset > USED_PAGE_SIZE){
        logger(LL_ERROR, __func__, "Unable to write to LinkedPage %ld, size %ld + offset %ld is too big",
               lp->page_index, size, src_offset);
        return LP_FAIL;
    }
    if(ch_write(lp->page_index, src, size, (off_t)(sizeof_LinkedPage_Header + src_offset)) == CachingFail){
        logger(LL_ERROR, __func__, "Unable to write to LinkedPage %ld", lp->page_index);
        return LP_FAIL;
    }
    return LP_SUCCESS;
}

/**
 * Loads next LinkedPage and allocates new one if needed
 * @param lp
 * @return pointer to next LinkedPage or NULL
 */

LinkedPage* lp_load_next(LinkedPage* lp){
    int64_t page_index = lp->page_index;
    int64_t next_idx = lp->next_page;
    if(lp->next_page == -1){
        next_idx = lp_init();
        if(next_idx == LP_FAIL){
            logger(LL_ERROR, __func__, "Unable to allocate new page");
            return NULL;
        }
        lp =  lp_load(page_index); // cache can remove page from memory after new page init
        lp->next_page = next_idx;
    }
    return lp_load(next_idx);
}


/**
 *  Goes to LinkedPage with given index
 *  @breif Goes to LinkedPage with given index
 * @return pointer to LinkedPage or NULL
 */

LinkedPage* lp_go_to(int64_t start_page_index, int64_t start_idx, int64_t stop_idx){
    LinkedPage* lp = lp_load(start_page_index);
    if(!lp){
        logger(LL_ERROR, __func__, "Unable to load LinkedPage %ld", start_page_index);
        return NULL;
    }

    while (stop_idx > start_idx){
        lp = lp_load_next(lp);
        start_idx++;
    }
    return lp;
}


/**
 *  Writes data to LinkedPage
 *  @breif Writes data to LinkedPage
 *  @param page_index start LinkedPage index
 *  @param src data to write
 *  @param size size of data to write
 *  @param src_offset offset in LinkedPage to write to
 * @return LP_SUCCESS or LP_FAIL
 */

int lp_write(int64_t page_index, void *src, int64_t size, int64_t src_offset) {

    LinkedPage* lp = lp_load(page_index);
    if(!lp){
        logger(LL_ERROR, __func__, "Unable to load LinkedPage %ld", page_index);
        return LP_FAIL;
    }


    logger(LL_INFO, __func__, "Writing to LinkedPage that starts in %ld page.", lp->page_index);
    int64_t starting_page = floor((double) src_offset / (double) USED_PAGE_SIZE);
    int64_t starting_offset = src_offset % (int64_t)USED_PAGE_SIZE;
    int64_t pages_needed = ceil((double)(size + starting_offset) / USED_PAGE_SIZE);
    int64_t current_page_idx = 0;

    // go to start page of write and allocate new pages if needed
    lp = lp_go_to(page_index, current_page_idx, starting_page);

    // write to pages until all data is written
    while (pages_needed > 0) {
        // calculate size to write
        int64_t size_to_write = size > USED_PAGE_SIZE - starting_offset ? (int64_t)USED_PAGE_SIZE - starting_offset : size;

        // write to page
        if (lp_write_page(lp, src, size_to_write, starting_offset) == LP_FAIL) {
            logger(LL_ERROR, __func__, "Unable to write to LinkedPage %ld", lp->page_index);
            return LP_FAIL;
        }

        // update variables
        size -= (int64_t)USED_PAGE_SIZE - starting_offset;
        src += USED_PAGE_SIZE - starting_offset;
        starting_offset = 0;
        pages_needed--;
        if(pages_needed != 0){
            lp = lp_load_next(lp);
        }
    }
    return LP_SUCCESS;
}


/**
 * Reads data from ONE LinkedPage
 * @param lp  LinkedPage to read from
 * @param dest  data to read to
 * @param size  size of data to read
 * @param src_offset  offset in LinkedPage to read from
 * @return  LP_SUCCESS or LP_FAIL
 */

int lp_read_copy_page(LinkedPage* lp, void* dest, int64_t size, int64_t src_offset){
    logger(LL_INFO, __func__, "Reading from LinkedPage %ld", lp->page_index);
    if(size + src_offset > USED_PAGE_SIZE){
        logger(LL_ERROR, __func__, "Unable to read from LinkedPage %ld, size %ld + offset %ld is too big",
               lp->page_index, size, src_offset);
        return LP_FAIL;
    }
    if(ch_copy_read(lp->page_index, dest, size, (off_t)(sizeof_LinkedPage_Header + src_offset)) == CachingFail){
        logger(LL_ERROR, __func__, "Unable to read from LinkedPage %ld", lp->page_index);
        return LP_FAIL;
    }
    return LP_SUCCESS;
}


/**
 * Reads data from LinkedPage
 * @param page_index  LinkedPage index to read from
 * @param dest  data to read to
 * @param size  virt size of data to read
 * @param src_offset  offset in Virt Mem LinkedPage to read from
 * @return  LP_SUCCESS or LP_FAIL
 */

int lp_read_copy(int64_t page_index, void* dest, int64_t size, int64_t src_offset){
    LinkedPage* lp = lp_load(page_index);

    if(!lp){
        logger(LL_ERROR, __func__, "Unable to load LinkedPage %ld", page_index);
        return LP_FAIL;
    }

    logger(LL_INFO, __func__, "Reading from LinkedPage %ld", lp->page_index);
    int64_t starting_page = floor((double) src_offset / (double) USED_PAGE_SIZE);
    int64_t starting_offset = src_offset % (int64_t)USED_PAGE_SIZE;
    int64_t pages_needed = ceil((double)(size + starting_offset) / USED_PAGE_SIZE);
    int64_t current_page_idx = 0;

    lp = lp_go_to(lp->page_index, current_page_idx,starting_page);

    while (pages_needed > 0 ){
        int64_t size_to_read = size > USED_PAGE_SIZE - starting_offset ? (int64_t)USED_PAGE_SIZE - starting_offset : size;
        if(lp_read_copy_page(lp, dest, size_to_read, starting_offset) == CachingFail){
            logger(LL_ERROR, __func__, "Unable to read from LinkedPage %ld", lp->page_index);
            return LP_FAIL;
        }
        starting_offset = 0;
        pages_needed--;
        if(pages_needed != 0){
            size -= (int64_t)USED_PAGE_SIZE - starting_offset;
            dest += USED_PAGE_SIZE - starting_offset;
            lp = lp_load_next(lp);
        }
    }

    return LP_SUCCESS;
}


