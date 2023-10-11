#include "table.h"
#include <string.h>

#define TABLE_CHUNK_SIZE 4096

Table* create_table(const char* name, Schema* schema){
    Table* table = malloc(sizeof(Table));
    strncpy(schema->name, name, MAX_NAME_LENGTH);
    table->schema = schema;
    table->pool = pool_create_by_chunk_size(TABLE_CHUNK_SIZE, table->schema->slot_size);
    table->rows_chblidx = create_linked_list();
    return table;
}

void free_table(void* ptr){
    Table* table = (Table*)ptr;
    if(table == NULL){
        return;
    }
    free_schema(table->schema);
    pool_destroy(table->pool);
    free_linked_list(table->rows_chblidx);
    free(table);
}

void table_insert(Table* table, void* data){
    void* slot = pool_alloc(table->pool);
    Chblidx* chblidx = chunk_chblidx_from_addr(table->pool->current_chunk, slot);
    linked_list_add_node(table->rows_chblidx, chblidx);
    if(slot == NULL){
        return;
    }
    memcpy(slot, data, table->schema->slot_size);
}

void table_select(Table* table){
    LinkedListIterator* lli = create_linked_list_iterator(table->rows_chblidx);
    while(iterator_has_next(lli)){
        Chblidx* chblidx = iterator_next_data(lli);
        void* slot = pool_addr_from_chblidx(table->pool, chblidx);
        LinkedListIterator* schfl = create_linked_list_iterator(table->schema->fields);
        while (iterator_has_next(schfl)){
            Field* field = iterator_next_data(schfl);
            switch (field->type) {
                case INT: {
                    uint64_t data = 0;
                    memcpy(&data, slot + field->offset, field->length );
                    printf("%"PRId64 " ", data);
                    break;
                }
                case FLOAT: {
                    float data = 0;
                    memcpy(&data, slot + field->offset, field->length );
                    printf("%f ", data);
                    break;
                }
                case CHAR: {
                    char* data = malloc(field->length);
                    memcpy(data, slot + field->offset, field->length );
                    printf("%s ", data);
                    free(data);
                    break;
                }
                case BOOL: {
                    bool data = 0;
                    memcpy(&data, slot + field->offset, field->length );
                    printf("%d ", data);
                    break;
                }
                default:
                    break;
            }
        }
    }
}

void table_delete(Table* table, Chblidx* chblidx){
    void* slot = pool_addr_from_chblidx(table->pool, chblidx);
    pool_dealloc(table->pool, slot);
}

void table_update(Table* table, Chblidx* chblidx, Row* row){
    void* slot = pool_addr_from_chblidx(table->pool, chblidx);
    memcpy(slot, row->data, table->schema->slot_size);
}