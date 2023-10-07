#ifndef PAGE_H
#define PAGE_H
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct{
    bool exists;
    size_t value;
} PossibleValue;

typedef struct Page{
    uint8_t* data;
    struct Page* next_page;
    bool is_dirty;
} Page;

#endif