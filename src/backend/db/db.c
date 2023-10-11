#include "db.h"
#include "../io/file_manager.h"

Database* db_open(const char* filename){
    Database* db = malloc(sizeof(Database));
    db->file_manager = create_file_manager_by_filename(filename);
    db->table_manager = create_table_manager();
    return db;
}

void db_close(Database* db){
    if(db == NULL){
        return;
    }
    close_file_manager(db->file_manager);
    free_table_manager(db->table_manager);
    free(db);
}