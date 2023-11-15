#include "../src/test.h"
#include "../src/backend/io/caching.h"
#include "backend/io/pager.h"
#include "backend/page_pool/page_pool.h"
#include "backend/page_pool/linked_blocks.h"

DEFINE_TEST(write_read){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    char str[] = "12345678";
    int64_t poop_idx = lb_ppl_init(9);
    chblix_t block1 = lb_alloc(poop_idx);
    assert(lb_write(poop_idx, &block1, str, sizeof(str), 0) == LB_SUCCESS);
    char* read_str = malloc(sizeof(str));
    lb_read(poop_idx, &block1, read_str, sizeof(str), 0);
    assert(strcmp(str, read_str) == 0);
    free(read_str);
    pg_delete();
}

DEFINE_TEST(several_write){
    assert(pg_init("test.db") == PAGER_SUCCESS);
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
    pg_delete();
}

DEFINE_TEST(close_and_open){
    assert(pg_init("test.db") == PAGER_SUCCESS);
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
    pg_close();
    assert(pg_init("test.db") == PAGER_SUCCESS);
    for(int64_t i = 0; i < count; i++){
        chblix_t block = blocks[i];
        char* read_str = malloc(sizeof(str));
        lb_read(ppidx, &block, read_str, sizeof(str), 0);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
    pg_delete();
}

DEFINE_TEST(dealloc){
    assert(pg_init("test.db") == PAGER_SUCCESS);
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

    pg_delete();

}

DEFINE_TEST(ultra_wide_page){
    assert(pg_init("test.db") == PAGER_SUCCESS);
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
    pg_delete();
}

DEFINE_TEST(varchar){
    assert(pg_init("test.db") == PAGER_SUCCESS);
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

    pg_delete();
}

DEFINE_TEST(foreach){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    char str[] = "12345678";
    int64_t block_size = 9;
    int64_t poop_idx = lb_ppl_init(block_size);
    int64_t count = 2;
    chblix_t blocks[count];
    for(int64_t i = 0; i < count; i++){
        chblix_t block = lb_alloc(poop_idx);
        blocks[i] = block;
        lb_write(poop_idx, &block, str, sizeof(str), 0);
    }
    page_pool_t* ppl = ppl_load(poop_idx);
    lb_for_each(chblix, ppl){
        char* read_srt = malloc(sizeof(str));
        lb_read(poop_idx, &chblix, read_srt, sizeof(str), 0);
        assert(strcmp(str, read_srt) == 0);
        free(read_srt);
    }
    pg_delete();
}

DEFINE_TEST(insert_number){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    int64_t num = 123456789;
    int64_t pool_idx = lb_ppl_init(sizeof(num));
    chblix_t block = lb_alloc(pool_idx);
    lb_write(pool_idx, &block, &num, sizeof(num), 0);
    int64_t read_num;
    lb_read(pool_idx, &block, &read_num, sizeof(num), 0);
    assert(num == read_num);
    pg_delete();
}

DEFINE_TEST(big_string){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    char* str = "This is a big string\n "
                "with multiple lines\n "
                "and it is very long\n"
                "and it is very long\n";
    int64_t ppl = lb_ppl_init(30);
    chblix_t block = lb_alloc(ppl);
    lb_write(ppl, &block, str, strlen(str) + 1, 0);
    char* read_str = malloc(strlen(str) + 1);
    lb_read(ppl, &block, read_str, strlen(str) + 1, 0);
    assert(strcmp(str, read_str) == 0);
    free(read_str);
    pg_delete();
}


int main(){
    RUN_SINGLE_TEST(write_read);
    RUN_SINGLE_TEST(several_write);
    RUN_SINGLE_TEST(close_and_open);
    RUN_SINGLE_TEST(dealloc);
    RUN_SINGLE_TEST(ultra_wide_page);
    RUN_SINGLE_TEST(varchar);
    RUN_SINGLE_TEST(foreach);
    RUN_SINGLE_TEST(insert_number);
    RUN_SINGLE_TEST(big_string);
}