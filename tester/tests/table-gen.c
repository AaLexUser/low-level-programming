#include "../src/test.h"
#include "backend/io/pager.h"
#include "backend/journal/metatab.h"
#include "generator/table-gen.h"

DEFINE_TEST(generate_table){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    int64_t mtabidx = mtab_init();
    int64_t vch_mgr = vch_init();
    gentab_mgr gentabMgr;
    gentabMgr.next_index = 0;
    int64_t tablix = gen_table(mtabidx, &gentabMgr, 100);
    assert(tablix != TABLE_FAIL);
    tab_print(tablix);
    assert(tab_drop(mtabidx, tablix) == TABLE_SUCCESS);
    pg_delete();
}

int main(){
    RUN_SINGLE_TEST(generate_table);
    return 0;
}