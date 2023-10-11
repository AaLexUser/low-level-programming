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

//pfchblidx - pool first chunk and block index
Chblidx* get_pfchblidx_from_str(VarcharManager* vm, const char* str){
    void* prev = NULL;
    Chblidx* pfchblidx = NULL;
    for(size_t i = 0; i < strlen(str); i+= DATA_PER_BLOCK_SIZE){
        void* block = pool_alloc(vm->pool);
        if(block == NULL){
           return pfchblidx;
        }
        memset(block, 0, sizeof(Chblidx));
        memcpy(block + sizeof(Chblidx), str + i, DATA_PER_BLOCK_SIZE);
        if(prev != NULL){
            Chblidx* chblix = pool_chblidx_from_addr(vm->pool, prev);
            memcpy(prev, chblix, sizeof(Chblidx));
        }
        else {
            pfchblidx = pool_chblidx_from_addr(vm->pool, block);
        }
        prev = block;
    }
    return pfchblidx;
}

char* get_str_from_pfbi(VarcharManager* vm, Chblidx* pfchblidx, size_t str_len){
    char* str = malloc(str_len + 1);
    Chblidx* next = 0;
    void* block = pool_addr_from_chblidx(vm->pool, pfchblidx);
    for(size_t i = 0; i < str_len; i+= DATA_PER_BLOCK_SIZE){
        memcpy(&next, block, sizeof(Chblidx));
        memcpy(str + i, block + sizeof(Chblidx), DATA_PER_BLOCK_SIZE);
        if(next == 0){
            break;
        }
        block = pool_addr_from_chblidx(vm->pool, next);
    }
    str[str_len + 1] = '\0';
    return str;
}