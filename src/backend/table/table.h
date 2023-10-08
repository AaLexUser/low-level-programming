#pragma once
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include "schema.h"
#include "../../utils/pool.h"


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
void free_table(Table* table);
void table_insert(Table* table, void* data);
void table_select(Table* table);