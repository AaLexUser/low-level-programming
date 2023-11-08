#include "parray.h"
#include "utils/logger.h"
#include <stdio.h>

/**
 * Initializes PArray
 * @param block_size
 * @return page_index or PA_FAIL
 */
int64_t pa_init(int64_t block_size){
    int64_t page_index = lp_init_m(sizeof(PArray));
    if(page_index == LP_FAIL){
        logger(LL_ERROR, __func__, "Unable to allocate page");
        return PA_FAIL;
    }
    PArray *pa = (PArray *) lp_load(page_index);
    if(!pa){
        logger(LL_ERROR, __func__, "Unable to allocate page");
        return PA_FAIL;
    }
    pa->page_idx = page_index;
    pa->block_size = block_size;
    pa->size = 0;
    return page_index;
}

/**
 * Loads PArray
 * @param page_index
 * @return
 */
int64_t pa_load(int64_t page_index){
    PArray *pa = (PArray *) lp_load(page_index);
    if(!pa){
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    return page_index;
}

/**
 * Destroys PArray
 * @param page_index
 * @return
 */

int pa_destroy(int64_t page_index) {
    logger(LL_INFO, __func__, "Destroying PArray");
    if (lp_delete(page_index) == LP_FAIL) {
        logger(LL_ERROR, __func__, "Unable to deallocate page");
        return PA_FAIL;
    }
    return PA_SUCCESS;
}


/**
 * Writes to PArray
 * @param page_index
 * @param block_idx
 * @param src
 * @param size
 * @param src_offset
 * @return  PA_SUCCESS or PA_FAIL
 */
int pa_write(int64_t page_index, int64_t block_idx, void *src, int64_t size, int64_t src_offset){
    PArray *pa = (PArray *) lp_load(page_index);
    if(!pa){
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    if(pa->block_size >= src_offset + size){
        logger(LL_ERROR, __func__, "Unable to write to PArray");
        return PA_FAIL;
    }
    off_t offset =  block_idx * pa->block_size + src_offset ;
    if(lp_write(pa->page_idx, src, size, offset) == LP_FAIL){
        logger(LL_ERROR, __func__, "Unable to write to PArray");
        return PA_FAIL;
    }
    pa->size = block_idx > pa->size ? block_idx + 1 : pa->size;
    return PA_SUCCESS;
}

/**
 * Reads from PArray
 * @param page_index
 * @param block_idx
 * @param dest
 * @param size
 * @param src_offset
 * @return  PA_SUCCESS or PA_FAIL
 */

int pa_read(int64_t page_index, int64_t block_idx, void *dest, int64_t size, int64_t src_offset) {
    PArray *pa = (PArray *) lp_load(page_index);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    if (pa->block_size >= src_offset + size) {
        logger(LL_ERROR, __func__, "Unable to read from PArray");
        return PA_FAIL;
    }
    off_t offset = block_idx * pa->block_size + src_offset;
    if (lp_read_copy(pa->page_idx, dest, size, offset) == LP_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read from PArray");
        return PA_FAIL;
    }
    return PA_SUCCESS;
}

/**
 * Append data to PArray
 * @param page_index
 * @param src
 * @param size
 * @return  PA_SUCCESS or PA_FAIL
 */

int pa_append(int64_t page_index, void *src) {
    PArray *pa = (PArray *) lp_load(page_index);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    off_t offset = pa->size * pa->block_size;
    if (lp_write(pa->page_idx, src, pa->block_size, offset) == LP_FAIL) {
        logger(LL_ERROR, __func__, "Unable to push to PArray");
        return PA_FAIL;
    }
    pa->size++;
    return PA_SUCCESS;
}

/**
 * Pop data from PArray
 * @param pa_index
 * @param dest
 * @return  PA_SUCCESS or PA_FAIL
 */

int pa_pop(int64_t pa_index, void *dest) {
    PArray *pa = (PArray *) lp_load(pa_index);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    if (pa->size == 0) {
        logger(LL_INFO, __func__, "Unable to pop from PArray");
        return PA_EMPTY;
    }
    off_t offset = (pa->size - 1) * pa->block_size;
    if (lp_read_copy(pa->page_idx, dest, pa->block_size, offset) == LP_FAIL) {
        logger(LL_ERROR, __func__, "Unable to pop from PArray");
        return PA_FAIL;
    }
    pa->size--;
    return PA_SUCCESS;
}

/**
 * delete data from PArray
 * @param page_index
 * @param dest
 * @param size
 * @return  PA_SUCCESS or PA_FAIL
 */

int pa_delete(int64_t page_index, int64_t block_idx) {
    PArray *pa = (PArray *) lp_load(page_index);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    if (pa->size <= block_idx) {
        logger(LL_ERROR, __func__, "Unable to delete from PArray");
        return PA_FAIL;
    }
    off_t offset = block_idx * pa->block_size;
    if (lp_write(pa->page_idx, NULL, pa->block_size, offset) == LP_FAIL) {
        logger(LL_ERROR, __func__, "Unable to delete from PArray");
        return PA_FAIL;
    }
    pa->size = block_idx == (pa->size - 1) ? pa->size-1 : pa->size;
    return PA_SUCCESS;
}

/**
 * Get size of PArray
 * @param page_index
 * @return  size of PArray
 */

int64_t pa_size(int64_t page_index) {
    PArray *pa = (PArray *) lp_load(page_index);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    return pa->size;
}

/**
 * Get block size of PArray
 * @param page_index
 * @return  block size of PArray
 */

int64_t pa_block_size(int64_t page_index) {
    PArray *pa = (PArray *) lp_load(page_index);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    return pa->block_size;
}

/**
 * Get data from PArray
 * @param page_index
 * @param block_idx
 * @param dest
 * @return  PA_SUCCESS or PA_FAIL
 */

int pa_at(int64_t page_index, int64_t block_idx, void *dest){
    PArray *pa = (PArray *) lp_load(page_index);
    if (!pa) {
        logger(LL_ERROR, __func__, "Unable to load page");
        return PA_FAIL;
    }
    if (pa->size <= block_idx) {
        logger(LL_ERROR, __func__, "Unable to read from PArray");
        return PA_FAIL;
    }
    off_t offset = block_idx * pa->block_size;
    if (lp_read_copy(pa->page_idx, dest, pa->block_size, offset) == LP_FAIL) {
        logger(LL_ERROR, __func__, "Unable to read from PArray");
        return PA_FAIL;
    }
    return PA_SUCCESS;
}

/**
 * Push unique int64_t to PArray
 * @param pa_index
 * @param value
 * @return  PA_SUCCESS or PA_FAIL
 */

int pa_push_unique_int64(int64_t pa_index, int64_t value){
    int64_t size = pa_size(pa_index);
    for(int64_t i = 0; i < size; i++){
        int64_t val = -1;
        if(pa_at(pa_index, i, &val) != PA_SUCCESS){
            logger(LL_ERROR, __func__, "Unable to get value");
            return PA_FAIL;
        }
        if(val == value){
            return PA_SUCCESS;
        }
    }
    if(pa_append(pa_index, &value) != PA_SUCCESS){
        logger(LL_ERROR, __func__, "Unable to append value");
        return PA_FAIL;
    }
    return PA_SUCCESS;
}

