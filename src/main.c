#include "backend/table/table.h"
#include "backend/table/schema.h"

int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);
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
    table_select(table);
    free_table(table);
}