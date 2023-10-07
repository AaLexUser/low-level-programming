#ifndef STRING_H
#define STRING_H
#include <string.h>
typedef struct __attribute__((__packed__)){
    size_t length;
    char* data;
} String;

#endif