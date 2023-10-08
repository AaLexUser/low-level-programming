#include "table.h"
#define TABLE_CHUNK_SIZE 4096

void create_table(const char* name, Schema* schema){
    Table* table = malloc(sizeof(Table));
    table->schema = schema;
    table->pool = pool_create_by_chunk_size(TABLE_CHUNK_SIZE, table->schema->slot_size);
}

void free_table(Table* table){
    if(table == NULL){
        return;
    }
    free_schema(table->schema);
    pool_destroy(table->pool);
    free(table);
}

void table_insert(Table* table, Row* row){
    void* slot = pool_alloc(table->pool);
    if(slot == NULL){
        return;
    }
    memcpy(slot, row->data, table->schema->slot_size);
}

void table_delete(Table* table, Chblidx* chblidx){
    void* slot = pool_addr_from_chblidx(table->pool, chblidx);
    pool_dealloc(table->pool, slot);
}

void table_update(Table* table, Chblidx* chblidx, Row* row){
    void* slot = pool_addr_from_chblidx(table->pool, chblidx);
    memcpy(slot, row->data, table->schema->slot_size);
}