#pragma once
#include <stdint.h>
#include "../pager/pager.h"
#include "schema.h"
#define size_of_attribute(Struct, Attribute) sizeof(((Struct*)0)->Attribute)

typedef struct{
    Schema* schema;
    void* data[];
} Table;