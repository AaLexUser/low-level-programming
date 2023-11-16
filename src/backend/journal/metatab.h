#pragma once

#include "backend/table/table_base.h"

int64_t mtab_init();
int64_t mtab_find_tab(int64_t mtabidx, const char* name);
int mtab_add(int64_t mtabidx, const char* name, int64_t index);