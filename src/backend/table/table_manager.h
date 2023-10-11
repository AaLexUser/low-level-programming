#ifndef TABLE_MANAGER_H_
#define TABLE_MANAGER_H_
#include "../../utils/linked_list.h"
#include "../varchar/varchar_manager.h"
#include "schema.h"
#include "table.h"
#include <stdlib.h>

typedef struct {
    LinkedList* tables;
    VarcharManager* varcharManager;
} TableManager;

TableManager* create_table_manager();
void free_table_manager(TableManager* manager);
void table_manager_add_table(TableManager* manager, Table* table);
void table_manager_add_table_inplace(TableManager* manager, const char* name, Schema* schema);
Table* table_manager_get_table(TableManager* manager, const char* name);
void table_manager_remove_table(TableManager* manager, const char* name);
void table_manager_print_tables(TableManager* manager);
void table_manager_print_table(TableManager* manager, const char* name);

#endif