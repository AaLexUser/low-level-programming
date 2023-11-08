#include "caching.h"
#include "../../utils/hashtable.h"
#include "../../utils/logger.h"
#include "../../utils/roundup.h"
#include <inttypes.h>
#include <math.h>
#include <time.h>

// flag = 1 - occupied flag = 2 - removed_from_cache flag = 3 - deleted flag = 0 - unknown

typedef struct{
    size_t size, used, max_used, capacity;
    uint32_t* usage_count;
    time_t* last_used;
    void** cached_page_ptr;
    char* flags;
} Cacher;

Cacher ch;


/**
 * Cacher initialization
 * @return CachingSuccess or CachingFail
 */
int ch_init(){
    logger(LL_INFO, __func__ , "Caching initialization.");
    ch.size = ch.used = ch.max_used = ch.capacity = 0;
    ch.usage_count = NULL;
    ch.last_used = NULL;
    ch.cached_page_ptr = NULL;
    ch.flags = NULL;
    return CachingSuccess;
}

size_t ch_size() {return ch.size;}
size_t ch_used() {return ch.used;}
void* ch_cached_page(size_t index) {return ch.cached_page_ptr[index];}
size_t ch_page_index(off_t offset) {return get_page_index(offset);}
size_t get_ch_usage_memory_space(){
    return PAGE_SIZE * ch.size;
}
int ch_page_status(size_t index) {return index <= ch.capacity ? ch.flags[index]: -1;}

/**
 * Reserve new capacity for cacher.
 * @param new_capacity
 * @return CachingSuccess or CachingFail
 */

int ch_reserve(size_t new_capacity){
    if((new_capacity) <= ch.max_used) {
       return CachingSuccess;
    }
    logger(LL_INFO, __func__, "Reserving new cacher capacity: %ld -> %ld.", ch.capacity, new_capacity);
    size_t ch_new_capacity = new_capacity;
    roundupsize(ch_new_capacity);
    if(ch_new_capacity < new_capacity){
        logger(LL_ERROR, __func__, "Integer overflow while reserving new cacher capacity: %ld.", new_capacity);
        return CachingFail;
    }
    size_t ch_new_max_used = (ch_new_capacity >> 1) + (ch_new_capacity >> 2); /*3/4 of the new capacity*/
    if (ch_new_max_used < (new_capacity)){
        ch_new_capacity <<= 1;
        if (ch_new_capacity < (new_capacity))  {
            logger(LL_ERROR, __func__, "Integer overflow while reserving new cacher capacity: %ld.", new_capacity);
            return CachingFail;
        }
        ch_new_max_used = (ch_new_capacity >> 1) + (ch_new_capacity >> 2);
    }
    char *ch_new_flags = malloc(ch_new_capacity*sizeof(char));
    if(!ch_new_flags){
        logger(LL_ERROR, __func__, "Unable allocate new flags for cacher.");
        return CachingFail;
    }
    void** ch_new_cached_page_ptr = malloc(ch_new_capacity*sizeof(void*));
    if(!ch_new_cached_page_ptr){
        free(ch_new_flags);
        logger(LL_ERROR, __func__, "Unable allocate new cached_page_ptr for cacher.");
        return CachingFail;
    }
    uint32_t* ch_new_usage_count = malloc(ch_new_capacity*sizeof(uint32_t));
    if(!ch_new_usage_count){
        free(ch_new_flags);
        free(ch_new_cached_page_ptr);
        logger(LL_ERROR, __func__, "Unable allocate new usage_count for cacher.");
        return CachingFail;
    }
    time_t* ch_new_last_used = malloc(ch_new_capacity * sizeof(time_t));
    if(!ch_new_last_used){
        free(ch_new_flags);
        free(ch_new_cached_page_ptr);
        free(ch_new_usage_count);
        logger(LL_ERROR, __func__, "Unable allocate new last_used for cacher.");
        return CachingFail;
    }
    memset(ch_new_last_used, 0, ch_new_capacity);
    memset(ch_new_flags, 0, ch_new_capacity);
    memset(ch_new_usage_count, 0, ch_new_capacity);
    for(size_t ch_i = 0; ch_i < ch.capacity; ch_i++){
        if(ch.flags[ch_i] != 1) continue;
        ch_new_flags[ch_i] = 1;
        ch_new_cached_page_ptr[ch_i] = ch.cached_page_ptr[ch_i];
        ch_new_usage_count[ch_i] = ch.usage_count[ch_i];
        ch_new_last_used[ch_i] = ch.last_used[ch_i];
    }
    free(ch.flags);
    free(ch.cached_page_ptr);
    ch.flags = ch_new_flags;
    ch.cached_page_ptr = ch_new_cached_page_ptr;
    ch.capacity = ch_new_capacity;
    ch.max_used = ch_new_max_used;
    ch.usage_count = ch_new_usage_count;
    ch.last_used = ch_new_last_used;
    ch.used = ch.size;
    return CachingSuccess;
}

/**
 * Put page to cacher
 * @param page_index
 * @param mapped_page_ptr
 * @return CachingSuccess or CachingFail
 */

int ch_put(uint64_t page_index, void* mapped_page_ptr){
    logger(LL_INFO, __func__, "Putting page %ld to cache", page_index);
    if(get_ch_usage_memory_space() >= CH_MAX_MEMORY_USAGE){
        uint64_t count = ch_unmap_some_pages();
        logger(LL_INFO, __func__, "Unmaped %ld pages", count);
    }
    size_t ch_new_size = ch.size ? ch.size : 2;
    ch_new_size = (page_index <= ch_new_size) ? ch_new_size : page_index + 1;
    if(ch_reserve(ch_new_size) == CachingFail){
        logger(LL_ERROR, __func__ , "Unable to reserve cacher capacity.");
        return CachingFail;
    }
    if(ch.flags[page_index] == 1){
        return CachingSuccess;
    }
    else{
        ch.flags[page_index] = 1;
        ch.cached_page_ptr[page_index] = mapped_page_ptr;
        ch.size++;
    }
    return CachingSuccess;
}

/**
 * Get page from cacher
 * @param page_index
 * @return pointer to page or NULL
 */

void* ch_get(uint64_t page_index){
    if(!ch.size){
        logger(LL_INFO, __func__ , "Cacher size is 0.");
        return NULL;
    }
    if(page_index > ch.capacity){
        logger(LL_INFO, __func__, "Requesting not existing key");
        return NULL;
    }
    if(ch.flags[page_index] != 1){
        return NULL;
    }
    ch.usage_count[page_index]++;
    time_t now;
    ch.last_used[page_index] = time(&now);
    return ch.cached_page_ptr[page_index];
}
/**
 * Remove page from cache
 * @param index
 * @return CachingSuccess or CachingFail
 */

int ch_remove(uint64_t index){
    logger(LL_INFO, __func__, "Removing page %ld from cache", index);
    void* page = NULL;
    if(!(page = ch_get(index) )){
        logger(LL_ERROR, __func__, "Unable to get a page %ld", index);
        return CachingFail;
    }
    if(unmap_page(page) == -1){
        logger(LL_ERROR, __func__, "Unable to unmap page %ld", index);
        return CachingFail;
    }
    ch.size--;
    ch.flags[index] = 2;
    ch.cached_page_ptr[index] = NULL;
    return CachingSuccess;
}

/**
 * Mapping new page and caching it.
 * @return new page index or CachingFail
 */

int64_t ch_new_page(){
    logger(LL_INFO, __func__, "Requesting new page");
    if (init_page() == FILE_FAIL) {
        logger(LL_ERROR, __func__, "Unable to init page");
        return CachingFail;
    }
    int64_t page_index = get_current_page_index();
    void* mmaped_page_ptr = get_cur_mmaped_data();
    ch_put(page_index, mmaped_page_ptr);
    return page_index;
}

/**
 * Load page from Cache or from File
 * @param page_index
 * @return pointer to page or NULL
 */

void* ch_load_page(uint64_t page_index){
    logger(LL_INFO, __func__, "Loading page %ld", page_index);
    void* page = NULL;
    if((page = ch_get(page_index)) != NULL){
        return page;
    }
    if(page_index >= get_file_size()){
        logger(LL_ERROR, __func__, "chunk_t index is out of file range");
        return NULL;
    }
    mmap_page(get_page_offset(page_index));
    void* mmaped_page_ptr = get_cur_mmaped_data();
    ch_put(page_index, mmaped_page_ptr);

    //Increase usage
    ch.usage_count[page_index]++;
    time_t now;
    ch.last_used[page_index] = time(&now);

    return mmaped_page_ptr;
}
/**
 * Write on page
 * @param page_index
 * @param src
 * @param size
 * @param offset
 * @return CachingSuccess or CachingFail
 */

int ch_write(uint64_t page_index, void* src, size_t size, off_t offset){
    logger(LL_INFO, __func__,
           "Writing to page %ld on offset %ld, size %ld bytes.", page_index, offset, size);

    void* page = NULL;
    if(page_index > get_max_page_index()){
        logger(LL_ERROR, __func__, "chunk_t index is out of range");
        return CachingFail;
    }
    if(!(page = ch_load_page(page_index))){
        return CachingFail;
    }

    memcpy(page + offset, src, size);
    if(sync_page(page) == -1){
        return CachingFail;
    };

    //Increase usage
    ch.usage_count[page_index]++;
    time_t now;
    ch.last_used[page_index] = time(&now);

    return CachingSuccess;
}

/**
 * Clear page
 * @param page_index
 * @return CachingSuccess or CachingFail
 */

int ch_clear_page(uint64_t page_index){
    logger(LL_INFO, __func__, "Clearing page %ld", page_index);
    void* page = NULL;
    if(page_index > get_max_page_index()){
        logger(LL_ERROR, __func__, "chunk_t index is out of range");
        return CachingFail;
    }
    if(!(page = ch_load_page(page_index))){
        logger(LL_ERROR, __func__, "Unable to load page %ld", page_index);
        return CachingFail;
    }
    memset(page, 0, PAGE_SIZE);
    sync_page(page);
    return CachingSuccess;
}


/**
 * Make copy to dest from page
 * @param page_index
 * @param dest
 * @param size
 * @param offset
 * @return CachingSuccess or CachingFail
 */

int ch_copy_read(uint64_t page_index, void* dest, size_t size, off_t offset){
    void* page = NULL;
    if(!(page = ch_load_page(page_index))){
        return CachingFail;
    }
    memcpy(dest, page + offset, size);
    sync_page(page);

    //Increase usage
    ch.usage_count[page_index]++;
    time_t now;
    ch.last_used[page_index] = time(&now);

    return CachingSuccess;
}

/**
 * Unsafe read from page
 * @param page_index
 * @param dest
 * @param offset
 * @return CachingSuccess or CachingFail
 */

void* ch_read(uint64_t page_index, off_t offset){
    void* page = NULL;
    if(!(page = ch_load_page(page_index))){
        return NULL;
    }

    //Increase usage
    ch.usage_count[page_index]++;
    time_t now;
    ch.last_used[page_index] = time(&now);

    return page + offset;
}


uint64_t ch_begin(){return 0;}
uint64_t ch_end(){return ch.capacity;}

static uint64_t ch_nearest_cached_index(const char *flags, size_t capacity, uint64_t index){
    while (index < capacity && flags[index] != 1) {
        index++;
    }
    return index;
}

bool ch_cached(uint64_t index){
    return ch.flags[index] == 1;
}

#define ch_for_each_cached(index) for ( \
size_t index = ch_nearest_cached_index(ch.flags, ch.capacity, ch_begin());\
(index) != ch_end() && ch_cached(index);                                  \
(index)++, (index) = ch_nearest_cached_index(ch.flags, ch.capacity, (index)) \
)                                \

bool ch_valid(uint64_t index){
    return  ch.flags[index] == 1 || ch.flags[index] == 2;
}

static uint64_t ch_nearest_valid_index(const char *flags, size_t capacity, uint64_t index){
    while (index < capacity && !ch_valid(index)){
        index++;
    }
    return index;
}

#define ch_for_each_valid(index) for ( \
size_t index = ch_nearest_cached_valid(ch.flags, ch.capacity, ch_begin());\
(index) != ch_end() && ch_valid(index);                                  \
(index)++, (index) = ch_nearest_valid_index(ch.flags, ch.capacity, (index)) \
)                                \


/**
 * Cacher destroy
 * @return CachingSuccess
 */
int ch_destroy(){
    logger(LL_INFO, __func__ , "Cacher destroy");
    ch.size = ch.used = ch.max_used = ch.capacity = 0;
    ch_for_each_cached(index){
        ch_remove(index);
    }
    free(ch.usage_count);
    free(ch.cached_page_ptr);
    free(ch.flags);
    return CachingSuccess;
}


/**
 * Find least used count
 * @return  least used count
 */

uint32_t ch_find_least_used_count(){
    uint32_t min_usage = UINT32_MAX;
    ch_for_each_cached(index){
        if(ch.usage_count[index] < min_usage){
            min_usage = ch.usage_count[index];
        }
    }
    return min_usage;
}

/**
 * Find least used time
 * @return  least used time
 */

time_t ch_find_least_used_time(){
    time_t min_time = time(NULL);
    ch_for_each_cached(index){
        if(ch.last_used[index] < min_time){
            min_time = ch.last_used[index];
        }
    }
    return min_time;
}


/**
 * Unmap some pages
 * @brief Unmapping pages with smallest usage count
 * @return number of unmapped pages
 */

uint64_t ch_unmap_some_pages(){
    logger(LL_INFO, __func__, "chunk_t unmapping start");
    uint64_t unmap_count = 0;
    uint64_t usage_c = 0;
    uint64_t min_time = ch_find_least_used_time();
    double time_threshold = 2; // 2 sec
    double execute_count = 0;
    while (unmap_count == 0) {
        execute_count++;
        ch_for_each_cached(index) {
            if ((double)ch.last_used[index] < ((double)min_time + time_threshold)) {
                if (ch_remove(index) != CachingFail) {
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
 * Delete last page
 * @brief Delete last page from file
 * @return CachingSuccess or CachingFail
 */

int ch_delete_last_page(){
    if(get_file_size() == 0){
        return CachingSuccess;
    }
    int64_t page_index = get_max_page_index();
    if(ch.flags[page_index] != 3){
        logger(LL_ERROR, __func__, "Last page is not marked as deleted");
        return CachingFail;
    }
    logger(LL_INFO, __func__, "Deleting page %ld", page_index);
    if(delete_last_page() == FILE_FAIL){
        logger(LL_ERROR, __func__, "Unable to delete last page");
        return CachingFail;
    }
    return CachingSuccess;
}

/**
 * Mark as deleted page
 * @brief Mark page as deleted
 * @param page_index
 * @return CachingSuccess or CachingFail
 */

int ch_delete_page(uint64_t page_index){
    if(get_file_size() == 0){
        logger(LL_ERROR, __func__, "File is empty");
        return CachingFail;
    }
    if(page_index > get_max_page_index()){
        logger(LL_ERROR, __func__, "chunk_t index is out of range");
        return CachingFail;
    }
    logger(LL_INFO, __func__, "Deleting page %ld", page_index);
    ch_clear_page(page_index);
    if(ch_remove(page_index) == CachingFail){ // Remove page from cache
        logger(LL_ERROR, __func__, "Unable to remove page %ld from cache", page_index);
        return CachingFail;
    }
    ch.flags[page_index] = 3; // Mark page as deleted
    if(page_index == get_max_page_index()){
        ch_delete_last_page();
    }
    return CachingSuccess;
}


