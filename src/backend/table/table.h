#pragma once
#include "backend/comporator/comporator.h"
#include "backend/journal/metatab.h"
#include "table_base.h"

int64_t tab_init(int64_t mtabidx, const char* name, int64_t schidx);
chblix_t tab_get_row(int64_t tablix, field_t* field, void* value, datatype_t type);
void tab_print(int64_t tablix);
int64_t tab_join(int64_t mtabidx, int64_t leftidx, int64_t rightidx, field_t join_field, const char* name);