#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PAGE_SIZE
#define PAGE_SIZE 4096
#endif

int init_file(const char* file_name);
off_t get_cur_page_offset();
void* get_cur_mmaped_data();
uint64_t number_pages();
uint64_t get_page_index(off_t page_offset);
uint64_t get_current_page_index();
off_t get_page_offset(uint64_t page_index);
uint64_t get_file_size();
int mmap_page(off_t offset);
int sync_page(void* mmaped_data);
int unmap_page(void* mmaped_data);
int close_file();
int delete_file();
int init_page();
int write_page(void* src, uint64_t size, off_t offset);
int read_page(void* dest, uint64_t size, off_t offset);
#endif