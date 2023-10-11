#ifndef DB_H
#define DB_H
#include "../io/file_manager.h"
#include "../table/table_manager.h"
#include <string.h>
typedef struct{
    FileManager *file_manager;
    TableManager *table_manager;
} Database;
Database* db_open(const char* filename);
void db_close(Database* db);
#endif