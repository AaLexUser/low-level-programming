#ifndef VARCHAR_MANAGER_H_
#define VARCHAR_MANAGER_H_

#include "../../utils/chunky_pool.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define VARCHAR_BLOCK_SIZE 16
#define VARCHAR_NUM_OF_BLOCKS 20
#define DATA_PER_BLOCK_SIZE (VARCHAR_BLOCK_SIZE - sizeof (uint32_t))
typedef struct _VarcharManager{
    Pool* pool;
} VarcharManager;

VarcharManager* varcharmanager_create();
void varcharmanager_destroy(VarcharManager* manager);
Chblidx* get_pfchblidx_from_str(VarcharManager* vm, const char* str);
char* get_str_from_pfbi(VarcharManager* vm, Chblidx* pfchblidx, size_t str_len);





#endif