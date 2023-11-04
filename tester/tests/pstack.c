#include "../src/test.h"
#include "../src/backend/io/caching.h"
#include "../src/backend/pstack/pstack.h"
#include "../src/utils/logger.h"
#include <stdio.h>

DEFINE_TEST(write_and_read){
    assert(init_file("test.db") == 0);
    if(get_file_size() != 0){
        assert(delete_file() == 0);
        assert(init_file("test.db") == 0);
    }
    uint64_t numbers = 12345678;
    ch_init();
    PStack_List* pstl = pst_list_init(sizeof(uint64_t), 0);
    pst_push(pstl, &numbers);
    uint64_t read_numbers = 0;
    pst_pop(pstl, &read_numbers);
    assert(numbers == read_numbers);
    pst_list_destroy(pstl);
    ch_destroy();
    delete_file();
}

DEFINE_TEST(expand_and_reduce){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    uint64_t count = (PAGE_SIZE - sizeof_PStack_Header) / sizeof(uint64_t);
    PStack_List* pstl = pst_list_init(sizeof(uint64_t), 0);
    for(uint64_t i = 0; i < 2 * count; i++){
        pst_push(pstl, &i);
    }
    for(int64_t i = (int64_t)(2 * count) - 1; i >= 0; i--){
        uint64_t read_numbers;
        pst_pop(pstl, &read_numbers);
        assert(i == read_numbers);
    }

    pst_list_destroy(pstl);
    ch_destroy();
    delete_file();
}

DEFINE_TEST(different_type){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    char* str = "Hello world";
    uint64_t count = (PAGE_SIZE - sizeof_PStack_Header) / (strlen(str) + 1);
    PStack_List* pstl = pst_list_init((strlen(str) + 1), 0);
    for(uint64_t i = 0; i < 2 * count; i++){
        pst_push(pstl, str);
    }
    for(int64_t i = (int64_t)(2 * count) - 1; i >= 0; i--){
        char *read_str = malloc(strlen(str) + 1);
        pst_pop(pstl, read_str);
        assert(strcmp(str,read_str) == 0);
    }

    pst_list_destroy(pstl);
    ch_destroy();
    delete_file();
}

DEFINE_TEST(close_and_open){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    uint64_t count = (PAGE_SIZE - sizeof_PStack_Header) / sizeof(uint64_t);
    PStack_List* pstl = pst_list_init(sizeof(uint64_t), 0);
    logger(LL_DEBUG, __func__, "Writing to file");
    for(uint64_t i = 0; i < 2 * count; i++){
        if(i == count){
            logger(LL_DEBUG, __func__, "Fist half");
        }
        pst_push(pstl, &i);
    }
    pst_list_destroy(pstl);
    ch_destroy();
    close_file();
    logger(LL_DEBUG, __func__, "Reopening file");
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pstl = pst_list_init(sizeof(uint64_t), 0);
    logger(LL_DEBUG, __func__, "Reading from file");
    for(int64_t i = (int64_t)(2 * count) - 1; i >= 0; i--){
        uint64_t read_numbers;
        pst_pop(pstl, &read_numbers);
        assert(i == read_numbers);
    }
    pst_list_destroy(pstl);
    ch_destroy();
    delete_file();
}

DEFINE_TEST(remove_from_cache_current){
    if(init_file("test.db") == -1){
        exit(EXIT_FAILURE);
    }
    uint64_t numbers = 12345678;
    ch_init();
    PStack_List* pstl = pst_list_init(sizeof(uint64_t), 0);
    pst_push(pstl, &numbers);
    ch_remove(pstl->current_idx);
    uint64_t read_numbers;
    pst_pop(pstl, &read_numbers);
    assert(numbers == read_numbers);
    pst_list_destroy(pstl);
    ch_destroy();
    delete_file();
}

int main(){
    RUN_SINGLE_TEST(write_and_read);
    RUN_SINGLE_TEST(expand_and_reduce);
    RUN_SINGLE_TEST(different_type);
    RUN_SINGLE_TEST(close_and_open);
    RUN_SINGLE_TEST(remove_from_cache_current);
}