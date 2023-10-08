#pragma once
#include <stdint.h>
#include "../pager/pager.h"
#include "schema.h"
#include "../../utils/pool.h"
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

typedef struct _Row{
    void* data;
} Row;

typedef struct{
    Schema* schema;
    Pool* pool;
} Table;