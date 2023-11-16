
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Must supply a database filename.\n");
        exit(EXIT_FAILURE);
    }
//    char* filename = argv[1];
//    Database* db = db_open(filename);
//    db_close(db);
//    HT(int,int) ht;
//    ht_init(ht);


//
//    setbuf(stdout, NULL);
//    Schema* schema = create_schema("test");
//    schema_add_int_field(schema, "id");
//    schema_add_char_field(schema, "name", 3);
//    schema_add_float_field(schema, "score");
//    schema_add_bool_field(schema, "pass");
//    table_manager_add_table_inplace(db->table_manager, "test", schema);
}