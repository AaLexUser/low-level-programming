#include "file.h"
#include "../../utils/logger.h"
#include <fcntl.h>
#include <inttypes.h>
#include <stdlib.h>

off_t fl_cur_page_offset(file_t* file){
    return file->cur_page_offset;
}

void* fl_cur_mmaped_data(file_t* file){
    return file->cur_mmaped_data;
}

off_t fl_file_size(file_t* file){
    struct stat st;
    stat(file->filename, &st);
    return st.st_size;
}

int64_t fl_max_page_index(file_t* file){
    return (int64_t)(fl_file_size(file) / PAGE_SIZE) - 1;
}

uint64_t fl_number_pages(file_t* file){
    return fl_file_size(file) / PAGE_SIZE;
}

uint64_t fl_page_index(off_t page_offset){
    return page_offset / PAGE_SIZE;
}

off_t fl_page_offset(uint64_t page_index){
    return (off_t)page_index * PAGE_SIZE;
}

int64_t fl_current_page_index(file_t* file){
    return file->cur_page_offset / PAGE_SIZE;
}

#if defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#include <unistd.h>

/**
 * @brief       File initialization
 * @param[in]   filename: name of file
 * @param[out]  file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int init_file(const char* file_name, file_t* file){
    file->filename = (char*)malloc(strlen(file_name)+1);
    strncpy(file->filename, file_name, strlen(file_name)+1);
    logger(LL_INFO, __func__ ,"Opening file %s.", file->filename);
    file->fd = open(file->filename,
                           O_RDWR | // Read/Write mode
                           O_CREAT, // Create file if it does not exist
                           S_IWUSR | // User write permission
                           S_IRUSR // User read permission
    );
    if (file->fd == -1){
        logger(LL_ERROR, __func__ ,"Unable to open file.");
        return FILE_FAIL;
    }

    if(fl_file_size(file) != 0){
        if(mmap_page(file->cur_page_offset, file) == FILE_FAIL){
            logger(LL_ERROR, __func__, "Unable map file");
            return FILE_FAIL;
        }
    }
    return FILE_SUCCESS;
}


/**
 * @brief       Map File
 * @param[in]   offset: offset in file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int mmap_page(off_t offset, file_t* file){
    logger(LL_INFO, __func__,
           "Mapping page from file with descriptor %d and file size %" PRIu64, file->fd, fl_file_size(file));
    if(fl_file_size(file) == 0){
        return FILE_FAIL;
    }

    if((file->cur_mmaped_data = mmap(NULL, PAGE_SIZE,
                                            PROT_WRITE |
                                            PROT_READ,
                           MAP_SHARED, file->fd, offset)) == MAP_FAILED){
        logger(LL_ERROR, __func__ , "Unable to map file: %s %d.", strerror(errno), errno);
        return FILE_FAIL;
    }
    file->cur_page_offset = offset;
    logger(LL_INFO, __func__, "chunk_t %ld mapped on address %p", fl_current_page_index(file), file->cur_mmaped_data);
    return FILE_SUCCESS;
}

/**
 * @brief       Synchronize a mapped region
 * @param[in]   mmaped_data: pointer to mapped data
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
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
 * @brief       Unmap page
 * @param[in]   mmaped_data: pointer to mapped data
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int unmap_page(void* mmaped_data, file_t* file){
    if(fl_file_size(file) == 0){
        return FILE_SUCCESS;
    }

    logger(LL_INFO, __func__,
           "Unmapping page from file with pointer %p and file size %" PRIu64,
           mmaped_data, fl_file_size(file));
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
 * @brief       Close file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int close_file(file_t* file){
    close(file->fd);
    file->fd = -1;
    free(file->filename);
    return FILE_SUCCESS;
}

/**
 * @brief       Delete file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int delete_file(file_t* file){
    logger(LL_INFO, __func__, "Deleting file with name %s.", file->filename);
    if(unlink(file->filename) == -1){
        logger(LL_ERROR, __func__, "Unable delete file with name %s.", file->filename);
        return FILE_FAIL;
    }
    close_file(file);
    return FILE_SUCCESS;
}

/**
 * @brief       Initialize new page
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int init_page(file_t* file){
    logger(LL_INFO, __func__ , "Init new page");
    if(ftruncate(file->fd,  (off_t) (fl_file_size(file) + PAGE_SIZE)) == -1){
        logger(LL_ERROR, __func__, "Unable change file size: %s %d", strerror(errno), errno);
        return FILE_FAIL;
    }
    file->cur_page_offset = fl_file_size(file) - PAGE_SIZE;
    if(mmap_page(file->cur_page_offset, file) == FILE_FAIL){
        logger(LL_ERROR, __func__, "Unable to mmap file.");
    }
    return FILE_SUCCESS;
}

/**
 * @brief       Delete last page in file
 * @param[in]   file: pointer to file_t
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int delete_last_page(file_t* file){
    if(fl_file_size(file) == 0){
        return FILE_SUCCESS;
    }
    logger(LL_INFO, __func__ , "Starting delete page %ld", fl_page_index(fl_file_size(file) - PAGE_SIZE));
    if(ftruncate(file->fd,  (off_t) (fl_file_size(file) - PAGE_SIZE)) == -1){
        logger(LL_ERROR, __func__, "Unable change file size: %s %d", strerror(errno), errno);
        return FILE_FAIL;
    }
    return FILE_SUCCESS;
}

/**
 * @brief       Copies size bytes from memory area src to mapped_file_page and make synchronization with
 *              original file.
 * @param[in]   file: pointer to file_t
 * @param[in]   src: source
 * @param[in]   size: size to write
 * @param[in]   offset: offset in page to write to
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int write_page(file_t* file, void* src, uint64_t size, off_t offset){
    logger(LL_INFO, __func__ , "Writing to fd %d with size %"PRIu64 "%"PRIu64 " bytes.",
           file->fd, fl_file_size(file), size);
    if(file->cur_mmaped_data == NULL){
        logger(LL_ERROR, __func__, "Unable write, mapped file is NULL.");
        return FILE_FAIL;
    }
    memcpy(file->cur_mmaped_data + offset, src, size);
    sync_page(file->cur_mmaped_data);
    return FILE_SUCCESS;
}

/**
 * @brief       Copies size bytes to memory area dest from mapped_file_page.
 * @param[in]   file: pointer to file_t
 * @param[out]  dest: destination
 * @param[in]   size: size to read
 * @param[in]   offset: offset in page to read from
 * @return      FILE_SUCCESS on success, FILE_FAIL otherwise
 */

int read_page(file_t* file, void* dest, uint64_t size, off_t offset){
    logger(LL_INFO, __func__ , "Reading from fd %d with size %"PRIu64 "%"PRIu64 " bytes.",
           file->fd, fl_file_size(file), size);
    if(file->cur_mmaped_data == NULL){
        logger(LL_ERROR, __func__, "Unable write, mapped file is NULL.");
        return FILE_FAIL;
    }
    memcpy(dest, file->cur_mmaped_data + offset, size);
    return FILE_SUCCESS;
}

#endif
#if defined(_WIN32)
#include <windows.h>

HANDLE hFile;

/**
 * \brief       File initialization
 * \param[in]   filename: name of file
 * \return      FILE_SUCCESS on success or FILE_FAIL otherwise
 */

int init_file(const char* file_name) {
    filename = (char*)malloc(strlen(file_name) + 1);
    strncpy(filename, file_name, strlen(file_name) + 1);
    logger(LL_INFO, __func__, "Opening file %s.", filename);

    HANDLE hFile = CreateFile(filename,
        GENERIC_READ | GENERIC_WRITE, // Read/Write mode
        0, // No sharing
        NULL, // Default security attributes
        OPEN_ALWAYS, // Open existing file or create new file
        FILE_ATTRIBUTE_NORMAL, // Normal file
        NULL); // No template file

    if (hFile == INVALID_HANDLE_VALUE) {
        logger(LL_ERROR, __func__, "Unable to open file.");
        return FILE_FAIL;
    }

    LARGE_INTEGER size;
    if (!GetFileSizeEx(hFile, &size)) {
        logger(LL_ERROR, __func__, "Unable to get file size.");
        CloseHandle(hFile);
        return FILE_FAIL;
    }

    file_size = size.QuadPart;

    if (file_size != 0) {
        
    }

    CloseHandle(hFile);
    return FILE_SUCCESS;
}


#endif

