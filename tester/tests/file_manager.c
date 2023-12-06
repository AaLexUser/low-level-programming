#include "../src/test.h"
#include "core/io/file.h"
#include <stdio.h>

DEFINE_TEST(write_and_read){
    file_t* file = malloc(sizeof(file_t));
    assert(init_file("test.db", file) == FILE_SUCCESS);
    if(fl_file_size(file) > 0){
        assert(delete_file(file) == FILE_SUCCESS);
        assert(init_file("test.db", file) == FILE_SUCCESS);
    }
    char str[] = "12345678";
    init_page(file);
    write_page(file, str, sizeof(str), 0);
    unmap_page(fl_cur_mmaped_data(file), file);
    mmap_page(0, file);
    char* read_str = malloc(sizeof(str));
    read_page(file, read_str, sizeof(str), 0);
    assert(strcmp(str, read_str) == 0);
    delete_file(file);
    free(file);
}

DEFINE_TEST(two_write){
    file_t* file = malloc(sizeof(file_t));
    assert(init_file("test.db", file) == FILE_SUCCESS);
    if(fl_file_size(file) > 0){
        assert(delete_file(file) == FILE_SUCCESS);
        assert(init_file("test.db", file) == FILE_SUCCESS);
    }
    char str1[] = "12345678";
    init_page(file);
    write_page(file, str1, sizeof(str1), 0);
    unmap_page(fl_cur_mmaped_data(file), file);
    mmap_page(0, file);
    char str2[] = "abcdefg";
    write_page(file, str2, sizeof(str2), sizeof(str1));

    char* read_str2 = malloc(sizeof(str2));
    read_page(file, read_str2,sizeof(str2), sizeof(str1));
    assert(strcmp(str2, read_str2) == 0);
    delete_file(file);
    free(file);
}
DEFINE_TEST(two_pages){
    file_t* file = malloc(sizeof(file_t));
    assert(init_file("test.db", file) == FILE_SUCCESS);
    if(fl_file_size(file) > 0){
        assert(delete_file(file) == FILE_SUCCESS);
        assert(init_file("test.db", file) == FILE_SUCCESS);
    }
    char str1[] = "12345678";
    init_page(file);
    write_page(file, str1, sizeof(str1), 0);
    unmap_page(fl_cur_mmaped_data(file), file);
    init_page(file);
    char str2[] = "abcdefg";
    write_page(file, str2, sizeof(str2), 0);
    unmap_page(fl_cur_mmaped_data(file), file);
    mmap_page(0, file);
    char* read_str1 = malloc(sizeof(str1));
    read_page(file, read_str1,sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    unmap_page(fl_cur_mmaped_data(file), file);
    mmap_page(PAGE_SIZE, file);
    char* read_str2 = malloc(sizeof(str2));
    read_page(file, read_str2,sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    delete_file(file);
    free(file);
}

DEFINE_TEST(delete_last_page){
    file_t* file = malloc(sizeof(file_t));
    assert(init_file("test.db", file) == FILE_SUCCESS);
    if(fl_file_size(file) > 0){
        assert(delete_file(file) == FILE_SUCCESS);
        assert(init_file("test.db", file) == FILE_SUCCESS);
    }
    char str1[] = "12345678";
    init_page(file);
    write_page(file, str1, sizeof(str1), 0);
    unmap_page(fl_cur_mmaped_data(file), file);
    init_page(file);
    char str2[] = "abcdefg";
    write_page(file, str2, sizeof(str2), 0);
    unmap_page(fl_cur_mmaped_data(file), file);
    mmap_page(0, file);
    char* read_str1 = malloc(sizeof(str1));
    read_page(file, read_str1,sizeof(str1), 0);
    assert(strcmp(str1, read_str1) == 0);
    unmap_page(fl_cur_mmaped_data(file), file);
    mmap_page(PAGE_SIZE, file);
    char* read_str2 = malloc(sizeof(str2));
    read_page(file, read_str2,sizeof(str2), 0);
    assert(strcmp(str2, read_str2) == 0);
    assert(unmap_page(fl_cur_mmaped_data(file), file) == 0);
    assert(2 * PAGE_SIZE == (int) fl_file_size(file));
    assert(delete_last_page(file) == 0);
    assert(1 * PAGE_SIZE == (int) fl_file_size(file));
    assert(delete_file(file) == 0);
    free(file);
}

int main(){
    RUN_SINGLE_TEST(write_and_read);
    RUN_SINGLE_TEST(two_write);
    RUN_SINGLE_TEST(two_pages);
    RUN_SINGLE_TEST(delete_last_page);
}