#include "table_manager.h"
#include "../../utils/linked_list.h"
#include "schema.h"
#include "table.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TableManager* create_table_manager(){
    TableManager* manager = malloc(sizeof(TableManager));
    manager->tables = create_linked_list_with_free(free_table);
    return manager;
}

void free_table_manager(TableManager* manager){
    if(manager == NULL){
        return;
    }
    free_linked_list(manager->tables);
    free(manager);
}

void table_manager_add_table(TableManager* manager, Table* table){
    linked_list_add_node(manager->tables, table);
}

void table_manager_add_table_inplace(TableManager* manager, const char* name, Schema* schema){
    Table* table = create_table(name, schema);
    linked_list_add_node(manager->tables, table);
}

Table* table_manager_get_table(TableManager* manager, const char* name){
    Table* table;
    LinkedListIterator* llr = create_linked_list_iterator(manager->tables);
    while (iterator_has_next(llr))
    {
        table = iterator_next_data(llr);
        if(strcmp(table->schema->name, name) == 0){
            return table;
        }
    }
    return NULL;
}

void table_manager_remove_table(TableManager* manager, const char* name){
    Table* table;
    LinkedListIterator* llr = create_linked_list_iterator(manager->tables);
    while (iterator_has_next(llr))
    {
        table = iterator_next_data(llr);
        if(strcmp(table->schema->name, name) == 0){
            iterator_remove(llr);
            return;
        }
    }
}

void table_manager_print_tables(TableManager* manager){
    Table* table;
    LinkedListIterator* llr = create_linked_list_iterator(manager->tables);
    while (iterator_has_next(llr))
    {
        table = iterator_next_data(llr);
        printf("%s\n", table->schema->name);
    }
}

void table_manager_print_table(TableManager* manager, const char* name){
    Table* table;
    LinkedListIterator* llr = create_linked_list_iterator(manager->tables);
    while (iterator_has_next(llr))
    {
        table = iterator_next_data(llr);
        if(strcmp(table->schema->name, name) == 0){
            table_select(table);
            return;
        }
    }
}

