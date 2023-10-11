#include "cstring.h"
#include <stdint.h>
#include <string.h>

int32_t compare_strings(String* a, String* b) {
    return strcmp(a->data, b->data);
}

static void clear_string(String* str){
    str->length = 0;
    free(str->data);
    str->data = NULL;
}

void free_string(String* str) {
    clear_string(str);
    free(str);
}

String* create_string_with_length(const char* str, size_t length){
    String* string = malloc(sizeof(String));
    string->length = length;
    string->data = malloc(length);
    strncpy(string->data, str, length);
    return string;
}

String *create_string(const char *str) {
    String *string = malloc(sizeof(String));
    string->length = strlen(str) + 1;
    string->data = malloc(string->length);
    strncpy(string->data, str, string->length);
    return string;
}

void* string_serialize(String* string){
    size_t size = sizeof(uint64_t) + string->length;
    void* buffer = malloc(size);
    memcpy(buffer, &string->length, sizeof(uint64_t));
    memcpy(buffer + sizeof(uint64_t), string->data, string->length);
    return buffer;
}

String* string_deserialize(void* buffer){
    String* string = malloc(sizeof(String));
    memcpy(&string->length, buffer, sizeof(uint64_t));
    string->data = malloc(string->length);
    memcpy(string->data, buffer + sizeof(uint64_t), string->length);
    return string;
}
