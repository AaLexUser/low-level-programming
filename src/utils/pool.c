#include "pool.h"
#include <stdio.h>

/**
 * Creates chunk
 *
 * @param: block_size - size of block in bytes
 * @param: num_of_blocks - number of blocks in chunk
 * @param: chunk_memory_size - size of chunk memory in bytes
 * @param: chunk - pointer to resulting chunk
 * @return: 0 - success
 *         1 - failed
 */

bool __chunk_create(size_t block_size, uint32_t num_of_blocks, size_t chunk_memory_size, Chunk* chunk){
    chunk = malloc(sizeof(Chunk));
    chunk->chunkHeader.chunkid = 0;
    chunk->chunkHeader.num_of_free_blocks = num_of_blocks;
    chunk->chunkHeader.num_of_used_blocks = 0;
    chunk->chunkHeader.block_size = block_size;
    chunk->chunkHeader.num_of_blocks_per_chunk = num_of_blocks;
    void* ptr = malloc(chunk_memory_size);
    if (ptr == NULL){
        return 1;
    }
    memset(ptr, 0, chunk_memory_size);
    chunk->mem_start = ptr;
    chunk->next = chunk->mem_start;
    return 0;
}
/**
 * Creates pool with one chunk
 *
 * @param block_size - size of block in bytes
 * @param num_of_blocks - number of blocks in chunk
 * @param chunk_memory_size - size of chunk memory in bytes
 * @param pool - pointer to resulting pool
 * @return 0 - success
 *          1 - failed
 */

bool __pool_create(size_t block_size, uint32_t num_of_blocks, size_t chunk_memory_size, Pool* pool){
    pool = malloc(sizeof(Pool));
    pool->poolHeader.block_size = block_size;
    Chunk *chunk;
    if(__chunk_create(block_size, num_of_blocks, chunk_memory_size, chunk)){
        free(pool);
        return 1;
    };
    chunk->next_chunkid = -1;
    chunk->prev_chunkid = -1;
    pool->head = chunk;
    pool->poolHeader.chunks_number = 1;
    pool->current_chunk = chunk;
    return 0;
}

/**
 * Creates pool with one chunk
 *
 * @param block_size - size of block in bytes
 * @param num_of_blocks - number of blocks in chunk
 * @param pool - pointer to resulting pool
 * @return 0 - success
 *         1 - failed
 */

bool pool_create(size_t block_size, uint32_t num_of_blocks, Pool* pool){
    return __pool_create(block_size, num_of_blocks, block_size * num_of_blocks, pool);
}

/**
 * Creates pool with one chunk by chunk memory size
 *
 * @param block_size - size of block in bytes
 * @param pool - pointer to resulting pool
 * @param chunk_memory_size - size of chunk memory in bytes
 * @return 0 - success
 *         1 - failed
 */

bool pool_create_by_chunk_size(size_t block_size, Pool* pool, size_t chunk_memory_size){
    uint32_t num_of_blocks = chunk_memory_size / block_size;
    if(num_of_blocks == 0){
        num_of_blocks = 1;
    }
    return __pool_create(block_size, num_of_blocks, chunk_memory_size, pool);
}

/**
 * Frees chunk
 *
 * @param chunk - pointer to chunk
 */

void chunk_free(Chunk* chunk){
    if(chunk == NULL){
        return;
    }
    free(chunk->mem_start);
    free(chunk->next);
    free(chunk);
}

void chunks_free(Chunk* chunk){
    if(chunk == NULL){
        return;
    }
    while(chunk->next_chunk != NULL){
        Chunk* next = chunk->next_chunk;
        free(chunk->mem_start);
        free(chunk);
        chunk = next;
    }
}

void pool_destroy(Pool* pool){
    if(pool == NULL){
        return;
    }
    chunks_free(pool->current_chunk);
    free(pool);
}

void* chunk_addr_from_index(Chunk* chunk, uint32_t index){
    if(chunk == NULL){
        return NULL;
    }
    if(index >= chunk->chunkHeader.num_of_blocks_per_chunk){
        return NULL;
    }
    return chunk->mem_start + (index * chunk->chunkHeader.block_size);
}


void * pool_addr_from_chblidx(Pool* pool, Chblidx* chblidx){
    if(pool == NULL){
        return NULL;
    }
    if(chblidx->blockid >= pool->head->chunkHeader.num_of_blocks_per_chunk){
        return NULL;
    }
    Chunk* chunk = pool->head;
    while(chunk != NULL && chunk->chunkHeader.chunkid != chblidx->chunkid){
        chunk = chunk->next_chunk;
    }
    return chunk_addr_from_index(chunk, chblidx->blockid);
}

uint32_t chunk_index_from_addr(Chunk* chunk, const void* addr){
    if(chunk == NULL || addr == NULL){
        return 0;
    }
    if(addr < chunk->mem_start || addr > chunk->mem_start + chunk->chunkHeader.num_of_blocks_per_chunk * chunk->chunkHeader.block_size){
        return 0;
    }
    return (addr - chunk->mem_start) / chunk->chunkHeader.block_size;
}

Chblidx* chunk_chblidx_from_addr(Chunk* chunk, const void* addr){
    Chblidx *chblidx = malloc(sizeof(Chblidx));
    if(chunk == NULL || addr == NULL){
        free(chblidx);
        return NULL;
    }
    chblidx->chunkid = chunk->chunkHeader.chunkid;
    chblidx->blockid = chunk_index_from_addr(chunk, addr);
    return chblidx;
}

Chblidx* pool_chblidx_from_addr(Pool* pool, const void* addr){
    if(pool == NULL || addr == NULL){
        return NULL;
    }
    Chunk* chunk = pool->head;
    while(chunk != NULL){
        if(addr >= chunk->mem_start && addr <= chunk->mem_start + chunk->chunkHeader.num_of_blocks_per_chunk * chunk->chunkHeader.block_size){
            Chblidx *chblidx = malloc(sizeof(Chblidx));
            chblidx->blockid = chunk_index_from_addr(chunk, addr);
            chblidx->chunkid  = chunk->chunkHeader.chunkid;
            return chblidx;
        }
        chunk = chunk->next_chunk;
    }
    return NULL;
}

bool chunk_turn_from_addr_to_index(Chunk* chunk, uint32_t* index){
    if(chunk == NULL){
        return false;
    }
    if(chunk->next == NULL){
        return chunk->chunkHeader.num_of_blocks_per_chunk;
    }
    uint32_t bytes_to_next = chunk->next - chunk->mem_start;
    return bytes_to_next / chunk->chunkHeader.block_size;;
}

void* chunk_turn_from_index_to_addr(Chunk* chunk, uint32_t next){
    if(chunk == NULL){
        return NULL;
    }
    if(next == chunk->chunkHeader.num_of_blocks_per_chunk){
        return NULL;
    }
    return chunk->mem_start + (next * chunk->chunkHeader.block_size);
}


Chunk* add_chunk_custom(Pool* pool,
                    int32_t chunkid,
                  uint32_t num_of_blocks_per_chunk,
                  uint32_t block_size,
                  Chunk* prev_chunk){
    Chunk* chunk = malloc(sizeof(Chunk));

    chunk->chunkHeader.chunkid = chunkid;
    chunk->chunkHeader.num_of_free_blocks = num_of_blocks_per_chunk;
    chunk->chunkHeader.num_of_used_blocks = 0;
    chunk->chunkHeader.block_size = block_size;
    chunk->chunkHeader.num_of_blocks_per_chunk = num_of_blocks_per_chunk;
    void* ptr = malloc(block_size * num_of_blocks_per_chunk);
    if (ptr == NULL) {
        printf("Failed malloc chunk");
        exit(EXIT_FAILURE);
    }
    chunk->mem_start = ptr;
    chunk->next = chunk->mem_start;
    chunk->prev_chunk = prev_chunk;
    chunk->next_chunk = NULL;

    if(pool->current_chunk)
        pool->current_chunk->next_chunk = chunk;
    pool->current_chunk = chunk;
    pool->poolHeader.chunks_number++;
    return chunk;

}

Chunk* add_new_chunk(Pool* pool){
    int32_t chunkid = pool->current_chunk->chunkHeader.chunkid + 1;
    uint32_t num_of_blocks_per_chunk = pool->head->chunkHeader.num_of_blocks_per_chunk;
    uint32_t block_size = pool->poolHeader.block_size;
    Chunk* prev_chunk = pool->current_chunk;
    return add_chunk_custom(pool,
                        chunkid,
                        num_of_blocks_per_chunk,
                        block_size,
                        prev_chunk);
}




void* pool_alloc(Pool* pool) {
    if (pool == NULL) {
        return NULL;
    }
    if (pool->current_chunk->chunkHeader.num_of_used_blocks < pool->current_chunk->chunkHeader.num_of_blocks_per_chunk) {
        uint32_t *p = (uint32_t *) chunk_addr_from_index(pool->current_chunk, pool->current_chunk->chunkHeader.num_of_used_blocks);
        *p = pool->current_chunk->chunkHeader.num_of_used_blocks + 1;
        pool->current_chunk->chunkHeader.num_of_used_blocks++;
    }
    void* ret = NULL;
    if (pool->current_chunk->chunkHeader.num_of_free_blocks > 0) {
        ret = pool->current_chunk->next;
        pool->current_chunk->chunkHeader.num_of_free_blocks--;
        if (pool->current_chunk->chunkHeader.num_of_free_blocks != 0) {
            pool->current_chunk->next = chunk_addr_from_index(pool->current_chunk, *(uint32_t *) pool->current_chunk->next);
        } else {
            pool->current_chunk->next = NULL;
        }
    }
    else {
        if(pool->current_chunk->next_chunk != NULL){
            pool->current_chunk = pool->current_chunk->next_chunk;
            ret = pool_alloc(pool);
        }
        else {
            add_new_chunk(pool);
            ret = pool_alloc(pool);
        }
    }
    return ret;
}

Chunk* get_chunk_from_addr(Pool* pool, void* addr){
    if(addr == NULL){
        return NULL;
    }
    Chunk* chunk = pool->head;
    while(chunk != NULL){
        if(addr >= chunk->mem_start && addr <= chunk->mem_start + chunk->chunkHeader.num_of_blocks_per_chunk * chunk->chunkHeader.block_size){
            return chunk;
        }
        chunk = chunk->next_chunk;
    }
    return NULL;
}

uint32_t get_chunk_start_index(Pool* pool, Chunk* chunk){
    if(chunk == NULL){
        return 0;
    }
    uint32_t index = 0;
    Chunk* current = pool->head;
    while(current != chunk){
        index += current->chunkHeader.num_of_blocks_per_chunk;
        current = current->next_chunk;
    }
    return index;
}

void pool_dealloc(Pool* pool, void* p){
    if (pool == NULL) {
        return;
    }
    Chunk* chunk = get_chunk_from_addr(pool, p);
    uint32_t index;
    if (chunk->next != NULL){
        index = chunk_index_from_addr(chunk, chunk->next);
    }
    else {
        index = chunk->chunkHeader.num_of_used_blocks;
    }
    memcpy(p, &index, sizeof(index));
    chunk->next = p;

    ++chunk->chunkHeader.num_of_free_blocks;
    if(chunk->chunkHeader.num_of_free_blocks == chunk->chunkHeader.num_of_blocks_per_chunk){
        if(pool->head == chunk){
            if(chunk->next_chunk) {
                chunk->next_chunk->prev_chunk = NULL;
                pool->head = chunk->next_chunk;
            }
        }
        else {
            chunk->prev_chunk->next_chunk = chunk->next_chunk;
            chunk->next_chunk->prev_chunk = chunk->prev_chunk;
        }
        chunk_free(chunk);
        pool->poolHeader.chunks_number--;
        return;
    }
    if(chunk->chunkHeader.chunkid < pool->current_chunk->chunkHeader.chunkid){
        pool->current_chunk = chunk;
    }
}

bool serialize_chunk(Chunk* chunk, uint8_t* buf){
    ChunkPackage* chunkPackage = (ChunkPackage *) buf;

    size_t header_size = sizeof(ChunkPackage);
    size_t data_size = chunk->chunkHeader.num_of_blocks_per_chunk
                       * chunk->chunkHeader.block_size;
    chunkPackage->package_size = header_size + data_size;

    memcpy(&chunkPackage->chunkHeader, &chunk->chunkHeader, sizeof(ChunkHeader));
    if(chunk_turn_from_addr_to_index(chunk, &chunkPackage->next_index)){
        return false;
    }
    chunkPackage->next_chunkid = chunk->next_chunk == NULL ? -1 : chunk->next_chunk->chunkHeader.chunkid;
    chunkPackage->prev_chunkid = chunk->prev_chunk == NULL ? -1 : chunk->prev_chunk->chunkHeader.chunkid;
    memcpy(chunkPackage->data, chunk->mem_start, data_size);
    return true;
}


bool pool_serialization(Pool* pool, uint8_t* buf){
    PoolPackage* poolPackage = (PoolPackage*)buf;
    size_t pool_header_size = sizeof(PoolPackage);

    //serialization of pool package header
    memcpy(&poolPackage->poolHeader, &pool->poolHeader, sizeof(PoolHeader));
    poolPackage->head_index = pool->head == NULL ? -1 : pool->head->chunkHeader.chunkid;
    poolPackage->current_chunk_index = pool->current_chunk == NULL ? -1: pool->current_chunk->chunkHeader.chunkid;

    // serialization of chunks
    Chunk* chunk = pool->head;
    off_t offset = 0;
    while(chunk != NULL){
        if(!serialize_chunk(chunk, poolPackage->chunks + offset)) return false;
        ChunkPackage* chunkPackage = (ChunkPackage *) (poolPackage->chunks + offset);
        offset += chunkPackage->package_size;
        chunk = chunk->next_chunk;
    }

    poolPackage->package_size = pool_header_size + offset;
    return true;
}

bool chunk_deserialization(uint8_t* buf, Pool* pool, Chunk* chunk){
    ChunkPackage* chunkPackage = (ChunkPackage*)buf;

    size_t header_size = sizeof(ChunkPackage);
    size_t data_size = chunk->chunkHeader.num_of_blocks_per_chunk
                       * chunk->chunkHeader.block_size;

    chunk = add_chunk_custom(pool,
                             chunkPackage->chunkHeader.chunkid,
                             chunkPackage->chunkHeader.num_of_blocks_per_chunk,
                             chunkPackage->chunkHeader.block_size,
                             pool->current_chunk);
    memcpy(&chunk->chunkHeader, &chunkPackage->chunkHeader, sizeof(ChunkHeader));
    chunk->next = chunk_turn_from_index_to_addr(chunk, chunkPackage->next_index);

    memcpy(chunk->mem_start, chunkPackage->data, data_size);
    return true;
}

Pool* pool_deserialization(uint8_t* buf){
    PoolPackage* poolPackage = (PoolPackage*)buf;
    Pool* pool = malloc(sizeof(Pool));
    size_t data_size = poolPackage->package_size - sizeof (PoolPackage);


    //deserialize pool header
    memcpy(&pool->poolHeader, &poolPackage->poolHeader, sizeof(PoolHeader));

    //deserialize chunks
    off_t offset = 0;
    Chunk* prev = NULL;
    Chunk* next = NULL;
    while(offset >= data_size) {
        Chunk* chunk = NULL;
        chunk_deserialization(poolPackage->chunks + offset, chunk);
    }





    uint64_t total_size = 0;
    memcpy(&total_size, data, sizeof(uint64_t));
    Pool* pool = malloc(sizeof(Pool));
    memcpy(&pool->poolHeader, data + sizeof(uint64_t), sizeof(PoolHeader));
    size_t offset = sizeof(uint64_t) + sizeof(PoolHeader);
    pool->head = malloc(sizeof(Chunk));
    memcpy(&pool->head->chunkHeader, data + offset, sizeof(ChunkHeader));
    offset += sizeof(ChunkHeader);
    pool->head->mem_start = data + offset;
    pool->head->next = chunk_turn_from_index_to_addr(pool->head, pool->head->chunkHeader.next_index);
    pool->head->next_chunk = NULL;
    pool->head->prev_chunk = NULL;
    pool->current_chunk = pool->head;
    Chunk* chunk = pool->head;
    for(uint32_t i = 1; i < pool->poolHeader.chunks_number; i++) {
        chunk->next_chunk = malloc(sizeof(Chunk));
        chunk->next_chunk->prev_chunk = chunk;
        chunk = chunk->next_chunk;
        memcpy(&chunk->chunkHeader, data + offset, sizeof(ChunkHeader));
        offset += sizeof(ChunkHeader);
        chunk->mem_start = data + offset;
        offset += pool->poolHeader.num_of_blocks_per_chunk * pool->poolHeader.block_size;
        chunk->next = chunk_turn_from_index_to_addr(chunk, chunk->chunkHeader.next_index);
        chunk->next_chunk = NULL;
    }
    return pool;
}