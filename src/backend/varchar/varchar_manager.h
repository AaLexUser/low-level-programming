#ifndef VARCHAR_MANAGER_H_
#define VARCHAR_MANAGER_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "../../utils/pool.h"

#define VARCHAR_BLOCK_SIZE 16
#define VARCHAR_NUM_OF_BLOCKS 20
#define DATA_PER_BLOCK_SIZE (VARCHAR_BLOCK_SIZE - sizeof (uint32_t))
typedef struct _VarcharManager{
    Pool* pool;
} VarcharManager;




#endif