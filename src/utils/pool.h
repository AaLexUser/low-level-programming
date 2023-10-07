#ifndef _POOL_H_
#define _POOL_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct _Chunk {
    struct _Chunk* next;
    uint32_t num_of_free_blocks;
    uint32_t num_of_used_blocks;
} Chunk;

typedef struct _Pool {
    uint32_t num_of_blocks;
    uint32_t block_size;
    uint32_t num_of_free_blocks;
    uint32_t num_of_used_blocks;
    void* mem_start;
    void* next;
} Pool;

Pool* pool_create(size_t block_size, uint32_t num_of_blocks);
void pool_destroy(Pool* pool);
void pool_clear(Pool* pool);
unsigned char * pool_addr_from_index(Pool* pool, uint32_t index);
uint32_t pool_index_from_addr(Pool* pool, const void* addr);
bool pool_expand_capacity(Pool* pool, uint32_t num_of_blocks);
void* pool_alloc(Pool* pool); 
void pool_dealloc(Pool* pool, void* p);



#endif