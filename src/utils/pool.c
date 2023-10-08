#include "pool.h"

Pool* pool_create(size_t block_size, uint32_t num_of_blocks){
    Pool* pool = malloc(sizeof(Pool));
    pool->global_num_of_blocks = num_of_blocks;
    pool->num_of_blocks_per_chunk = num_of_blocks;
    pool->block_size = block_size;
    Chunk *chunk = malloc(sizeof(Chunk));
    chunk->next_chunk = NULL;
    chunk->prev_chunk = NULL;
    chunk->chunkid = 0;
    chunk->num_of_free_blocks = num_of_blocks;
    chunk->num_of_used_blocks = 0;
    chunk->block_size = block_size;
    chunk->num_of_blocks_per_chunk = num_of_blocks;
    void* ptr = malloc(block_size * num_of_blocks);
    if (ptr == NULL){
        exit(EXIT_FAILURE);
    }
    chunk->mem_start = ptr;
    chunk->next = chunk->mem_start;
    pool->head = chunk;
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
    if(index >= chunk->num_of_blocks_per_chunk){
        return NULL;
    }
    return chunk->mem_start + (index * chunk->block_size);
}

void * pool_addr_from_chblidx(Pool* pool, Chblidx* chblidx){
    if(pool == NULL){
        return NULL;
    }
    if(chblidx->blockid >= pool->num_of_blocks_per_chunk){
        return NULL;
    }
    Chunk* chunk = pool->head;
    while(chunk != NULL && chunk->chunkid != chblidx->chunkid){
        chunk = chunk->next_chunk;
    }
    return chunk_addr_from_index(chunk, chblidx->blockid);
}

uint32_t chunk_index_from_addr(Chunk* chunk, const void* addr){
    if(chunk == NULL || addr == NULL){
        return 0;
    }
    if(addr < chunk->mem_start || addr > chunk->mem_start + chunk->num_of_blocks_per_chunk * chunk->block_size){
        return 0;
    }
    return (addr - chunk->mem_start) / chunk->block_size;
}

Chblidx* pool_chblidx_from_addr(Pool* pool, const void* addr){
    Chblidx *chblidx = malloc(sizeof(Chblidx));
    if(pool == NULL || addr == NULL){
        return NULL;
    }
    Chunk* chunk = pool->head;
    while(chunk != NULL){
        if(addr >= chunk->mem_start && addr <= chunk->mem_start + chunk->num_of_blocks_per_chunk * chunk->block_size){
            chblidx->blockid = chunk_index_from_addr(chunk, addr);
            chblidx->chunkid  = chunk->chunkid;
        }
        chunk = chunk->next_chunk;
    }
}

Chunk* add_new_chunk(Pool* pool){
    Chunk* chunk = malloc(sizeof(Chunk));
    chunk->next_chunk = NULL;
    chunk->num_of_free_blocks = pool->num_of_blocks_per_chunk;
    chunk->num_of_used_blocks = 0;
    chunk->block_size = pool->block_size;
    chunk->num_of_blocks_per_chunk = pool->num_of_blocks_per_chunk;
    void* ptr = malloc(pool->block_size * pool->num_of_blocks_per_chunk);
    if (ptr == NULL) {
        exit(EXIT_FAILURE);
    }
    chunk->mem_start = ptr;
    chunk->next = chunk->mem_start;
    chunk->chunkid = pool->current_chunk->chunkid + 1;
    chunk->prev_chunk = pool->current_chunk;
    pool->current_chunk->next_chunk = chunk;
    pool->current_chunk = chunk;
    return chunk;
}


void* pool_alloc(Pool* pool) {
    if (pool == NULL) {
        return NULL;
    }
    if (pool->current_chunk->num_of_used_blocks < pool->current_chunk->num_of_blocks_per_chunk) {
        uint32_t *p = (uint32_t *) chunk_addr_from_index(pool->current_chunk, pool->current_chunk->num_of_used_blocks);
        *p = pool->current_chunk->num_of_used_blocks + 1;
        pool->current_chunk->num_of_used_blocks++;
    }
    void* ret = NULL;
    if (pool->current_chunk->num_of_free_blocks > 0) {
        ret = pool->current_chunk->next;
        pool->current_chunk->num_of_free_blocks--;
        if (pool->current_chunk->num_of_free_blocks != 0) {
            pool->current_chunk->next = chunk_addr_from_index(pool->current_chunk, *(uint32_t *) pool->current_chunk->next);
        } else {
            pool->current_chunk->next = NULL;
        }
    }
    else {
        add_new_chunk(pool);
        ret = pool_alloc(pool);
    }
    return ret;
}

Chunk* get_chunk_from_addr(Pool* pool, void* addr){
    if(addr == NULL){
        return NULL;
    }
    Chunk* chunk = pool->head;
    while(chunk != NULL){
        if(addr >= chunk->mem_start && addr <= chunk->mem_start + chunk->num_of_blocks_per_chunk * chunk->block_size){
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
        index += current->num_of_blocks_per_chunk;
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
        *(uint32_t*)p = chunk->num_of_used_blocks;
        chunk->next = p;
    }
    ++chunk->num_of_free_blocks;
    if(chunk->num_of_free_blocks == chunk->num_of_blocks_per_chunk){
        if(pool->head == chunk){
            chunk->next_chunk->prev_chunk = NULL;
            pool->head = chunk->next_chunk;
        }
        else {
            chunk->prev_chunk->next_chunk = chunk->next_chunk;
            chunk->next_chunk->prev_chunk = chunk->prev_chunk;
        }
        chunk_free(chunk);
        return;
    }
    if(chunk->chunkid < pool->current_chunk->chunkid){
        pool->current_chunk = chunk;
    }
}