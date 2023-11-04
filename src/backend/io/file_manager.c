#include "file_manager.h"
#include "../../utils/logger.h"
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/mman.h>

char* filename = NULL;
int fd;
uint64_t file_size;
void* cur_mmaped_data;
off_t cur_page_offset = 0;

off_t get_cur_page_offset(){
    return cur_page_offset;
}

void* get_cur_mmaped_data(){
    return cur_mmaped_data;
}

uint64_t get_file_size(){
    return file_size;
}

int64_t get_max_page_index(){
    return (int64_t)(file_size / PAGE_SIZE) - 1;
}

uint64_t number_pages(){
    return file_size / PAGE_SIZE;
}

uint64_t get_page_index(off_t page_offset){
    return page_offset / PAGE_SIZE;
}

off_t get_page_offset(uint64_t page_index){
    return (off_t)page_index * PAGE_SIZE;
}

int64_t get_current_page_index(){
    return cur_page_offset / PAGE_SIZE;
}


/**
 * File initialization
 * @param filename
 * @return FILE_SUCCESS or FILE_FAIL
 */

int init_file(const char* file_name){
    filename = (char*)malloc(strlen(file_name)+1);
    strncpy(filename, file_name, strlen(file_name)+1);
    logger(LL_INFO, __func__ ,"Opening file %s.", filename);
    fd = open(filename,
                           O_RDWR | // Read/Write mode
                           O_CREAT, // Create file if it does not exist
                           S_IWUSR | // User write permission
                           S_IRUSR // User read permission
    );
    if (fd == -1){
        logger(LL_ERROR, __func__ ,"Unable to open file.");
        return FILE_FAIL;
    }
    file_size = (uint64_t) lseek(fd, 0, SEEK_END);
    if(file_size != 0){
        if(mmap_page(cur_page_offset) == FILE_FAIL){
            logger(LL_ERROR, __func__, "Unable map file");
            return FILE_FAIL;
        }
    }
    return FILE_SUCCESS;
}


/**
 * Map File
 * @param offset
 * @return FILE_SUCCESS or FILE_FAIL
 */

int mmap_page(off_t offset){
    logger(LL_INFO, __func__,
           "Mapping page from file with descriptor %d and file size %" PRIu64, fd, file_size);
    if(file_size == 0){
        return FILE_FAIL;
    }

    if((cur_mmaped_data = mmap(NULL, PAGE_SIZE,
                                            PROT_WRITE |
                                            PROT_READ,
                           MAP_SHARED, fd, offset)) == MAP_FAILED){
        logger(LL_ERROR, __func__ , "Unable to map file: %s %d.", strerror(errno), errno);
        return FILE_FAIL;
    };
    cur_page_offset = offset;
    logger(LL_INFO, __func__, "Page %ld mapped on address %p", get_current_page_index(), cur_mmaped_data);
    return FILE_SUCCESS;
}

/**
 * Synchronize a mapped region
 * @return FILE_SUCCESS or FILE_FAIL
 */

int sync_page(void* mmaped_data){
    logger(LL_INFO, __func__, "Syncing page on address - %p.", mmaped_data);
    if(msync(mmaped_data, PAGE_SIZE, MS_ASYNC) == -1){
        logger(LL_ERROR, __func__ ,
               "Unable sync page on address - %p: %s %d.", mmaped_data, strerror(errno), errno);
        return FILE_FAIL;
    }
    return FILE_SUCCESS;
}

/**
 * Unmap page
 * @return FILE_SUCCESS or FILE_FAIL
 */

int unmap_page(void* mmaped_data){
    if(file_size == 0){
        return FILE_SUCCESS;
    }

    logger(LL_INFO, __func__,
           "Unmapping page from file with pointer %p and file size %" PRIu64,
           mmaped_data, file_size);
    if(sync_page(mmaped_data) == FILE_FAIL){
        return FILE_FAIL;
    }
    if(munmap(mmaped_data, PAGE_SIZE) == -1){
        logger(LL_ERROR, __func__, "Unable unmap page with pointer %p: %s %d.", mmaped_data, strerror(errno), errno);
        return FILE_FAIL;
    };
    mmaped_data = NULL;
    return FILE_SUCCESS;
}

/**
 * Close file
 * @return FILE_SUCCESS or FILE_FAIL
 */

int close_file(){
    close(fd);
    fd = -1;
    file_size = -1;
    free(filename);
    return FILE_SUCCESS;
}

/**
 * Delete file
 * @return FILE_SUCCESS or FILE_FAIL
 */

int delete_file(){
    logger(LL_INFO, __func__, "Deleting file with name %s.", filename);
    if(unlink(filename) == -1){
        logger(LL_ERROR, __func__, "Unable delete file with name %s.", filename);
        return FILE_FAIL;
    }
    close_file();
    return FILE_SUCCESS;
}

/**
 * Truncate or extend a file to a page size
 * @param new_size
 * @return FILE_SUCCESS or FILE_FAIL
 */

int init_page(){
    logger(LL_INFO, __func__ , "Init new page");
    if(ftruncate(fd,  (off_t) (file_size + PAGE_SIZE)) == -1){
        logger(LL_ERROR, __func__, "Unable change file size: %s %d", strerror(errno), errno);
        return FILE_FAIL;
    }
    cur_page_offset = (off_t)file_size;
    file_size += PAGE_SIZE;
    if(mmap_page(cur_page_offset) == FILE_FAIL){
        logger(LL_ERROR, __func__, "Unable to mmap file.");
    }
    return FILE_SUCCESS;
}

/**
 * Delete last page in file
 * @return FILE_SUCCESS or FILE_FAIL
 */

int delete_last_page(){
    if(file_size == 0){
        return FILE_SUCCESS;
    }
    logger(LL_INFO, __func__ , "Starting delete page %ld", get_page_index((off_t)file_size - PAGE_SIZE));
    if(ftruncate(fd,  (off_t) (file_size - PAGE_SIZE)) == -1){
        logger(LL_ERROR, __func__, "Unable change file size: %s %d", strerror(errno), errno);
        return FILE_FAIL;
    }
    file_size -= PAGE_SIZE;
    return FILE_SUCCESS;
}

/**
 * Copies size bytes from memory area src to mapped_file_page and make synchronization with
 * original file.
 * @param src
 * @param size
 * @param offset - page offset
 * @return FILE_SUCCESS or FILE_FAIL
 */

int write_page(void* src, uint64_t size, off_t offset){
    logger(LL_INFO, __func__ , "Writing to fd %d with size %"PRIu64 "%"PRIu64 " bytes.", fd, file_size, size);
    if(cur_mmaped_data == NULL){
        logger(LL_ERROR, __func__, "Unable write, mapped file is NULL.");
        return FILE_FAIL;
    }
    memcpy(cur_mmaped_data + offset, src, size);
    sync_page(cur_mmaped_data);
    return FILE_SUCCESS;
}

/**
 * Copies size bytes to memory area dest from mapped_file_page.
 * @param dest
 * @param size
 * @param offset
 * @return FILE_SUCCESS or FILE_FAIL
 */

int read_page(void* dest, uint64_t size, off_t offset){
    logger(LL_INFO, __func__ , "Reading from fd %d with size %"PRIu64 "%"PRIu64 " bytes.", fd, file_size, size);
    if(cur_mmaped_data == NULL){
        logger(LL_ERROR, __func__, "Unable write, mapped file is NULL.");
        return FILE_FAIL;
    }
    memcpy(dest, cur_mmaped_data + offset, size);
    return FILE_SUCCESS;
}

