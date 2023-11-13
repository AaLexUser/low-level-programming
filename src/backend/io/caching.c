#include "caching.h"
#include "../../utils/hashtable.h"
#include "../../utils/logger.h"
#include "../../utils/roundup.h"
#include <inttypes.h>
#include <math.h>
#include <time.h>

// flag = 1 - occupied flag = 2 - removed_from_cache flag = 3 - deleted flag = 0 - unknown

/**
 * @brief   Get current file size
 * @param[in]   ch: pointer to caching_t
 * @return  file size
 */

off_t ch_file_size(caching_t* ch){
    return fl_file_size(&ch->file);
}

/**
 * @brief   Get current max page index
 * @param[in]   ch: pointer to caching_t
 * @return  max page index
 */

int64_t ch_max_page_index(caching_t* ch){
    return fl_max_page_index(&ch->file);
}

/**
 * @brief   Get number of pages in file
 * @param[in]   ch: pointer to caching_t
 * @return  number of pages
 */

uint64_t ch_number_pages(caching_t* ch){
    return fl_number_pages(&ch->file);
}

/**
 * @brief   Get page index by offset
 * @param   page_offset: offset of page in file
 * @return  page index
 */

uint64_t ch_page_index(off_t page_offset){
    return fl_page_index(page_offset);
}

/**
 * @brief   Get page offset by index
 * @param   page_index: index of page in file
 * @return  page offset
 */

off_t ch_page_offset(uint64_t page_index){
    return fl_page_offset(page_index);
}


/**
 * @brief       Cacher initialization
 * @param[in]   file_name: file name
 * @param[out]  ch: pointer to caching_t
 * @return      CH_SUCCESS on success, CH_FAIL on failure
 */
 
int ch_init(const char* file_name, caching_t* ch){
    logger(LL_INFO, __func__ , "Caching initialization.");
    if(init_file(file_name, &ch->file) == FILE_FAIL){
        logger(LL_ERROR, __func__ , "Unable to init file.");
        return CH_FAIL;
    }
    ch->size = ch->used = ch->max_used = ch->capacity = 0;
    ch->usage_count = NULL;
    ch->last_used = NULL;
    ch->cached_page_ptr = NULL;
    ch->flags = NULL;
    return CH_SUCCESS;
}

size_t ch_size(caching_t* ch) {return ch->size;}
size_t ch_used(caching_t* ch) {return ch->used;}
void* ch_cached_page(caching_t* ch, size_t index) {return ch->cached_page_ptr[index];}
size_t ch_usage_memory_space(caching_t* ch){
    return PAGE_SIZE * ch->size;
}
int ch_page_status(caching_t* ch, size_t index) {return index <= ch->capacity ? ch->flags[index]: -1;}

/**
 * @brief       Reserve new capacity for cacher.
 * @param[in]   ch: pointer to caching_t
 * @param[in]   new_capacity: new capacity
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

int ch_reserve(caching_t* ch, size_t new_capacity){
    if((new_capacity) <= ch->max_used) {
       return CH_SUCCESS;
    }
    logger(LL_INFO, __func__, "Reserving new cacher capacity: %ld -> %ld.", ch->capacity, new_capacity);
    size_t ch_new_capacity = new_capacity;
    roundupsize(ch_new_capacity);
    if(ch_new_capacity < new_capacity){
        logger(LL_ERROR, __func__, "Integer overflow while reserving new cacher capacity: %ld.", new_capacity);
        return CH_FAIL;
    }
    size_t ch_new_max_used = (ch_new_capacity >> 1) + (ch_new_capacity >> 2); /*3/4 of the new capacity*/
    if (ch_new_max_used < (new_capacity)){
        ch_new_capacity <<= 1;
        if (ch_new_capacity < (new_capacity))  {
            logger(LL_ERROR, __func__, "Integer overflow while reserving new cacher capacity: %ld.", new_capacity);
            return CH_FAIL;
        }
        ch_new_max_used = (ch_new_capacity >> 1) + (ch_new_capacity >> 2);
    }
    char *ch_new_flags = malloc(ch_new_capacity*sizeof(char));
    if(!ch_new_flags){
        logger(LL_ERROR, __func__, "Unable allocate new flags for cacher.");
        return CH_FAIL;
    }
    void** ch_new_cached_page_ptr = malloc(ch_new_capacity*sizeof(void*));
    if(!ch_new_cached_page_ptr){
        free(ch_new_flags);
        logger(LL_ERROR, __func__, "Unable allocate new cached_page_ptr for cacher.");
        return CH_FAIL;
    }
    uint32_t* ch_new_usage_count = malloc(ch_new_capacity*sizeof(uint32_t));
    if(!ch_new_usage_count){
        free(ch_new_flags);
        free(ch_new_cached_page_ptr);
        logger(LL_ERROR, __func__, "Unable allocate new usage_count for cacher.");
        return CH_FAIL;
    }
    time_t* ch_new_last_used = malloc(ch_new_capacity * sizeof(time_t));
    if(!ch_new_last_used){
        free(ch_new_flags);
        free(ch_new_cached_page_ptr);
        free(ch_new_usage_count);
        logger(LL_ERROR, __func__, "Unable allocate new last_used for cacher.");
        return CH_FAIL;
    }
    memset(ch_new_last_used, 0, ch_new_capacity);
    memset(ch_new_flags, 0, ch_new_capacity);
    memset(ch_new_usage_count, 0, ch_new_capacity);
    for(size_t ch_i = 0; ch_i < ch->capacity; ch_i++){
        if(ch->flags[ch_i] != 1) continue;
        ch_new_flags[ch_i] = 1;
        ch_new_cached_page_ptr[ch_i] = ch->cached_page_ptr[ch_i];
        ch_new_usage_count[ch_i] = ch->usage_count[ch_i];
        ch_new_last_used[ch_i] = ch->last_used[ch_i];
    }
    free(ch->flags);
    free(ch->cached_page_ptr);
    ch->flags = ch_new_flags;
    ch->cached_page_ptr = ch_new_cached_page_ptr;
    ch->capacity = ch_new_capacity;
    ch->max_used = ch_new_max_used;
    ch->usage_count = ch_new_usage_count;
    ch->last_used = ch_new_last_used;
    ch->used = ch->size;
    return CH_SUCCESS;
}

/**
 * @brief       Put page to cacher
 * @param[in]   ch: pointer to caching_t 
 * @param[in]   page_index: index of page
 * @param[in]   mapped_page_ptr: pointer to mmaped page
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

int ch_put(caching_t* ch, uint64_t page_index, void* mapped_page_ptr){
    logger(LL_INFO, __func__, "Putting page %ld to cache", page_index);
    if(ch_usage_memory_space(ch) >= CH_MAX_MEMORY_USAGE){
        uint64_t count = ch_unmap_some_pages(ch);
        logger(LL_INFO, __func__, "Unmaped %ld pages", count);
    }
    size_t ch_new_size = ch->size ? ch->size : 2;
    ch_new_size = (page_index <= ch_new_size) ? ch_new_size : page_index + 1;
    if(ch_reserve(ch, ch_new_size) == CH_FAIL){
        logger(LL_ERROR, __func__ , "Unable to reserve cacher capacity.");
        return CH_FAIL;
    }
    if(ch->flags[page_index] == 1){
        return CH_SUCCESS;
    }
    else{
        ch->flags[page_index] = 1;
        ch->cached_page_ptr[page_index] = mapped_page_ptr;
        ch->size++;
    }
    return CH_SUCCESS;
}

/**
 * @brief       Get page from cacher
 * @param[in]   ch: pointer to caching_t
 * @param[in]   page_index: index of page
 * @return      pointer to page or NULL
 */

void* ch_get(caching_t* ch, uint64_t page_index){
    if(!ch->size){
        logger(LL_INFO, __func__ , "Cacher size is 0.");
        return NULL;
    }
    if(page_index > ch->capacity){
        logger(LL_INFO, __func__, "Requesting not existing key");
        return NULL;
    }
    if(ch->flags[page_index] != 1){
        return NULL;
    }
    ch->usage_count[page_index]++;
    time_t now;
    ch->last_used[page_index] = time(&now);
    return ch->cached_page_ptr[page_index];
}
/**
 * @brief       Remove page from cache
 * @param[in]   ch: pointer to caching_t
 * @param[in]   index: index of page
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

int ch_remove(caching_t* ch, uint64_t index){
    logger(LL_INFO, __func__, "Removing page %ld from cache", index);
    void* page = NULL;
    if(!(page = ch_get(ch, index) )){
        logger(LL_ERROR, __func__, "Unable to get a page %ld", index);
        return CH_FAIL;
    }
    if(unmap_page(page, &ch->file) == -1){
        logger(LL_ERROR, __func__, "Unable to unmap page %ld", index);
        return CH_FAIL;
    }
    ch->size--;
    ch->flags[index] = 2;
    ch->cached_page_ptr[index] = NULL;
    return CH_SUCCESS;
}

/**
 * @brief       Mapping new page and caching it.
 * @param[in]   ch: pointer to caching_t
 * @return      new page index or CH_FAIL
 */

int64_t ch_new_page(caching_t* ch){
    logger(LL_INFO, __func__, "Requesting new page");
    if (init_page(&ch->file) == FILE_FAIL) {
        logger(LL_ERROR, __func__, "Unable to init page");
        return CH_FAIL;
    }
    int64_t page_index = fl_current_page_index(&ch->file);
    void* mmaped_page_ptr = fl_cur_mmaped_data(&ch->file);
    ch_put(ch, page_index, mmaped_page_ptr);
    return page_index;
}

/**
 * @brief       Load page from Cache or from File
 * @param[in]   ch: pointer to caching_t
 * @param[in]   page_index: index of page
 * @return      pointer to page or NULL
 */

void* ch_load_page(caching_t* ch, uint64_t page_index){
    logger(LL_INFO, __func__, "Loading page %ld", page_index);
    void* page = NULL;
    if((page = ch_get(ch, page_index)) != NULL){
        return page;
    }
    if(page_index > ch_max_page_index(ch)){
        logger(LL_ERROR, __func__, "chunk_t index is out of file range");
        return NULL;
    }
    mmap_page(ch_page_offset(page_index), &ch->file);
    void* mmaped_page_ptr = fl_cur_mmaped_data(&ch->file);
    ch_put(ch, page_index, mmaped_page_ptr);

    //Increase usage
    ch->usage_count[page_index]++;
    time_t now;
    ch->last_used[page_index] = time(&now);

    return mmaped_page_ptr;
}
/**
 * @brief       Write on page
 * @param[in]   ch: pointer to caching_t
 * @param[in]   page_index: index of page
 * @param[in]   src: source
 * @param[in]   size: size to write
 * @param[in]   offset: offset in page to write to
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

int ch_write(caching_t* ch, uint64_t page_index, void* src, size_t size, off_t offset){
    logger(LL_INFO, __func__,
           "Writing to page %ld on offset %ld, size %ld bytes.", page_index, offset, size);

    void* page = NULL;
    if(page_index > ch_max_page_index(ch)){
        logger(LL_ERROR, __func__, "chunk_t index is out of range");
        return CH_FAIL;
    }
    if(!(page = ch_load_page(ch, page_index))){
        return CH_FAIL;
    }

    memcpy(page + offset, src, size);
    if(sync_page(page) == -1){
        return CH_FAIL;
    };

    //Increase usage
    ch->usage_count[page_index]++;
    time_t now;
    ch->last_used[page_index] = time(&now);

    return CH_SUCCESS;
}

/**
 * @brief       Clear page
 * @param[in]   ch: pointer to caching_t
 * @param[in]   page_index: index of page
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

int ch_clear_page(caching_t* ch, uint64_t page_index){
    logger(LL_INFO, __func__, "Clearing page %ld", page_index);
    void* page = NULL;
    if(page_index > ch_max_page_index(ch)){
        logger(LL_ERROR, __func__, "chunk_t index is out of range");
        return CH_FAIL;
    }
    if(!(page = ch_load_page(ch, page_index))){
        logger(LL_ERROR, __func__, "Unable to load page %ld", page_index);
        return CH_FAIL;
    }
    memset(page, 0, PAGE_SIZE);
    sync_page(page);
    return CH_SUCCESS;
}


/**
 * @brief       Make copy to dest from page
 * @param[in]   ch: pointer to caching_t
 * @param[in]   page_index: index of page
 * @param[out]  dest: destination
 * @param[in]   size: size to write
 * @param[in]   offset: offset in page to write to
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

int ch_copy_read(caching_t* ch, uint64_t page_index, void* dest, size_t size, off_t offset){
    void* page = NULL;
    if(!(page = ch_load_page(ch, page_index))){
        return CH_FAIL;
    }
    memcpy(dest, page + offset, size);
    sync_page(page);

    //Increase usage
    ch->usage_count[page_index]++;
    time_t now;
    ch->last_used[page_index] = time(&now);

    return CH_SUCCESS;
}

/**
 * @brief       Unsafe read from page
 * @param[in]   ch: pointer to caching_t
 * @param[in]   page_index: index of page
 * @param[in]   dest: destination
 * @param[in]   offset: offset in page to read from
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

void* ch_read(caching_t* ch, uint64_t page_index, off_t offset){
    void* page = NULL;
    if(!(page = ch_load_page(ch, page_index))){
        return NULL;
    }

    //Increase usage
    ch->usage_count[page_index]++;
    time_t now;
    ch->last_used[page_index] = time(&now);

    return page + offset;
}


uint64_t ch_begin(){return 0;}
uint64_t ch_end(caching_t* ch){return ch->capacity;}

static uint64_t ch_nearest_cached_index(const char *flags, size_t capacity, uint64_t index){
    while (index < capacity && flags[index] != 1) {
        index++;
    }
    return index;
}

bool ch_cached(caching_t* ch, uint64_t index){
    return ch->flags[index] == 1;
}

#define ch_for_each_cached(index, ch) for ( \
size_t index = ch_nearest_cached_index(ch->flags, ch->capacity, ch_begin());\
(index) != ch_end(ch) && ch_cached(ch, index);                                  \
(index)++, (index) = ch_nearest_cached_index(ch->flags, ch->capacity, (index)) \
)                                \

bool ch_valid(caching_t* ch, uint64_t index){
    return  ch->flags[index] == 1 || ch->flags[index] == 2;
}

static uint64_t ch_nearest_valid_index(caching_t* ch, const char *flags, size_t capacity, uint64_t index){
    while (index < capacity && !ch_valid(ch, index)){
        index++;
    }
    return index;
}

#define ch_for_each_valid(index, ch) for ( \
size_t index = ch_nearest_cached_valid(ch->flags, ch->capacity, ch_begin());\
(index) != ch_end(ch) && ch_valid(ch, index);                                  \
(index)++, (index) = ch_nearest_valid_index(ch->flags, ch->capacity, (index)) \
)                                \


/**
 * @brief       caching destroy
 * @param[in]   ch: pointer to caching_t
 * @return      CH_SUCCESS
 */

int ch_destroy(caching_t* ch){
    logger(LL_INFO, __func__ , "Caching destroy");
    ch->size = ch->used = ch->max_used = ch->capacity = 0;
    ch_for_each_cached(index, ch){
        ch_remove(ch, index);
    }
    free(ch->usage_count);
    free(ch->cached_page_ptr);
    free(ch->flags);
    return CH_SUCCESS;
}

/**
 * @brief       Delete file and destroy caching
 * @param[in]   ch: pointer to caching_t
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

int ch_delete(caching_t* ch){
    logger(LL_INFO, __func__ , "Deleting file");
    ch_destroy(ch);
    return delete_file(&ch->file);
}

/**
 * @brief       Close file and destroy caching
 * @param[in]   ch: pointer to caching_t
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

int ch_close(caching_t* ch){
    logger(LL_INFO, __func__ , "Closing file");
    ch_destroy(ch);
    return close_file(&ch->file);
}


/**
 * @brief       Find least used count
 * @param[in]   ch: pointer to caching_t
 * @return      least used count
 */

uint32_t ch_find_least_used_count(caching_t* ch){
    uint32_t min_usage = UINT32_MAX;
    ch_for_each_cached(index, ch){
        if(ch->usage_count[index] < min_usage){
            min_usage = ch->usage_count[index];
        }
    }
    return min_usage;
}

/**
 * @brief       Find least used time
 * @param[in]   ch: pointer to caching_t
 * @return      least used time
 */

time_t ch_find_least_used_time(caching_t* ch){
    time_t min_time = time(NULL);
    ch_for_each_cached(index, ch){
        if(ch->last_used[index] < min_time){
            min_time = ch->last_used[index];
        }
    }
    return min_time;
}


/**
 * @brief       Unmapping pages with smallest usage count
 * @param[in]   ch: pointer to caching_t
 * @return      number of unmapped pages
 */

uint64_t ch_unmap_some_pages(caching_t* ch){
    logger(LL_INFO, __func__, "chunk_t unmapping start");
    uint64_t unmap_count = 0;
    uint64_t usage_c = 0;
    uint64_t min_time = ch_find_least_used_time(ch);
    double time_threshold = 2; // 2 sec
    double execute_count = 0;
    while (unmap_count == 0) {
        execute_count++;
        ch_for_each_cached(index, ch) {
            if ((double)ch->last_used[index] < ((double)min_time + time_threshold)) {
                if (ch_remove(ch, index) != CH_FAIL) {
                    logger(LL_INFO, __func__, "Unmapped page %ld", index);
                    unmap_count++;
                }
            }
        }
        time_threshold = time_threshold + pow(2, execute_count);
    }
    logger(LL_INFO, __func__, "Unmapped %ld pages with usage %ld", unmap_count, usage_c);
    return unmap_count;
}


/**
 * @brief       Delete last page from file
 * @param[in]   ch: pointer to caching_t
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

int ch_delete_last_page(caching_t* ch){
    if(ch_file_size(ch) == 0){
        return CH_SUCCESS;
    }
    int64_t page_index = ch_max_page_index(ch);
    if(ch->flags[page_index] != 3){
        logger(LL_ERROR, __func__, "Last page is not marked as deleted");
        return CH_FAIL;
    }
    logger(LL_INFO, __func__, "Deleting page %ld", page_index);
    if(delete_last_page(&ch->file) == FILE_FAIL){
        logger(LL_ERROR, __func__, "Unable to delete last page");
        return CH_FAIL;
    }
    return CH_SUCCESS;
}

/**
 * @brief       Mark page as deleted
 * @param[in]   ch: pointer to caching_t
 * @param       page_index: index of page
 * @return      CH_SUCCESS on success, CH_FAIL otherwise
 */

int ch_delete_page(caching_t* ch, uint64_t page_index){
    if(ch_file_size(ch) == 0){
        logger(LL_ERROR, __func__, "File is empty");
        return CH_FAIL;
    }
    if(page_index > ch_max_page_index(ch)){
        logger(LL_ERROR, __func__, "chunk_t index is out of range");
        return CH_FAIL;
    }
    logger(LL_INFO, __func__, "Deleting page %ld", page_index);
    ch_clear_page(ch, page_index);
    if(ch_remove(ch, page_index) == CH_FAIL){ // Remove page from cache
        logger(LL_ERROR, __func__, "Unable to remove page %ld from cache", page_index);
        return CH_FAIL;
    }
    ch->flags[page_index] = 3; // Mark page as deleted
    if(page_index == ch_max_page_index(ch)){
        ch_delete_last_page(ch);
    }
    return CH_SUCCESS;
}


