#pragma once

#include "core/io/pager.h"
#include "backend/journal/metatab.h"

typedef struct db{
    pager_t* pager;
    int64_t meta_table_idx;
    int64_t varchar_mgr_idx;
} db_t;

void* db_init(const char* filename);
int db_close(db_t* db);
int db_drop(db_t* db);

enum dbsts_t {DB_SUCCESS = 0, DB_FAIL = -1};