#include <printf.h>
#include "backend/io/caching.h"
#include "backend/io/pager.h"
#include "backend/page_pool/page_pool.h"
#include "backend/io/linked_blocks.h"
#include "utils/logger.h"

#define LOGGER_LEVEL 2

void init_db(const char* filename){
    /* Init new db file */
    if(init_file(filename) != FILE_SUCCESS){
        printf("Failed to init file\n");
        exit(EXIT_FAILURE);
    }
    if(get_file_size() != 0){
        if(delete_file() != 0){
            printf("Failed to delete file\n");
            exit(EXIT_FAILURE);
        }
        if(init_file("test.db") != 0){
            printf("Failed to init file\n");
            exit(EXIT_FAILURE);
        }
    }
}

int out_file(const char* filename){
    int fd = open(filename,
                  O_RDWR | //Read Write mode
                  O_CREAT | // Create file if it does not exist
                  O_TRUNC, // Truncate file
                  S_IWUSR|  // User write permission
                  S_IRUSR // User read permission
    );
    if(fd == -1){
        printf("Failed to open file %s %d %s.", filename, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}

int main(){

    init_db("test.db");

    ch_init();
    pg_init();
    char str[] = "12345678";
    int fd = out_file("linked_blocks_mem.csv");
    FILE *file = fdopen(fd, "w");
    fprintf(file, "FileSize;BlocksCount\n");
    int64_t block_size = 9;
    int64_t ppidx = lb_ppl_init(block_size);
    uint64_t max_file_size =  (uint64_t)1024 * 1024; // 1 mb
    uint64_t allocate_count = 500;
    uint64_t deallocate_count = 400;
    uint64_t block_count = 0;
    while(get_file_size() < max_file_size){
        chblix_t blocks[allocate_count];
        for(int64_t i = 0; i < allocate_count; i++){
            chblix_t block = lb_alloc(ppidx);
            blocks[i] = block;
            block_count++;
            lb_write(ppidx, &block, str, sizeof(str), 0);
        }
        for(int64_t i = 0; i < deallocate_count; i++){
            lb_dealloc(ppidx, &blocks[i]);
            block_count--;
        }
        fprintf(file, "%llu;%llu\n", get_file_size(), block_count);
        fflush(file);
        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", get_file_size(), block_count);
    }
    close(fd);
    pg_destroy();
    ch_destroy();
    delete_file();
}