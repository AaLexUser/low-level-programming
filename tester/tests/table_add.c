#include "../src/test.h"
#include "../../src/backend/table/table.h"

DEFINE_TEST(table_insert){
    Schema* schema = create_schema("test");
    schema_add_int_field(schema, "id");
    schema_add_char_field(schema, "name", 3);
    schema_add_float_field(schema, "score");
    schema_add_bool_field(schema, "pass");

    Table *table = create_table("test", schema);
    void* data = malloc(schema->slot_size);
    int64_t id = 1;
    size_t offset = 0;
    memcpy(data, &id, sizeof(int64_t));
    offset += sizeof(int64_t);
    char* name = "aaa";
    strncpy(data + offset, name, sizeof(char) * 3);
    offset += sizeof(char) * 4;
    float score = 1.1f;
    memcpy(data + offset, &score, sizeof(float));
    offset += sizeof(float);
    bool pass = true;
    memcpy(data + offset, &pass, sizeof(bool));
    table_insert(table, data);
    free(data);
    LinkedListIterator* lli = create_linked_list_iterator(table->rows_chblidx);
    while(iterator_has_next(lli)){
        Chblidx* chblidx = iterator_next_data(lli);
        void* slot = pool_addr_from_chblidx(table->pool, chblidx);
        LinkedListIterator* schfl = create_linked_list_iterator(table->schema->fields);
        while (iterator_has_next(schfl)){
            Field* field = iterator_next_data(schfl);
            switch (field->type) {
                case INT: {
                    int64_t data = 0;
                    memcpy(&data, slot + field->offset, field->length );
                    assert(data == 1);
                    break;
                }
                case FLOAT: {
                    float data = 0;
                    memcpy(&data, slot + field->offset, field->length );
                    assert(data == 1.1f);
                    break;
                }
                case CHAR: {
                    char* data = malloc(field->length);
                    memcpy(data, slot + field->offset, field->length );
                    assert(strcmp(data, "aaa") == 0);
                    free(data);
                    break;
                }
                case BOOL: {
                    bool data = 0;
                    memcpy(&data, slot + field->offset, field->length );
                    assert(data == true);
                    break;
                }
                default:
                    break;
            }
        }
    }
    free_table(table);
}

int main() {
    RUN_SINGLE_TEST(table_insert);
    return 0;
}