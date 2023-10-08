#pragma once
#include <stdint.h>
#include "../pager/pager.h"
#include "schema.h"
#include "../../utils/pool.h"

typedef struct _Row{
    void* data;
} Row;

typedef struct{
    char name[MAX_NAME_LENGTH];
    Schema* schema;
    Pool* pool;
} Table;