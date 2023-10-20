#include "file_manager.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

/**
 * Create file manager
 * @return pointer to the file manager
 */

FileManager* create_file_manager(){
    FileManager* manager = malloc(sizeof(FileManager));
    manager->file_descriptor = -1;
    manager->file_length = 0;
    return manager;
}

/**
 * Create file manager by filename
 * @param filename
 * @return pointer to the file manager
 */

FileManager* create_file_manager_by_filename(const char* filename){
    FileManager* manager = malloc(sizeof(FileManager));
    manager->file_descriptor = open(filename,
                                    O_RDWR | // Read/Write mode
                                    O_CREAT, // Create file if it does not exist
                                    S_IWUSR | // User write permission
                                    S_IRUSR // User read permission
    );
    if (manager->file_descriptor == -1){
        printf("Unable to open file\n");
        exit(EXIT_FAILURE);
    }
    manager->file_length = lseek(manager->file_descriptor, 0, SEEK_END);
    return manager;
}

void close_file_manager(FileManager* manager){
    close(manager->file_descriptor);
    free(manager);
}

/**
 * Read file from offset to offset + length
 * @param manager - file manager
 * @param offset - offset
 * @param length - length
 * @return - pointer to the memory
 */

void* read_file(FileManager* manager, off_t offset, size_t length){
    void* dest = malloc(length);
    memset(dest, 0, length);
    struct stat statbuf;
    void* src;
    lseek(manager->file_descriptor, offset, SEEK_SET);
    if(fstat(manager->file_descriptor, &statbuf) < 0){
        printf("Error reading file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    if((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, manager->file_descriptor, offset)) == MAP_FAILED){
        printf("Error reading file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    memcpy(dest, src, length);
    return dest;
}

/**
 * Write file from offset to offset + length
 * @param manager - file manager
 * @param offset - offset
 * @param src - pointer to the source memory
 * @param length - length
 */

void write_file(FileManager* manager, off_t offset, void* src, size_t length){
    lseek(manager->file_descriptor, offset, SEEK_SET);
    void* dest;
    struct stat statbuf;
    if(fstat(manager->file_descriptor, &statbuf) < 0){
        printf("Error reading file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    if((dest = mmap(0, statbuf.st_size, PROT_WRITE, MAP_SHARED, manager->file_descriptor, offset)) == MAP_FAILED){
        printf("Error reading file: %d\n", errno);
        exit(EXIT_FAILURE);
    }
    memcpy(dest, src, length);
}