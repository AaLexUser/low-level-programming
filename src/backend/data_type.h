#pragma once
#include "journal/varchar_mgr.h"
#include <stdbool.h>
#include <stdlib.h>

typedef enum datatype {DATA_TYPE_UNKNOWN = -1, INT = 0, FLOAT = 1, VARCHAR = 2, CHAR = 3, BOOL = 4} datatype_t;

typedef union data {
    int64_t int_val;
    float float_val;
    char* char_val;
    bool bool_val;
    vch_ticket_t* vch_val;
} data_t;