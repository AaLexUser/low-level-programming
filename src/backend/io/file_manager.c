#include "file_manager.h"
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>


FileManager* create_file_manager(){
    FileManager* manager = malloc(sizeof(FileManager));
    manager->file_descriptor = -1;
    manager->file_length = 0;
    return manager;
}

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

void* read_file(FileManager* manager, uint32_t offset, uint32_t length){
    void* dest = malloc(length);
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