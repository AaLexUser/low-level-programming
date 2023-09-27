#pragma once
#include <string.h>
#include "../table/table.h"
#include "../pager/pager.h"

Table* db_open(const char* filename);
void db_close(Table* table);