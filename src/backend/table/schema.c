#include "schema.h"

void clear_field(Field* field){
    free(field->name);
    field->type = DATA_TYPE_UNKNOWN;
    field->length = 0;
    field->offset = 0;
}

void clear_schema(Schema* schema){
    free(schema->name);
    clear_linked_list(schema->fields);
    schema->slot_size = 0;
}

void free_field_data(void* ptr){
    Field* field = (Field*)ptr;
    clear_field(field);
}

void free_field(void* ptr){
    Field* field = (Field*)ptr;
    free_field_data(field);
    free(field);
}

Schema* create_schema(const char* name){
    Schema* schema = malloc(sizeof(Schema));
    schema->fields = create_linked_list_with_free(free_field);
    schema->slot_size = 0;
    strncpy(schema->name, name, MAX_NAME_LENGTH);
    return schema;
}


void free_schema(Schema* schema){
    if(schema == NULL){
        return;
    }
    free(schema->name);
    free_linked_list(schema->fields);
}

void schema_add_field(Schema* schema, const char* name, DATA_TYPE type, size_t length){
    Field* field = malloc(sizeof(Field));
    strncpy(field->name, name, MAX_NAME_LENGTH);
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

DATA_TYPE schema_get_field_type(Schema* schema, const char* name){
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

void schema_add_varchar_field(Schema* schema, const char* name){
    schema_add_field(schema, name, VARCHAR, sizeof(Chblidx));
}

void schema_add_char_field(Schema* schema, const char* name, size_t length){
    schema_add_field(schema, name, CHAR, length + 1);
}

void schema_add_bool_field(Schema* schema, const char* name){
    schema_add_field(schema, name, BOOL, sizeof(bool));
}

LinkedList* schema_get_fields(Schema* schema){
    return schema->fields;
}

void* schema_serialize(Schema* schema){
    if(schema == NULL){
        return NULL;
    }
    uint64_t total_size = sizeof(uint64_t ) + MAX_NAME_LENGTH + schema->fields->size * (MAX_NAME_LENGTH + sizeof(DATA_TYPE) + sizeof(uint64_t));
    void* buffer = malloc(total_size);
    memset(buffer, 0, total_size);
    memcpy(buffer, &total_size, sizeof(uint64_t));
    strncpy(buffer + sizeof(uint64_t), schema->name, MAX_NAME_LENGTH);
    uint64_t offset = sizeof(uint64_t) + MAX_NAME_LENGTH;
    LinkedListIterator* llr = create_linked_list_iterator(schema->fields);
    while(iterator_has_next(llr)){
        Field* field = iterator_next_data(llr);
        strncpy(buffer + offset, field->name, MAX_NAME_LENGTH);
        offset += MAX_NAME_LENGTH;
        memcpy(buffer + offset, &field->type, sizeof(DATA_TYPE));
        offset += sizeof(DATA_TYPE);
        memcpy(buffer + offset, &field->length, sizeof(field->length));
        offset += sizeof(field->length);
    }
    free_linked_list_iterator(llr);
    return buffer;
}

Schema* schema_deserialize(void* buffer){
    if(buffer == NULL){
        return NULL;
    }
    uint64_t total_size = 0;
    memcpy(&total_size, buffer, sizeof(uint64_t));
    char name[MAX_NAME_LENGTH];
    strncpy(name, buffer + sizeof(uint64_t), MAX_NAME_LENGTH);
    Schema* schema = create_schema(name);
    uint64_t offset = sizeof(uint64_t) + MAX_NAME_LENGTH;
    while(offset != total_size){
        char name[MAX_NAME_LENGTH];
        strncpy(name, buffer + offset, MAX_NAME_LENGTH);
        offset += MAX_NAME_LENGTH;
        DATA_TYPE type = DATA_TYPE_UNKNOWN;
        memcpy(&type, buffer + offset, sizeof(DATA_TYPE));
        offset += sizeof(DATA_TYPE);
        uint64_t length = 0;
        memcpy(&length, buffer + offset, sizeof(uint64_t));
        offset += sizeof(uint64_t);
        schema_add_field(schema, name, type, length);
    }
    return schema;
}

