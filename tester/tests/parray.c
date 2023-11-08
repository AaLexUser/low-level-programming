#include "../src/test.h"
#include "../src/backend/io/file_manager.h"
#include "../src/backend/io/caching.h"
#include "backend/io/pager.h"
#include "backend/io/linked_pages.h"
#include "backend/utils/parray.h"

DEFINE_TEST(write_and_read){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    char str[] = "1234567";
    int64_t array = pa_init(sizeof(str));
    for(int64_t i = 0; i < 1000; i++){
        pa_append(array, str);
    }
    for(int64_t i = 0; i < 1000; i++){
        char* read_str = malloc(sizeof(str));
        pa_at(array, i, read_str);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
    pa_destroy(array);
    pg_destroy();
    ch_destroy();
    delete_file();
}

DEFINE_TEST(close_and_open){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    char str[] = "1234567";
    int64_t array = pa_init(sizeof(str));
    for(int64_t i = 0; i < 3000; i++){
        pa_append(array, str);
    }
    pg_destroy();
    ch_destroy();
    close_file();

    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    for(int64_t i = 0; i < 3000; i++){
        char* read_str = malloc(sizeof(str));
        pa_at(array, i, read_str);
        assert(strcmp(str, read_str) == 0);
        free(read_str);
    }
    pa_destroy(array);
    pg_destroy();
    ch_destroy();
    delete_file();
}



int main(){
    RUN_SINGLE_TEST(write_and_read);
    RUN_SINGLE_TEST(close_and_open);
}