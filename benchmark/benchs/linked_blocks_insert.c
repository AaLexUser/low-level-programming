#include <printf.h>
#include "backend/io/caching.h"
#include "backend/io/pager.h"
#include "backend/page_pool/page_pool.h"
#include "backend/page_pool/linked_blocks.h"
#include "utils/logger.h"
#include <sys/time.h>
#include <stdio.h>

#ifdef LOGGER_LEVEL
#undef LOGGER_LEVEL
#endif
#define LOGGER_LEVEL 2

void init_db(const char* filename){
    /* Init new db file */
    if(pg_init(filename) != FILE_SUCCESS){
        printf("Failed to init file\n");
        exit(EXIT_FAILURE);
    }
    if(pg_file_size() != 0){
        if(pg_delete() != 0){
            printf("Failed to delete file\n");
            exit(EXIT_FAILURE);
        }
        if(pg_init("test.db") != 0){
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
    struct timespec start, end;

    init_db("test.db");

    char str[] = "2345678";
    int fd = out_file("linked_blocks_insert.csv");
    FILE *file = fdopen(fd, "w");
    fprintf(file, "Time;BlocksCount\n");
    int64_t block_size = 8;
    int64_t ppidx = lb_ppl_init(block_size);
    page_pool_t* ppl = ppl_load(ppidx);
    uint64_t max_file_size =  (uint64_t)1024 * 1024; // 1 mb
    uint64_t allocate_count = max_file_size / ppl->block_size;
    uint64_t block_count = 0;
    while(allocate_count > block_count){
        chblix_t blocks[allocate_count];
        chblix_t block = lb_alloc(ppidx);
        blocks[block_count] = block;
        block_count++;
        clock_gettime(CLOCK_UPTIME_RAW, &start);
        lb_write(ppidx, &block, str, sizeof(str), 0);
        clock_gettime(CLOCK_UPTIME_RAW, &end);
        uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        fprintf(file, "%llu;%llu\n", delta_us, block_count);
        fflush(file);
        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), block_count);
    }
    close(fd);
    pg_delete();
}