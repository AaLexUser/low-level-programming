#pragma once
#include "backend/data_type.h"
#include "backend/journal/varchar_mgr.h"
#include "conditions.h"
#include "backend/db/db.h"
#include <stdbool.h>
#include <string.h>

data_t comp_cmp(db_t* db, datatype_t type, void* val1, void* val2);
bool comp_eq(db_t* db, datatype_t type, void* val1, void* val2);
bool comp_compare(db_t* db, datatype_t type, void* val1, void* val2, condition_t cond);