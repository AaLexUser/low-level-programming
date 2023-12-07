#include "db.h"

void* db_init(const char* filename){
    db_t* db = malloc(sizeof(db_t));
    if(pg_init(filename) != PAGER_SUCCESS){
        return NULL;
    }
    db->meta_table_idx = mtab_init();
    if(db->meta_table_idx == TABLE_FAIL){
        return NULL;
    }

    db->varchar_mgr_idx = vch_init();
    if(db->varchar_mgr_idx == TABLE_FAIL){
        return NULL;
    }
    return db;
}
int db_close(db_t* db){
    int res =  pg_close() == PAGER_SUCCESS ? DB_SUCCESS : DB_FAIL;
    free(db);
    return res;
}
int db_drop(db_t* db){
    int res = pg_delete() == PAGER_SUCCESS ? DB_SUCCESS : DB_FAIL;
    free(db);
    return res;
}