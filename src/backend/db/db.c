#include "db.h"

static void* db_create(void){
    pg_alloc();
    db_t* db = pg_load_page(1);
    if(!db){
        return NULL;
    }
    table_t* meta_tab = mtab_init();
    if(meta_tab == NULL){
        return NULL;
    }
    db->meta_table_idx = table_index(meta_tab);
    db->varchar_mgr_idx = vch_init();
    if(db->varchar_mgr_idx == TABLE_FAIL){
        return NULL;
    }
    return db;
}

void* db_init(const char* filename){
    if(pg_init(filename) != PAGER_SUCCESS){
        return NULL;
    }
    if(pg_max_page_index() == 0){
        return db_create();
    }
    db_t* db = pg_load_page(1);
    if(!db){
        return NULL;
    }
    return db;
}

int db_close(void){
    int res =  pg_close() == PAGER_SUCCESS ? DB_SUCCESS : DB_FAIL;
    return res;
}
int db_drop(void){
    int res = pg_delete() == PAGER_SUCCESS ? DB_SUCCESS : DB_FAIL;
    return res;
}
