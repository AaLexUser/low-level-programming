#include "../src/test.h"
#include "../src/backend/io/file_manager.h"
#include <stdio.h>

DEFINE_TEST(write_and_read){
    if(init_file("test.db") == -1){
        exit(EXIT_FAILURE);
    }
    char str[] = "12345678";
    init_page();
    write_page(str, sizeof(str), 0);
    unmap_page(get_cur_mmaped_data());
    mmap_page(0);
    char* read_str = malloc(sizeof(str));
    read_page(read_str, sizeof(str), 0);
    assert(strcmp(str, read_str) == 0);
    delete_file();
}

DEFINE_TEST(two_write){
    if(init_file("test.db") == -1){
        exit(EXIT_FAILURE);
    }
    char str1[] = "12345678";
    init_page();
    write_page(str1, sizeof(str1), 0);
    unmap_page(get_cur_mmaped_data());
    mmap_page(0);
    char str2[] = "abcdefg";
    write_page(str2, sizeof(str2), sizeof(str1));

    char* read_str2 = malloc(sizeof(str2));
    read_page(read_str2,sizeof(str2), sizeof(str1));
    assert(strcmp(str2, read_str2) == 0);

    delete_file();
}
DEFINE_TEST(two_pages){
    if(init_file("test.db") == -1){
        exit(EXIT_FAILURE);
    }
    char str1[] = "12345678";
    init_page();
    write_page(str1, sizeof(str1), 0);
    unmap_page(get_cur_mmaped_data());
    init_page();
    char str2[] = "abcdefg";
    write_page(str2, sizeof(str2), 0);
    unmap_page(get_cur_mmaped_data());
    mmap_page(0);
    char* read_str1 = malloc(sizeof(str1));
    read_page(read_str1,sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    unmap_page(get_cur_mmaped_data());
    mmap_page(PAGE_SIZE);
    char* read_str2 = malloc(sizeof(str2));
    read_page(read_str2,sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    delete_file();
}

DEFINE_TEST(delete_last_page){
    if(init_file("test.db") == -1){
        exit(EXIT_FAILURE);
    }
    char str1[] = "12345678";
    init_page();
    write_page(str1, sizeof(str1), 0);
    unmap_page(get_cur_mmaped_data());
    init_page();
    char str2[] = "abcdefg";
    write_page(str2, sizeof(str2), 0);
    unmap_page(get_cur_mmaped_data());
    mmap_page(0);
    char* read_str1 = malloc(sizeof(str1));
    read_page(read_str1,sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    unmap_page(get_cur_mmaped_data());
    mmap_page(PAGE_SIZE);
    char* read_str2 = malloc(sizeof(str2));
    read_page(read_str2,sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    assert(unmap_page(get_cur_mmaped_data()) == 0);
    assert(2 * PAGE_SIZE == (int) get_file_size());
    assert(delete_last_page() == 0);
    assert(1 * PAGE_SIZE == (int) get_file_size());
    assert(delete_file() == 0);
}

int main(){
    RUN_SINGLE_TEST(write_and_read);
    RUN_SINGLE_TEST(two_write);
    RUN_SINGLE_TEST(two_pages);
    RUN_SINGLE_TEST(delete_last_page);
}