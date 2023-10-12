#include "varchar_manager.h"

typedef struct Block{
    Chblidx next;
    uint8_t data[];
} Block;

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
    Chblidx* pfchblidx = NULL;
    Block* prev = NULL;

    size_t block_size = DATA_PER_BLOCK_SIZE - sizeof(Chblidx);

    // Calculate number of blocks needed
    size_t num_blocks = (strlen(str) + block_size - 1) / block_size;

    for (int i = 0; i < num_blocks; i++) {

        // Allocate block
        Block* block = pool_alloc(vm->pool);

        // Copy string data into block
        size_t len = strlen(str + i*block_size);
        if (len > block_size) len = block_size;
        memcpy(block->data, str + i*block_size, len);

        // Link blocks
        if (prev)
            memcpy(&prev->next, pool_chblidx_from_addr(vm->pool,block), sizeof(Chblidx));
        else pfchblidx = pool_chblidx_from_addr(vm->pool, block);

        prev = block;

    }
    return pfchblidx;
}

char* get_str_from_pfbi(VarcharManager* vm, Chblidx* pfchblidx, size_t str_len){
    char* str = malloc(str_len + 1);
    size_t block_size = DATA_PER_BLOCK_SIZE - sizeof(Chblidx);
    size_t offset = 0;
    Block* block = pool_addr_from_chblidx(vm->pool, pfchblidx);
    while (block != NULL) {
        size_t copy_len = block_size;
        if (offset + copy_len > str_len) {
            copy_len = str_len - offset;
        }

        memcpy(str + offset, block->data, copy_len);

        offset += copy_len;
        Block* prev = block;
        block = pool_addr_from_chblidx(vm->pool,&block->next);
        pool_dealloc(vm->pool, prev);
    }
    str[str_len] = '\0';
    return str;
}