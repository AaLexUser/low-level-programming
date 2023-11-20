#include "../src/test.h"
#include "../../src/utils/chunky_pool.h"

DEFINE_TEST(dealloc){
    Pool *pool = NULL;
    pool_create(8, 3, pool);
    char str1[] = "12345678";
    void* block1 = pool_alloc(pool);
    strncpy(block1, str1, 8);

    char str2[] = "abcdefgh";
    void* block2 =  pool_alloc(pool);
    strncpy(block2, str2, 8);

    pool_dealloc(pool, block1);
    uint32_t res = 0;
    memcpy(&res, block1, sizeof(uint32_t));
    assert(res == 2);
    pool_dealloc(pool, block2);
    pool_destroy(pool);
}

DEFINE_TEST(serialization_deserialization){
    Pool *pool_before = NULL;
    pool_create(8, 3, pool_before);
    char str1_before[] = "12345678";
    void* block1_before = pool_alloc(pool_before);
    strncpy(block1_before, str1_before, 8);

    char str2_before[] = "abcdefgh";
    void* block2_before =  pool_alloc(pool_before);
    strncpy(block2_before, str2_before, 8);

    Chblidx* chblidx_block_1 = pool_chblidx_from_addr(pool_before,
                                                      block1_before);
    Chblidx* chblidx_block_2 = pool_chblidx_from_addr(pool_before,
                                                      block2_before);
    void* serialized_pool;
    pool_serialization(pool_before, serialized_pool);

    Pool* pool_after = pool_deserialization(serialized_pool);

    void* block1_after = pool_addr_from_chblidx(pool_after,
                                          chblidx_block_1);
    void* block2_after = pool_addr_from_chblidx(pool_after,
                                                chblidx_block_2);

    char* str1_after;
    strncpy(str1_after, block1_after, 8);
    assert(strcmp(str1_after, str1_before) == 0);

    char* str2_after;
    strncpy(str2_after, block2_after, 8);
    assert(strcmp(str2_after, str2_before) == 0);

    pool_destroy(pool_before);
    pool_destroy(pool_after);
}

int main(){
    RUN_SINGLE_TEST(dealloc);
    RUN_SINGLE_TEST(serialization_deserialization);
}