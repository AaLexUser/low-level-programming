#ifndef _POOL_H_
#define _POOL_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct __attribute__((packed)) _Chblidx {
    uint32_t chunkid;
    uint32_t blockid;
} Chblidx;

typedef struct __attribute__((packed)) _ChunkHeader{
    uint32_t chunkid;
    uint32_t num_of_blocks_per_chunk;
    uint32_t block_size;
    uint32_t num_of_free_blocks;
    uint32_t num_of_used_blocks;
    uint32_t next_index;
    uint32_t next_chunkid;
    uint32_t prev_chunkid;
}ChunkHeader;

typedef struct _Chunk {
    ChunkHeader chunkHeader;
    void* mem_start;
    void* next;
    struct _Chunk* next_chunk;
    struct _Chunk* prev_chunk;
} Chunk;

typedef struct __attribute__((packed)) _PoolHeader {
    uint32_t num_of_blocks_per_chunk;
    uint32_t chunks_number;
    uint32_t block_size;
    uint32_t head_index;
    uint32_t current_chunk_index;
} PoolHeader;

typedef struct _Pool {
    PoolHeader poolHeader;
    Chunk* head;
    Chunk* current_chunk;
} Pool;

Pool* pool_create_by_chunk_size(size_t chunk_size, size_t block_size);
Pool* pool_create(size_t block_size, uint32_t num_of_blocks);
void chunk_free(Chunk* chunk);
void chunks_free(Chunk* chunk);
void pool_destroy(Pool* pool);
void* chunk_addr_from_index(Chunk* chunk, uint32_t index);
void * pool_addr_from_chblidx(Pool* pool, Chblidx* chblidx);
uint32_t chunk_index_from_addr(Chunk* chunk, const void* addr);
Chblidx* pool_chblidx_from_addr(Pool* pool, const void* addr);
Chunk* add_new_chunk(Pool* pool);
Chunk* get_chunk_from_addr(Pool* pool, void* addr);
uint32_t get_chunk_start_index(Pool* pool, Chunk* chunk);
void* pool_alloc(Pool* pool); 
void pool_dealloc(Pool* pool, void* p);
void* pool_serialization(Pool* pool);
Pool* pool_deserialization(void* data);



#endif