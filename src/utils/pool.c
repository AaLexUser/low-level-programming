#include "pool.h"

Pool* pool_create(size_t block_size, uint32_t num_of_blocks){
    Pool* pool = malloc(sizeof(Pool));
    pool->num_of_blocks = num_of_blocks;
    pool->block_size = block_size;
    pool->num_of_free_blocks = num_of_blocks;
    pool->num_of_used_blocks = 0;
    void* ptr = malloc(block_size * num_of_blocks);
    if (ptr == NULL){
        exit(EXIT_FAILURE);
    }
    pool->mem_start = ptr;
    pool->next = pool->mem_start;
    return pool;
}

void pool_destroy(Pool* pool){
    if(pool == NULL){
        return;
    }
    free(pool->mem_start);
    free(pool);
}

void pool_clear(Pool* pool){
    if(pool == NULL){
        return;
    }
    pool->num_of_free_blocks = pool->num_of_blocks;
    pool->num_of_used_blocks = 0;
    pool->next = pool->mem_start;
}

unsigned char * pool_addr_from_index(Pool* pool, uint32_t index){
    if(pool == NULL){
        return NULL;
    }
    if(index >= pool->num_of_blocks){
        return NULL;
    }
    return pool->mem_start + (index * pool->block_size);
}

uint32_t pool_index_from_addr(Pool* pool, const void* addr){
    if(pool == NULL){
        return 0;
    }
    if(addr < pool->mem_start){
        return 0;
    }
    if(addr >= pool->mem_start + (pool->num_of_blocks * pool->block_size)){
        return 0;
    }
    return (addr - pool->mem_start) / pool->block_size;
}

bool pool_expand_capacity(Pool* pool, uint32_t num_of_blocks){
    if(pool == NULL){
        false;
    }
    if(num_of_blocks > pool->num_of_blocks){
        false;
    }
    pool->num_of_free_blocks += num_of_blocks;
    void* ptr = realloc(pool->mem_start, pool->block_size * num_of_blocks);
    if(ptr == NULL){
        false;
    }
    return true;
}

void* pool_alloc(Pool* pool) {
    if (pool == NULL) {
        return NULL;
    }
    if (pool->num_of_used_blocks < pool->num_of_blocks) {
        uint32_t *p = (uint32_t *) pool_addr_from_index(pool, pool->num_of_used_blocks);
        *p = pool->num_of_used_blocks + 1;
        pool->num_of_used_blocks++;
    }
    void* ret = NULL;
    if (pool->num_of_free_blocks > 0) {
        ret = pool->next;
        pool->num_of_free_blocks--;
        if (pool->num_of_free_blocks != 0) {
            pool->next = pool_addr_from_index(pool, *(uint32_t *) pool->next);
        } else {
            pool->next = NULL;
        }
    }
    else {
        if(pool_expand_capacity(pool, pool->num_of_blocks * 2)){
            ret = pool->next;
            pool->num_of_free_blocks--;
            if (pool->num_of_free_blocks != 0) {
                pool->next = pool_addr_from_index(pool, *(uint32_t *) pool->next);
            } else {
                pool->next = NULL;
            }
        };
    }
    return ret;
}

void pool_dealloc(Pool* pool, void* p){
    if (pool == NULL) {
        return;
    }
    if (pool->next != NULL){
        *(uint32_t*)p = pool_index_from_addr(pool, pool->next);
        pool->next = p;
    }
    else {
        *(uint32_t*)p = pool->num_of_used_blocks;
        pool->next = p;
    }
    ++pool->num_of_free_blocks;
}