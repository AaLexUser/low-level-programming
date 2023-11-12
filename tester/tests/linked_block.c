#include "../src/test.h"
#include "../src/backend/io/caching.h"
#include "backend/io/pager.h"
#include "backend/page_pool/page_pool.h"
#include "backend/io/linked_blocks.h"

DEFINE_TEST(write_read){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    char str[] = "12345678";
    int64_t poop_idx = lb_ppl_init(9);
    chblix_t block1 = lb_alloc(poop_idx);
    assert(lb_write(poop_idx, &block1, str, sizeof(str), 0) == LB_SUCCESS);
    char* read_str = malloc(sizeof(str));
    lb_read(poop_idx, &block1, read_str, sizeof(str), 0);
    assert(strcmp(str, read_str) == 0);
    free(read_str);
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
    int64_t poop_idx = lb_ppl_init(block_size);
    int64_t count = 466;
    chblix_t blocks[count];
    for(int64_t i = 0; i < count; i++){
        chblix_t block = lb_alloc(poop_idx);
        blocks[i] = block;
        lb_write(poop_idx, &block, str, sizeof(str), 0);
    }
    for(int64_t i = 0; i < count; i++){
        chblix_t block = blocks[i];
        char* read_str = malloc(sizeof(str));
        lb_read(poop_idx, &block, read_str, sizeof(str), 0);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
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
    int64_t ppidx = lb_ppl_init(block_size);
    int64_t count = (int64_t)(PAGE_SIZE - sizeof_Page_Header) / block_size;
    count--; // to prevent page_pool expand
    chblix_t blocks[count];
    for(int64_t i = 0; i < count ; i++){
        chblix_t block = lb_alloc(ppidx);
        blocks[i] = block;
        lb_write(ppidx, &block, str, sizeof(str), 0);
    }
    pg_destroy();
    ch_destroy();
    close_file();
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    for(int64_t i = 0; i < count; i++){
        chblix_t block = blocks[i];
        char* read_str = malloc(sizeof(str));
        lb_read(ppidx, &block, read_str, sizeof(str), 0);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
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
    int64_t ppidx = lb_ppl_init(block_size);
    page_pool_t* ppl = ppl_load(ppidx);
    linked_page_t* linkedPage = lp_load(ppl->current_idx);
    int64_t count = lp_useful_space_size(linkedPage) / ppl->block_size;
    chblix_t blocks[3 * count];
    for(int64_t i = 0; i < 3 * count ; i++){
        chblix_t block = lb_alloc(ppidx);
        blocks[i] = block;
        lb_write(ppidx, &block, str, sizeof(str), 0);
    }
    for(int64_t i = count; i < 2 * count ; i++){
        chblix_t block = blocks[i];
        lb_dealloc(ppidx, &block);
    }
    int64_t page = pg_alloc();
    assert(page == blocks[count].chunk_idx);

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
    int64_t ppidx = lb_ppl_init(block_size);
    int64_t count = 2;
    chblix_t blocks[count];
    for(int64_t i = 0; i < count; i++){
        chblix_t block = lb_alloc(ppidx);
        blocks[i] = block;
        lb_write(ppidx, &block, str, sizeof(str), PAGE_SIZE+1000);
    }
    for(int64_t i = 0; i < count; i++){
        chblix_t block = blocks[i];
        char* read_str = malloc(sizeof(str));
        lb_read(ppidx, &block, read_str, sizeof(str), PAGE_SIZE+1000);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
    pg_destroy();
    ch_destroy();
    delete_file();
}

DEFINE_TEST(varchar){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    char str1[] = "0123456789ABCDEF";
    char str2[] = "45678ABCDEF";
    int64_t block_size = 10;
    int64_t ppidx = lb_ppl_init(block_size);
    int64_t count = 2;
    chblix_t blocks[count];
    chblix_t block = lb_alloc(ppidx);
    blocks[0] = block;
    lb_write(ppidx, &block, str1, sizeof(str1), 0);

    block = lb_alloc(ppidx);
    blocks[1] = block;
    lb_write(ppidx, &block, str2, sizeof(str2), 0);

    char* read_str1 = malloc(sizeof(str1));
    lb_read(ppidx, &blocks[0], read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    free(read_str1);

    char* read_str2 = malloc(sizeof(str2));
    lb_read(ppidx, &blocks[1], read_str2, sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    free(read_str2);

    pg_destroy();
    ch_destroy();
    delete_file();
}


int main(){
    RUN_SINGLE_TEST(write_read);
    RUN_SINGLE_TEST(several_write);
    RUN_SINGLE_TEST(close_and_open);
    RUN_SINGLE_TEST(dealloc);
    RUN_SINGLE_TEST(ultra_wide_page);
    RUN_SINGLE_TEST(varchar);
}