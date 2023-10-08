#ifndef STRING_H
#define STRING_H
#include <string.h>
#include <stdlib.h>
typedef struct __attribute__((__packed__)){
    uint64_t length;
    char* data;
} String;

int32_t compare_strings(String* a, String* b);
void free_string(String* str);
String* create_string(const char* str);
void* string_serialize(String* string);
String* string_deserialize(void* buffer);
#endif