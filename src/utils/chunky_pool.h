#ifndef _POOL_H_
#define _POOL_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


typedef struct __attribute__((packed)) _Chblidx {
    int32_t chunkid;
    uint32_t blockid;
} Chblidx;

typedef struct __attribute__((packed)) _ChunkHeader{
    int32_t chunkid;
    uint32_t num_of_blocks_per_chunk;
    uint32_t block_size;
    uint32_t num_of_free_blocks;
    uint32_t num_of_used_blocks;
} ChunkHeader;

typedef struct _Chunk {
    ChunkHeader chunkHeader;
    void* mem_start;
    void* next;
    int32_t next_chunkid;
    int32_t prev_chunkid;
} chunk_t;

typedef struct __attribute__((packed)) {
    uint32_t package_size;
    ChunkHeader chunkHeader;
    uint32_t next_index;
    int32_t next_chunkid;
    int32_t prev_chunkid;
    uint8_t data[];
} ChunkPackage;

typedef struct __attribute__((packed)) _PoolHeader {
    uint32_t chunks_number;
    uint32_t block_size;
} PoolHeader;

typedef struct _Pool {
    PoolHeader poolHeader;
    HT(int32_t, size_t) chunkid_to_index;
    chunk_t* head;
    chunk_t* current_chunk;
} Pool;

typedef struct __attribute__((packed)) {
    uint32_t package_size;
    PoolHeader poolHeader;
    int32_t head_index;
    int32_t current_chunk_index;
    uint8_t chunks[];
} PoolPackage;



bool pool_create_by_chunk_size(size_t block_size, Pool* pool, size_t chunk_memory_size);
bool pool_create(size_t block_size, uint32_t num_of_blocks, Pool* pool);
void chunk_free(chunk_t* chunk);
void chunks_free(chunk_t* chunk);
void pool_destroy(Pool* pool);
void* chunk_addr_from_index(chunk_t* chunk, uint32_t index);
void * pool_addr_from_chblidx(Pool* pool, Chblidx* chblidx);
uint32_t chunk_index_from_addr(chunk_t* chunk, const void* addr);
Chblidx* chunk_chblidx_from_addr(chunk_t* chunk, const void* addr);
Chblidx* pool_chblidx_from_addr(Pool* pool, const void* addr);
chunk_t* add_new_chunk(Pool* pool);
chunk_t* get_chunk_from_addr(Pool* pool, void* addr);
uint32_t get_chunk_start_index(Pool* pool, chunk_t* chunk);
void* pool_alloc(Pool* pool); 
void pool_dealloc(Pool* pool, void* p);
bool pool_serialization(Pool* pool, uint8_t* buf);
Pool* pool_deserialization(uint8_t* buf);



#endif