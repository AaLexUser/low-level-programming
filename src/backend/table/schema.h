#ifndef SCHEMA_H
#define SCHEMA_H
#include <string.h>
#include "../data_type.h"
#include "../../utils/linked_list.h"
typedef struct{
    char* name;
    data_type type;
    size_t length;
    size_t offset;
} Field;

typedef struct{
    char* name;
    LinkedList* fields;
    size_t slot_size;
    uint32_t lpid;
} Schema;

Schema* create_schema(const char* name);
Schema* clear_schema(Schema* schema);
Schema* free_schema(Schema* schema);
void schema_add_field(Schema* schema, const char* name, data_type type, size_t length);
Field* schema_get_field(Schema* schema, const char* name);
data_type schema_get_field_type(Schema* schema, const char* name);
size_t schema_get_field_length(Schema* schema, const char* name);
size_t schema_get_field_offset(Schema* schema, const char* name);
void schema_add_int_field(Schema* schema, const char* name);
void schema_add_float_field(Schema* schema, const char* name);
void schema_add_string_field(Schema* schema, const char* name, size_t length);
void schema_add_bool_field(Schema* schema, const char* name);

#endif