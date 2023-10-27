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
void* get_mmaped_data();
int mmap_page(off_t offset);
int sync_page();
int unmap_page();
int close_file();
int delete_file();
int init_page();
int write_page(void* src, uint64_t size, off_t offset);
int read_page(void* dest, uint64_t size, off_t offset);
#endif