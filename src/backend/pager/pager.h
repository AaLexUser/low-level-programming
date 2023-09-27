#pragma once
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define TABLE_MAX_PAGES 100
extern const uint32_t PAGE_SIZE;
typedef struct{
    int file_descriptor;
    uint32_t file_length;
    void* pages[TABLE_MAX_PAGES];
}  Pager;

Pager* pager_open(const char* filename);
void* get_page(Pager* pager, uint32_t page_num);
void pager_flush(Pager* pager, uint32_t page_num, uint32_t size);