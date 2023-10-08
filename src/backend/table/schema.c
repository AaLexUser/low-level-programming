#include "schema.h"

static void free_field(void* ptr){
    Field* field = (Field*)ptr;
    free(field->name);
    free(field);
}

Schema* create_schema(const char* name){
    Schema* schema = malloc(sizeof(Schema));
    schema->fields = create_linked_list_with_free(free_field);
    schema->slot_size = sizeof (bool);
    schema->name = malloc(strlen(name) + 1);
    strcpy(schema->name, name);
    return schema;
}

Schema* clear_schema(Schema* schema){
    if(schema == NULL){
        return NULL;
    }
    clear_linked_list(schema->fields);
    return schema;
}

Schema* free_schema(Schema* schema){
    if(schema == NULL){
        return NULL;
    }
    free(schema->name);
    free_linked_list(schema->fields);
    free(schema);
    return NULL;
}

void schema_add_field(Schema* schema, const char* name, data_type type, size_t length){
    Field* field = malloc(sizeof(Field));
    field->name = malloc(strlen(name) + 1);
    strcpy(field->name, name);
    field->type = type;
    field->length = length;
    field->offset = schema->slot_size;
    schema->slot_size += length;
    linked_list_add_node(schema->fields, field);
}

Field* schema_get_field(Schema* schema, const char* name){
    Field* field;
    LinkedListIterator* llr = create_linked_list_iterator(schema->fields);
    while (iterator_has_next(llr))
    {
        field = iterator_next_data(llr);
        if(strcmp(field->name, name) == 0){
            free_linked_list_iterator(llr);
            return field;
        }
    }
    free_linked_list_iterator(llr);
    return NULL;
}

data_type schema_get_field_type(Schema* schema, const char* name){
    Field* field = schema_get_field(schema, name);
    if(field == NULL){
        return DATA_TYPE_UNKNOWN;
    }
    return field->type;
}

size_t schema_get_field_length(Schema* schema, const char* name){
    Field* field = schema_get_field(schema, name);
    if(field == NULL){
        return 0;
    }
    return field->length;
}

size_t schema_get_field_offset(Schema* schema, const char* name){
    Field* field = schema_get_field(schema, name);
    if(field == NULL){
        return 0;
    }
    return field->offset;
}

void schema_add_int_field(Schema* schema, const char* name){
    schema_add_field(schema, name, INT, sizeof(int64_t));
}

void schema_add_float_field(Schema* schema, const char* name){
    schema_add_field(schema, name, FLOAT, sizeof(float));
}

void schema_add_string_field(Schema* schema, const char* name, size_t length){
    schema_add_field(schema, name, STRING, length + 1);
}

void schema_add_bool_field(Schema* schema, const char* name){
    schema_add_field(schema, name, BOOL, sizeof(bool));
}

LinkedList* schema_get_fields(Schema* schema){
    return schema->fields;
}
