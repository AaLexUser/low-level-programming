#include "../src/test.h"
#include "../src/backend/io/file_manager.h"
#include "../src/backend/io/caching.h"
#include "backend/io/pager.h"
#include "backend/io/linked_pages.h"

DEFINE_TEST(simple_to_start){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    int64_t lp_idx = lp_init();
    assert(lp_idx != -1);
    linked_page_t* lp = lp_load(lp_idx);
    assert(lp->next_page == -1);
    assert(lp->page_index != -1);
    assert(lp_delete(lp->page_index) == LP_SUCCESS);
    pg_destroy();
    ch_destroy();
    delete_file();
}

DEFINE_TEST(write_read_to_single_page){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    int64_t lp = lp_init();
    assert(lp != -1);
    char str[] = "12345678";
    assert(lp_write(lp, str, sizeof(str), 0) == LP_SUCCESS);
    char* read_str = malloc(sizeof(str));
    assert(lp_read_copy(lp, read_str, sizeof(str), 0) == LP_SUCCESS);
    assert(strcmp(str, read_str) == 0);
    free(read_str);
    assert(lp_delete(lp) == LP_SUCCESS);
    pg_destroy();
    ch_destroy();
    delete_file();
}

DEFINE_TEST(write_reade_to_two_page){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    int64_t lp = lp_init();
    assert(lp != -1);
    char str1[] = "12345678";
    assert(lp_write(lp, str1, sizeof(str1), PAGE_SIZE + 40) == LP_SUCCESS);
    char str2[] = "abcdefgh";
    assert(lp_write(lp, str2, sizeof(str2), 12) == LP_SUCCESS);
    char* read_str1 = malloc(sizeof(str1));
    assert(lp_read_copy(lp, read_str1, sizeof(str1), PAGE_SIZE + 40) == LP_SUCCESS);
    assert(strcmp(str1, read_str1) == 0);
    free(read_str1);
    char* read_str2 = malloc(sizeof(str2));
    assert(lp_read_copy(lp, read_str2, sizeof(str2), 12) == LP_SUCCESS);
    assert(strcmp(str2, read_str2) == 0);
    free(read_str2);
    assert(lp_delete(lp) == LP_SUCCESS);
    pg_destroy();
    ch_destroy();
    delete_file();
}

DEFINE_TEST(close_and_open){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    int64_t lp = lp_init();
    assert(lp != -1);
    char str1[] = "12345678";
    assert(lp_write(lp, str1, sizeof(str1), PAGE_SIZE + 40) == LP_SUCCESS);
    char str2[] = "abcdefgh";
    assert(lp_write(lp, str2, sizeof(str2), 12) == LP_SUCCESS);
    pg_destroy();
    ch_destroy();
    close_file();

    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    assert(lp != -1);
    char* read_str1 = malloc(sizeof(str1));
    assert(lp_read_copy(lp, read_str1, sizeof(str1), PAGE_SIZE + 40) == LP_SUCCESS);
    assert(strcmp(str1, read_str1) == 0);
    free(read_str1);
    char* read_str2 = malloc(sizeof(str2));
    assert(lp_read_copy(lp, read_str2, sizeof(str2), 12) == LP_SUCCESS);
    assert(strcmp(str2, read_str2) == 0);
    free(read_str2);
    assert(lp_delete(lp) == LP_SUCCESS);
    pg_destroy();
    ch_destroy();
    delete_file();
}

DEFINE_TEST(caching_remove){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    int64_t lp = lp_init();
    assert(lp != -1);
    char str1[] = "12345678";
    assert(lp_write(lp, str1, sizeof(str1), CH_MAX_MEMORY_USAGE + 40) == LP_SUCCESS);
    char str2[] = "abcdefgh";
    assert(lp_write(lp, str2, sizeof(str2), 12) == LP_SUCCESS);
    char* read_str1 = malloc(sizeof(str1));
    assert(lp_read_copy(lp, read_str1, sizeof(str1), CH_MAX_MEMORY_USAGE + 40) == LP_SUCCESS);
    assert(strcmp(str1, read_str1) == 0);
    free(read_str1);
    char* read_str2 = malloc(sizeof(str2));
    assert(lp_read_copy(lp, read_str2, sizeof(str2), 12) == LP_SUCCESS);
    assert(strcmp(str2, read_str2) == 0);
    free(read_str2);
    assert(lp_delete(lp) == LP_SUCCESS);
    pg_destroy();
    ch_destroy();
    delete_file();
}


int main(){
    RUN_SINGLE_TEST(simple_to_start);
    RUN_SINGLE_TEST(write_read_to_single_page);
    RUN_SINGLE_TEST(close_and_open);
    RUN_SINGLE_TEST(caching_remove);
}