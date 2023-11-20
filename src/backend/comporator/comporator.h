#pragma once
#include "backend/data_type.h"
#include "backend/journal/varchar_mgr.h"
#include "conditions.h"
#include <stdbool.h>
#include <string.h>

data_t comp_cmp(datatype_t type, void* val1, void* val2);
bool comp_eq(datatype_t type, void* val1, void* val2);
bool comp_compare(datatype_t type, void* val1, void* val2, condition_t cond);