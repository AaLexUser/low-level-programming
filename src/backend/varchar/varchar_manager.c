#include "varchar_manager.h"

VarcharManager* varcharmanager_create(){
    VarcharManager* manager = malloc(sizeof(VarcharManager));
    manager->pool = pool_create(VARCHAR_BLOCK_SIZE, VARCHAR_NUM_OF_BLOCKS);
    return manager;
}

void varcharmanager_destroy(VarcharManager* manager){
    if(manager == NULL){
        return;
    }
    pool_destroy(manager->pool);
    free(manager);
}

void varcharmanager_clear(VarcharManager* manager){
    if(manager == NULL){
        return;
    }
    pool_clear(manager->pool);
}
//pfbi - pool first block index
uint32_t get_pfbi_from_str(VarcharManager* vm, const char* str){
    void* prev = NULL;
    uint32_t pfbi = -1;
    for(size_t i = 0; i < strlen(str); i+= DATA_PER_BLOCK_SIZE){
        void* block = pool_alloc(vm->pool);
        if(block != NULL){
           return pfbi;
        }
        memcpy(block, 0, sizeof(uint32_t));
        memcpy(block + sizeof(uint32_t), str + i, DATA_PER_BLOCK_SIZE);
        if(prev != NULL){
            uint32_t index = pool_index_from_addr(vm->pool, prev);
            memcpy(prev, &index, sizeof(uint32_t));
        }
        else {
            pfbi = pool_index_from_addr(vm->pool, block);
        }
        prev = block;
    }
    return pfbi;
}

char* get_str_from_pfbi(VarcharManager* vm, uint32_t pfbi, size_t str_len){
    char* str = malloc(str_len + 1);
    uint32_t next = 0;
    void* block = pool_addr_from_index(vm->pool, pfbi);
    for(size_t i = 0; i < str_len; i+= DATA_PER_BLOCK_SIZE){
        memcpy(&next, block, sizeof(uint32_t));
        memcpy(str + i, block + sizeof(uint32_t), DATA_PER_BLOCK_SIZE);
        if(next == 0){
            break;
        }
        block = pool_addr_from_index(vm->pool, next);
    }
    str[str_len + 1] = '\0';
    return str;
}