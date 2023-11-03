#include "../src/test.h"
#include "../src/backend/io/caching.h"
#include <stdio.h>

DEFINE_TEST(write_and_read){
    if(init_file("test.db") == -1){
        exit(EXIT_FAILURE);
    }
    char str[] = "12345678";
    ch_init();
    uint64_t page1 = ch_new_page();
    ch_write(page1, str, sizeof(str), 0);
    ch_new_page();
    ch_new_page();
    ch_new_page();
    char* read_str = malloc(sizeof(str));
    ch_copy_read(page1, read_str, sizeof(str), 0);
    assert(strcmp(str, read_str) == 0);
    ch_destroy();
    delete_file();
}

DEFINE_TEST(two_write){
    if(init_file("test.db") == -1){
        exit(EXIT_FAILURE);
    }
    char str1[] = "12345678";
    ch_init();
    uint64_t page1 = ch_new_page();
    ch_write(page1, str1, sizeof(str1), 0);
    ch_new_page();
    ch_new_page();
    ch_new_page();
    char str2[] = "abcdefg";
    ch_write(page1, str2, sizeof(str2), sizeof(str1));

    char* read_str2 = malloc(sizeof(str2));
    ch_copy_read(page1, read_str2, sizeof(str2), sizeof(str1));
    assert(strcmp(str2, read_str2) == 0);

    char* read_str1 = malloc(sizeof(str1));
    ch_copy_read(page1, read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);

    ch_destroy();
    delete_file();
}
DEFINE_TEST(two_pages){
    if(init_file("test.db") == -1){
        exit(EXIT_FAILURE);
    }

    char str1[] = "12345678";
    ch_init();
    uint64_t page1 = ch_new_page();
    if(ch_write(page1, str1, sizeof(str1), 0) == -1){
        ch_destroy();
        delete_file();
        exit(EXIT_FAILURE);
    }
    ch_new_page();
    ch_new_page();
    ch_new_page();
    char str2[] = "abcdefg";
    uint64_t page2 = ch_new_page();
    if(ch_write(page2, str2, sizeof(str2), 0) == -1){
        ch_destroy();
        delete_file();
        exit(EXIT_FAILURE);
    }
    ch_new_page();
    ch_new_page();
    ch_new_page();
    char* read_str1 = malloc(sizeof(str1));
    ch_copy_read(page1, read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    char* read_str2 = malloc(sizeof(str2));
    ch_copy_read(page2, read_str2, sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    ch_destroy();
    delete_file();
}

DEFINE_TEST(delete_last_page){
    assert(init_file("test.db") == 0);
    if(get_file_size() != 0){
        assert(delete_file() == 0);
        assert(init_file("test.db") == 0);
    }

    char str1[] = "12345678";
    ch_init();
    uint64_t page1 = ch_new_page();
    if(ch_write(page1, str1, sizeof(str1), 0) == -1){
        ch_destroy();
        delete_file();
        exit(EXIT_FAILURE);
    }
    ch_new_page();
    ch_new_page();
    ch_new_page();
    char str2[] = "abcdefg";
    uint64_t page2 = ch_new_page();
    if(ch_write(page2, str2, sizeof(str2), 0) == -1){
        ch_destroy();
        delete_file();
        exit(EXIT_FAILURE);
    }
    ch_new_page();
    ch_new_page();
    ch_new_page();
    assert(get_file_size() == 8 * PAGE_SIZE);
    assert(ch_delete_last_page() == 0);
    assert(get_file_size() == 7 * PAGE_SIZE);
    assert(ch_delete_last_page() == 0);
    assert(get_file_size() == 6 * PAGE_SIZE);
    assert(ch_delete_last_page() == 0);
    assert(get_file_size() == 5 * PAGE_SIZE);
    char* read_str1 = malloc(sizeof(str1));
    ch_copy_read(page1, read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    char* read_str2 = malloc(sizeof(str2));
    ch_copy_read(page2, read_str2, sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    assert(ch_delete_last_page() == 0);
    assert(get_file_size() == 4 * PAGE_SIZE);
    assert(ch_new_page() == 4);
    ch_destroy();
    delete_file();
}




//WARMING: THIS TEST MAY CRASH YOUR IDE, TAKES LONG TIME AND MUCH DISK MEM
DEFINE_TEST(cache_memory_save){
    size_t bignumber = 2097152;
    if(init_file("test.db") == -1){
        exit(EXIT_FAILURE);
    }

    char str1[] = "12345678";
    ch_init();
    uint64_t page1 = ch_new_page();
    if(ch_write(page1, str1, sizeof(str1), 0) == -1){
        ch_destroy();
        delete_file();
        exit(EXIT_FAILURE);
    }
    char str[] = "111111111";
    for(size_t i = 0; i < bignumber; i++){
        uint64_t page = ch_new_page();
        if(ch_write(page, str, sizeof(str), 0) == -1){
            ch_destroy();
            delete_file();
            exit(EXIT_FAILURE);
        }
    }
    char str2[] = "abcdefg";
    uint64_t page2 = ch_new_page();
    if(ch_write(page2, str2, sizeof(str2), 0) == -1){
        ch_destroy();
        delete_file();
        exit(EXIT_FAILURE);
    }
    for(size_t i = 0; i < bignumber; i++){
        uint64_t page = ch_new_page();
        if(ch_write(page, str, sizeof(str), 0) == -1){
            ch_destroy();
            delete_file();
            exit(EXIT_FAILURE);
        }
    }
    char* read_str1 = malloc(sizeof(str1));
    ch_copy_read(page1, read_str1, sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    char* read_str2 = malloc(sizeof(str2));
    ch_copy_read(page2, read_str2, sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    ch_destroy();
    delete_file();
}





int main(){
    RUN_SINGLE_TEST(write_and_read);
    RUN_SINGLE_TEST(two_write);
    RUN_SINGLE_TEST(two_pages);
    RUN_SINGLE_TEST(delete_last_page);
  //  RUN_SINGLE_TEST(cache_memory_save);
}