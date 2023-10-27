#pragma once
#include "../backend/io/file_manager.h"
#include <stdint.h>
#include <stdlib.h>
typedef struct _SimplePool {
    uint32_t num_of_blocks;
    uint32_t block_size;
    uint32_t num_of_free_blocks;
    uint32_t num_of_used_blocks;
    off_t mem_start;
    off_t next;
    FileManager fileManager;
} SimplePool;

inline SimplePool* create_pool (size_t block_size, uint32_t num_of_blocks){
    SimplePool* pool = (SimplePool*)malloc(sizeof(SimplePool));
    pool->block_size = block_size;
    pool->num_of_blocks = num_of_blocks;
    void* ptr = malloc(block_size*num_of_blocks);
    if(ptr == NULL){
        free(pool);
        return NULL;
    }
    memset(ptr, 0, block_size*num_of_blocks);
    write_file()
    pool->num_of_used_blocks = 0;
    pool->num_of_free_blocks = num_of_blocks;
    pool->
    return pool;
}

inline