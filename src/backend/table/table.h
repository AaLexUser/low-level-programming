#pragma once
#include "../../utils/pool.h"
#include "schema.h"
#include <inttypes.h>
#include <stdio.h>


typedef struct _Row{
    void* data;
} Row;

typedef struct{
    char name[MAX_NAME_LENGTH];
    Schema* schema;
    Pool* pool;
    LinkedList* rows_chblidx;
} Table;

Table* create_table(const char* name, Schema* schema);
void free_table(void* ptr);
void table_insert(Table* table, void* data);
void table_select(Table* table);