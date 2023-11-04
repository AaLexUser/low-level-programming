#include "../../utils/logger.h"
#include "../io/caching.h"
#include "pstack.h"
#include <stddef.h>
#include <stdlib.h>

PStack* pst_init(size_t block_size, int64_t page_index, PStack* pstack){
    logger(LL_INFO, __func__, "Initializing pstack");
    pstack->page_index = page_index;
    if(block_size > PAGE_SIZE){
        logger(LL_ERROR, __func__, "Block size doesn't fit in page");
        pst_destroy(pstack);
        return NULL;
    }
    pstack->block_size = block_size;
    pstack->size = 0;
    pstack->capacity = (PAGE_SIZE - sizeof_PStack_Header) / block_size;
    pstack->prev_page = -1;
    pstack->next_page = -1;
    return pstack;
}

/**
 * @brief Load or allocate a pstack
 *
 * @param block_size The size of a block in the pstack
 * @param page_index The index of the page where the pstack is located
 * @return PStack* The initialized pstack or NULL
 */

PStack* pst_load(size_t block_size, int64_t page_index){
    logger(LL_INFO, __func__, "Initializing pstack");
    PStack* pstack;
    if(page_index > get_max_page_index()){
        if ((page_index = ch_new_page()) == CachingFail){
            logger(LL_ERROR, __func__, "Unable to allocate new page");
            return NULL;
        }
        if(!(pstack = ch_load_page(page_index))) {
            logger(LL_INFO, __func__, "Unable to load page");
            return NULL;
        }
        if(!(pstack = pst_init(block_size, page_index, pstack))){
            logger(LL_ERROR, __func__, "Unable to initialize pstack");
            return NULL;
        }
    }
    else {
        if(!(pstack = ch_load_page(page_index))) {
            logger(LL_INFO, __func__, "Unable to load page");
            return NULL;
        }
    }
    return pstack;
}

/**
 * @brief Load current pstack
 * @param pstl
 * @return PStack* The current pstack or NULL
 */

PStack* pst_load_current(PStack_List* pstl){
    logger(LL_INFO, __func__, "Loading current pstack");
    PStack* current;
    if(!(current= ch_load_page(pstl->current_idx))){
        logger(LL_ERROR, __func__, "Unable to load current PStack");
        return NULL;
    }
    return current;
}

PStack* pst_go_to_head(PStack_List* pstl){
    logger(LL_INFO, __func__, "Going to head of pstack list");
    PStack* current;

    if (!(current= pst_load_current(pstl))){
        logger(LL_ERROR, __func__, "Unable to load current PStack");
        return NULL;
    }

    while(current->next_page != -1){
        if(pst_list_expand(pstl) == PSTACK_FAIL){
            logger(LL_ERROR, __func__, "Unable to expand PStack List");
            return NULL;
        }
        if (!(current= pst_load_current(pstl))){
            logger(LL_ERROR, __func__, "Unable to load current PStack");
            return NULL;
        }
    }

    if(current->size == current->capacity){
        if(pst_list_expand(pstl) == PSTACK_FAIL){
            logger(LL_ERROR, __func__, "Unable to expand PStack List");
            return NULL;
        }
        if (!(current= pst_load_current(pstl))){
            logger(LL_ERROR, __func__, "Unable to load current PStack");
            return NULL;
        }
    }

    return current;
}


/**
 * @brief Destroys pstack
 *
 * @param box The pstack to destroy
 * @return int PSTACK_SUCCESS or PSTACK_FAIL
 */

int pst_destroy(PStack* pst) {
    logger(LL_INFO, __func__, "Destroying pstack");
    ch_delete_page(pst->page_index);
    return PSTACK_SUCCESS;
}

/**
 * Expand PStack List
 * @param pstl
 * @return PSTACK_SUCCESS or PSTACK_FAIL
 */

int pst_list_expand(PStack_List* pstl){
    logger(LL_INFO, __func__, "Expanding pstack list");

    // Loading current pstack
    PStack* current = pst_load_current(pstl);
    if(!(current)){
        logger(LL_ERROR, __func__, "Unable to load current PStack");
        return PSTACK_FAIL;
    }

    uint64_t new_page_idx;
    uint64_t block_size = current->block_size;
    int64_t page_idx = current->page_index;

    PStack* new_pst;
    if(current->next_page != -1){
        new_page_idx = current->next_page;
        new_pst = pst_load(block_size, (int64_t) new_page_idx);
    }
    else {
        if((new_page_idx = ch_new_page()) == CachingFail){
            logger(LL_ERROR, __func__, "Unable to load new page");
            return PSTACK_FAIL;
        }
        new_pst = pst_load(block_size, (int64_t) new_page_idx);
        if(!(pst_init(block_size, (int64_t)new_page_idx, new_pst))){
            logger(LL_ERROR, __func__, "Unable to initialize new pstack");
            return PSTACK_FAIL;
        }
    }

    if(!new_pst){
        logger(LL_ERROR, __func__, "Unable to load new pstack");
        return PSTACK_FAIL;
    }
    new_pst->prev_page = page_idx;
    current->next_page = (int64_t)new_page_idx;
    pstl->current_idx = (int64_t)new_page_idx;
    return PSTACK_SUCCESS;
}

/**
 * Reduce PStack List
 * @param pstl
 * @return PSTACK_SUCCESS or PSTACK_FAIL
 */

int pst_list_reduce(PStack_List* pstl){
    logger(LL_INFO, __func__, "Reducing pstack list");

    PStack* current = pst_load_current(pstl);

    void* page;
    if(!(page = ch_load_page(current->prev_page))){
        logger(LL_ERROR, __func__, "Unable to load previous PStack");
        return PSTACK_FAIL;
    }
    pst_destroy(current);
    current = (PStack*) page;
    current->next_page = -1;
    pstl->current_idx = current->page_index;
    return PSTACK_SUCCESS;
}

/**
 * @brief Pushes data to pstack list
 * @param box  The pstack list to push data to
 * @param data  The data to push
 * @param size  The size of the data
 * @return  PSTACK_SUCCESS or PSTACK_FAIL
 */

int pst_push(PStack_List* pstl, void* data){
    logger(LL_INFO, __func__, "Pushing data to pstack");

    // Loading current pstack
    PStack* current = pst_load_current(pstl);
    if(!(current)){
        logger(LL_ERROR, __func__, "Unable to load current PStack");
        return PSTACK_FAIL;
    }

    // Going to head of pstack list
    if(current->size == current->capacity){
        logger(LL_INFO, __func__, "PStack is full, trying to expand");
        current = pst_go_to_head(pstl);
    }

    size_t size = current->block_size;
    unsigned long offset = current->block_size * current->size + sizeof_PStack_Header;
    if(ch_write(current->page_index, data, size, (off_t)offset) == CachingFail){
        logger(LL_ERROR, __func__, "Unable to write to pstack");
        return PSTACK_FAIL;
    }
    current->size++;
    if(current->size == current->capacity){
        if(pst_list_expand(pstl) == PSTACK_FAIL){
            logger(LL_ERROR, __func__, "Unable to expand PStack List");
            return PSTACK_FAIL;
        }
    }
    return PSTACK_SUCCESS;
}

/**
 *
 * @brief Pops data from pstack_list
 *
 * @param box  The pstack_list to pop data from
 * @param data  The data to pop
 * @param size  The size of the data
 * @return  PSTACK_SUCCESS or PSTACK_FAIL or PSTACK_EMPTY
 */

int pst_pop(PStack_List* pstl, void* data){
    logger(LL_INFO, __func__, "Popping data from pstack");

    // Loading current pstack
    PStack* current = pst_load_current(pstl);
    if(!(current)){
        logger(LL_ERROR, __func__, "Unable to load current PStack");
        return PSTACK_FAIL;
    }

    // Going to head of pstack list
    if(current->size == current->capacity){
        logger(LL_INFO, __func__, "PStack is full, trying to expand");
        current = pst_go_to_head(pstl);
    }



    if(current->size == 0){
        if(current->prev_page != -1){
            pst_list_reduce(pstl);
            current = pst_load_current(pstl);
        }
        else {
            logger(LL_INFO, __func__, "PStack is empty");
            return PSTACK_EMPTY;
        }
    }
    unsigned long offset = current->block_size * (current->size - 1) + sizeof_PStack_Header;
    if(ch_copy_read(current->page_index, data, current->block_size, (off_t)offset) == CachingFail){
        logger(LL_ERROR, __func__, "Unable to read from pstack");
        return PSTACK_FAIL;
    }
    current->size--;

    return PSTACK_SUCCESS;
}

/**
 * PStack_List Initialization
 * @param block_size
 * @param start_page_index
 * @return pointer to pStackList or NULL
 */

PStack_List* pst_list_init(size_t block_size, int64_t start_page_index){
    logger(LL_INFO, __func__, "Initializing pstack list");

    PStack_List* pStackList = malloc(sizeof(PStack_List));
    PStack* pst = pst_load(block_size, start_page_index);
    if(!pst){
        logger(LL_ERROR, __func__, "Unable to create a PStack");
        pst_list_destroy(pStackList);
        return NULL;
    }
    pStackList->current_idx = start_page_index;
    return pStackList;
}

/**
 * PStack_List Destroy
 * @param pStackList
 * @return PSTACK_SUCCESS or PSTACK_FAIL
 */

int pst_list_destroy(PStack_List* pStackList){
    logger(LL_INFO, __func__, "Destroying pstack list");
    free(pStackList);
    return PSTACK_SUCCESS;
}



