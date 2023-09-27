#pragma once
#include <stdbool.h>
#include <stdlib.h>
#include "../pager/pager.h"
#include "../table/table.h"
typedef struct {
    Table* table;
    uint32_t row_num;
    bool end_of_table; //Indicates a position one past the last element
} Cursor;

Cursor* table_start(Table* table);
Cursor* table_end(Table* table);
void cursor_advance(Cursor* cursor);
void* cursor_value(Cursor* cursor);