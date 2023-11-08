#include "../src/test.h"
#include "../src/backend/io/file_manager.h"
#include "../src/backend/io/caching.h"
#include "backend/page_pool/page_pool.h"
#include "backend/io/pager.h"
#include "utils/logger.h"
#include <stdio.h>

DEFINE_TEST(write_and_read){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    char str[] = "12345678";
    int64_t start_idx = ppl_init(9);
    page_pool_t* ppl = ppl_load(start_idx);
    chblix_t block1 = ppl_alloc(ppl);
    ppl_write_block(ppl, &block1, str, sizeof(str), 0);
    char* read_str;
    read_str = ppl_read_block(ppl, &block1, 0);
    assert(strcmp(str, read_str) == 0);
    ppl_destroy(ppl);
    pg_destroy();
    ch_destroy();
    delete_file();
}

DEFINE_TEST(several_write){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    char str[] = "12345678";
    int64_t block_size = 9;
    int64_t start_idx = ppl_init(block_size);
    page_pool_t* ppl = ppl_load(start_idx);
    int64_t count = (int64_t)(PAGE_SIZE - sizeof_Page_Header) / block_size;
    chblix_t blocks[count];
    for(int64_t i = 0; i < count; i++){
        chblix_t block = ppl_alloc(ppl);
        blocks[i] = block;
        ppl_write_block(ppl, &block, str, sizeof(str), 0);
    }
    for(int64_t i = 0; i < count; i++){
        chblix_t block = blocks[i];
        char* read_str = ppl_read_block(ppl, &block, 0);
        assert(strcmp(str, read_str) == 0);
    }
    ppl_destroy(ppl);
    pg_destroy();
    ch_destroy();
    delete_file();
}

DEFINE_TEST(close_and_open){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    char str[] = "12345678";
    int64_t block_size = 9;
    int64_t start_idx = ppl_init(block_size);
    page_pool_t* ppl = ppl_load(start_idx);
    int64_t count = (int64_t)(PAGE_SIZE - sizeof_Page_Header) / block_size;
    count--; // to prevent page_pool expand
    chblix_t blocks[count];
    for(int64_t i = 0; i < count ; i++){
        chblix_t block = ppl_alloc(ppl);
        blocks[i] = block;
        ppl_write_block(ppl, &block, str, sizeof(str), 0);
    }
    ppl_destroy(ppl);
    pg_destroy();
    ch_destroy();
    close_file();
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    ppl = ppl_load(start_idx);
    for(int64_t i = 0; i < count; i++){
        chblix_t block = blocks[i];
        char* read_str = ppl_read_block(ppl, &block, 0);
        assert(strcmp(str, read_str) == 0);
    }
    ppl_destroy(ppl);
    pg_destroy();
    ch_destroy();
    delete_file();
}

DEFINE_TEST(dealloc){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    char str[] = "12345678";
    int64_t block_size = 9;
    int64_t start_idx = ppl_init(block_size);
    page_pool_t* ppl = ppl_load(start_idx);
    linked_page_t* linkedPage = lp_load(ppl->current_idx);
    int64_t count = lp_useful_space_size(linkedPage) / block_size;
    chblix_t blocks[3 * count];
    for(int64_t i = 0; i < 3 * count ; i++){
        chblix_t block = ppl_alloc(ppl);
        blocks[i] = block;
        ppl_write_block(ppl, &block, str, sizeof(str), 0);
    }
    for(int64_t i = count; i < 2 * count ; i++){
        chblix_t block = blocks[i];
        ppl_dealloc(ppl, &block);
    }
    int64_t page = pg_alloc();
    assert(page == blocks[count].chunk_idx);

    ppl_destroy(ppl);
    pg_destroy();
    ch_destroy();
    delete_file();

}

DEFINE_TEST(ultra_wide_page){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    char str[] = "12345678";
    int64_t block_size = 2 * PAGE_SIZE;
    int64_t start_idx = ppl_init(block_size);
    page_pool_t* ppl = ppl_load(start_idx);
    int64_t count = 2;
    chblix_t blocks[count];
    for(int64_t i = 0; i < count; i++){
        chblix_t block = ppl_alloc(ppl);
        blocks[i] = block;
        ppl_write_block(ppl, &block, str, sizeof(str), PAGE_SIZE+1000);
    }
    for(int64_t i = 0; i < count; i++){
        chblix_t block = blocks[i];
        char* read_str = ppl_read_block(ppl, &block, PAGE_SIZE+1000);
        assert(strcmp(str, read_str) == 0);
    }
    ppl_destroy(ppl);
    pg_destroy();
    ch_destroy();
    delete_file();
}

int main(){
    RUN_SINGLE_TEST(write_and_read);
    RUN_SINGLE_TEST(several_write);
    RUN_SINGLE_TEST(close_and_open);
    RUN_SINGLE_TEST(dealloc);
    RUN_SINGLE_TEST(ultra_wide_page);
}