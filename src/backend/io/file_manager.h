#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct{
    int file_descriptor;
    off_t file_length;
} FileManager;


FileManager* create_file_manager();
FileManager* create_file_manager_by_filename(const char* filename);
void close_file_manager(FileManager* manager);
void* read_file(FileManager* manager, off_t offset, size_t length);
#endif