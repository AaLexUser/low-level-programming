#pragma once
#include "file_manager.h"

int ch_init();
int ch_destroy();
size_t ch_size();
size_t ch_used();
void* ch_cached_page(size_t index);
size_t ch_page_index(off_t offset);
int ch_reserve(size_t new_capacity);
int ch_put(uint64_t page_index, void* mapped_page_ptr);
void* ch_get(uint64_t page_index);
uint64_t ch_new_page();
void* ch_load_page(uint64_t page_index);
int ch_write(uint64_t page_index, void* src, size_t size, off_t offset);
int ch_copy_read(uint64_t page_index, void* dest, size_t size, off_t offset);
uint64_t ch_unmap_some_pages();
int ch_delete_last_page();