#include "../src/test.h"
#include "backend/io/pager.h"
#include "backend/journal/metatab.h"
#include "generator/table-gen.h"

DEFINE_TEST(generate_table){
    db_t* db = db_init("test.db");
    assert(db != NULL);
    gentab_mgr gentabMgr;
    gentabMgr.next_index = 0;
    int64_t tablix = gen_table(db->meta_table_idx, &gentabMgr, 100);
    assert(tablix != TABLE_FAIL);
    tab_print(db, tablix);
    assert(tab_drop(db, tablix) == TABLE_SUCCESS);
    db_drop(db);
}

int main(){
    RUN_SINGLE_TEST(generate_table);
    return 0;
}