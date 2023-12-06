#pragma once
#include "../command_processor/parser/parse.h"
#include "../../backend/cursor/cursor.h"
#include "utils/custom_print.h"
typedef enum {EXECUTE_TABLE_FULL, EXECUTE_SUCCESS} ExecuteResult;
void serialize_row(Row* source, void* destination);
void deserialize_row(void* source, Row* destination);
ExecuteResult execute_insert(Statement* statement, Table* table);
ExecuteResult execute_select(Statement* statement, Table* table);
ExecuteResult execute_statement(Statement* statement, Table* table);