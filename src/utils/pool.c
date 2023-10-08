#include "pool.h"

Pool* pool_create_by_chunk_size(size_t chunk_size, size_t block_size){
    return pool_create(block_size, (chunk_size - sizeof(ChunkHeader))/ block_size);
}

Pool* pool_create(size_t block_size, uint32_t num_of_blocks){
    Pool* pool = malloc(sizeof(Pool));
    pool->poolHeader.num_of_blocks_per_chunk = num_of_blocks;
    pool->poolHeader.block_size = block_size;
    Chunk *chunk = malloc(sizeof(Chunk));
    chunk->next_chunk = NULL;
    chunk->prev_chunk = NULL;
    chunk->chunkHeader.chunkid = 0;
    chunk->chunkHeader.num_of_free_blocks = num_of_blocks;
    chunk->chunkHeader.num_of_used_blocks = 0;
    chunk->chunkHeader.block_size = block_size;
    chunk->chunkHeader.num_of_blocks_per_chunk = num_of_blocks;
    void* ptr = malloc(block_size * num_of_blocks);
    if (ptr == NULL){
        exit(EXIT_FAILURE);
    }
    memset(ptr, 0, block_size * num_of_blocks);
    chunk->mem_start = ptr;
    chunk->next = chunk->mem_start;
    pool->head = chunk;
    pool->poolHeader.chunks_number++;
    pool->current_chunk = chunk;
    return pool;
}

void chunk_free(Chunk* chunk){
    if(chunk == NULL){
        return;
    }
    free(chunk->mem_start);
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
    if(chblidx->blockid >= pool->poolHeader.num_of_blocks_per_chunk){
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
        return NULL;
    }
    chblidx->chunkid = chunk->chunkHeader.chunkid;
    chblidx->blockid = chunk_index_from_addr(chunk, addr);
    return chblidx;
}

Chblidx* pool_chblidx_from_addr(Pool* pool, const void* addr){
    Chblidx *chblidx = malloc(sizeof(Chblidx));
    if(pool == NULL || addr == NULL){
        return NULL;
    }
    Chunk* chunk = pool->head;
    while(chunk != NULL){
        if(addr >= chunk->mem_start && addr <= chunk->mem_start + chunk->chunkHeader.num_of_blocks_per_chunk * chunk->chunkHeader.block_size){
            chblidx->blockid = chunk_index_from_addr(chunk, addr);
            chblidx->chunkid  = chunk->chunkHeader.chunkid;
        }
        chunk = chunk->next_chunk;
    }
}

uint32_t chunk_turn_from_addr_to_index(Chunk* chunk, void* next){
    if(chunk == NULL){
        return 0;
    }
    if(next == NULL){
        return chunk->chunkHeader.num_of_blocks_per_chunk;
    }
    return (next - chunk->mem_start) / chunk->chunkHeader.block_size;
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



Chunk* add_new_chunk(Pool* pool){
    Chunk* chunk = malloc(sizeof(Chunk));
    chunk->next_chunk = NULL;
    chunk->chunkHeader.num_of_free_blocks = pool->poolHeader.num_of_blocks_per_chunk;
    chunk->chunkHeader.num_of_used_blocks = 0;
    chunk->chunkHeader.block_size = pool->poolHeader.block_size;
    chunk->chunkHeader.num_of_blocks_per_chunk = pool->poolHeader.num_of_blocks_per_chunk;
    void* ptr = malloc(pool->poolHeader.block_size * pool->poolHeader.num_of_blocks_per_chunk);
    if (ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    chunk->mem_start = ptr;
    chunk->next = chunk->mem_start;
    chunk->chunkHeader.chunkid = pool->current_chunk->chunkHeader.chunkid + 1;
    chunk->prev_chunk = pool->current_chunk;
    pool->current_chunk->next_chunk = chunk;
    pool->current_chunk = chunk;
    pool->poolHeader.chunks_number++;
    return chunk;
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
    if (chunk->next != NULL){
        *(uint32_t*)p = chunk_index_from_addr(chunk, chunk->next);
        chunk->next = p;
    }
    else {
        *(uint32_t*)p = chunk->chunkHeader.num_of_used_blocks;
        chunk->next = p;
    }
    ++chunk->chunkHeader.num_of_free_blocks;
    if(chunk->chunkHeader.num_of_free_blocks == chunk->chunkHeader.num_of_blocks_per_chunk){
        if(pool->head == chunk){
            chunk->next_chunk->prev_chunk = NULL;
            pool->head = chunk->next_chunk;
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


void* pool_serialization(Pool* pool){
    uint64_t total_size = sizeof(uint64_t)
                    + sizeof(PoolHeader)
                    + pool->poolHeader.chunks_number
                        * sizeof(ChunkHeader)
                    + pool->poolHeader.num_of_blocks_per_chunk
                        * pool->poolHeader.block_size
                        * pool->poolHeader.chunks_number;
    void* res = malloc(total_size);
    memset(res, 0, total_size);
    memcpy(res, &total_size, sizeof(uint64_t));
    pool->poolHeader.head_index = pool->head->chunkHeader.chunkid;
    pool->poolHeader.current_chunk_index = pool->current_chunk->chunkHeader.chunkid;
    memcpy(res, &pool->poolHeader, sizeof(PoolHeader));
    Chunk* chunk = pool->head;
    uint32_t index = 0;
    while(chunk != NULL){
        chunk->chunkHeader.next_index = chunk_turn_from_addr_to_index(chunk, chunk->next);
        chunk->chunkHeader.next_chunkid = chunk->next_chunk == NULL ? 0 : chunk->next_chunk->chunkHeader.chunkid;
        chunk->chunkHeader.prev_chunkid = chunk->prev_chunk == NULL ? 0 : chunk->prev_chunk->chunkHeader.chunkid;
        memcpy(res + sizeof(PoolHeader)
            + index * sizeof(ChunkHeader)
            + index
                * pool->poolHeader.num_of_blocks_per_chunk
                * pool->poolHeader.block_size
                , &chunk->chunkHeader, sizeof(ChunkHeader));
        memcpy(res + sizeof(PoolHeader)
            + (index+1) * sizeof(ChunkHeader)
            + index
                * pool->poolHeader.num_of_blocks_per_chunk
                * pool->poolHeader.block_size,
                chunk->mem_start,
                pool->poolHeader.num_of_blocks_per_chunk * pool->poolHeader.block_size);
        chunk = chunk->next_chunk;
        index++;
    }
    return res;
}

Pool* pool_deserialization(void* data){
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