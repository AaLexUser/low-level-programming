#ifndef SCHEMA_H
#define SCHEMA_H
#include "../../utils/cstring.h"
#include "../../utils/linked_list.h"
#include "../../utils/pool.h"
#include "../data_type.h"
#include <string.h>
#define MAX_NAME_LENGTH 32
typedef struct __attribute__((packed)){
    char name[MAX_NAME_LENGTH];
    DATA_TYPE type;
    uint64_t length;
    uint64_t offset;
} Field;

typedef struct{
    char name[MAX_NAME_LENGTH];
    LinkedList* fields;
    size_t slot_size;
} Schema;

Schema* create_schema(const char* name);
void clear_schema(Schema* schema);
void free_schema(Schema* schema);
void schema_add_field(Schema* schema, const char* name, DATA_TYPE type, size_t length);
Field* schema_get_field(Schema* schema, const char* name);
DATA_TYPE schema_get_field_type(Schema* schema, const char* name);
size_t schema_get_field_length(Schema* schema, const char* name);
size_t schema_get_field_offset(Schema* schema, const char* name);
void schema_add_int_field(Schema* schema, const char* name);
void schema_add_float_field(Schema* schema, const char* name);
void schema_add_varchar_field(Schema* schema, const char* name);
void schema_add_char_field(Schema* schema, const char* name, size_t length);
void schema_add_bool_field(Schema* schema, const char* name);
void* schema_serialize(Schema* schema);
Schema* schema_deserialize(void* buffer);
#endif