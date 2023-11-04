#include "../src/test.h"
#include "../src/backend/io/pager.h"
#include "backend/io/caching.h"
#include <stdio.h>

DEFINE_TEST(allocate_deallocate){
    assert(init_file("test.db") == FILE_SUCCESS);
    ch_init();
    pg_init();
    int64_t page1 = pg_alloc();
    assert(page1 == 1);
    int64_t page2 = pg_alloc();
    assert(page2 == 2);
    int64_t page3 = pg_alloc();
    assert(page3 == 3);
    pg_dealloc(page2);
    int64_t page4 = pg_alloc();
    assert(page4 == 2);
    pg_dealloc(page1);
    pg_destroy();
    ch_destroy();
    delete_file();
}


int main(){
    RUN_SINGLE_TEST(allocate_deallocate);
}