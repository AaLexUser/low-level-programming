#pragma once
#include <string.h>
#include "../../../backend/io/input_buffer.h"
#include "../../../backend/table/table.h"
typedef enum {PREPARE_SUCCESS,
    PREPARE_UNRECOGNIZED_STATEMENT,
    PREPARE_SYNTAX_ERROR,
    PREPARE_STRING_TOO_LONG,
    PREPARE_NEGATIVE_ID
} PrepareResult;
typedef enum {STATEMENT_INSERT, STATEMENT_SELECT} StatementType;

typedef struct{
    StatementType type;
    Row row_to_insert;
} Statement;

PrepareResult prepare_insert(InputBuffer* inputBuffer, Statement* statement);
PrepareResult prepare_statement(InputBuffer* inputBuffer, Statement* statement);
