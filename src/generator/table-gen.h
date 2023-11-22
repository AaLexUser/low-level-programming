#pragma once
#include "backend/table/table.h"
#include "backend/journal/varchar_mgr.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct gentab_mgr{
    int64_t next_index;
}gentab_mgr;

int64_t gen_table(int64_t mtabidx, gentab_mgr* gentabMgr_ptr, int64_t min_number_of_rows);
int64_t gen_empty_table(int64_t mtabidx, gentab_mgr* gentabMgr_ptr);
void* gen_fill_element(field_t* field);
void* gen_fill_row(schema_t* schema);
int gen_fill_rows(int64_t tablix, int64_t number);