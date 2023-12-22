#include "../src/bench.h"
#include "backend/table/table.h"
#include "utils/logger.h"

const char* TEST_DB = "test.db";
const char* CSV_FILE = "table-mem.csv";
const char* CSV_HEADER= "FileSize;Rows\n";
const int TEST_TIME = 2*60;
const int ALLOCATION = 500;
const int DEALLOCATION = 400;

void insert_rows(table_t* table, schema_t* schema, int64_t start_index, int64_t number_of_rows) {
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            int64_t AGE;
            bool PASS;
    );
    for (int64_t index = start_index; index < start_index + number_of_rows; ++index) {
        row.ID = index;
        row.SCORE = 9.9f;
        row.AGE = index;
        row.PASS = true;
        chblix_t block = tab_insert(table, schema,&row);
        if (chblix_cmp(&block, &CHBLIX_FAIL) == 0) {
            logger(LL_ERROR, __func__, "Failed to insert row ");
            return;
        }
    }
}

void delete_rows(db_t* db, table_t* table, schema_t* schema, field_t* field, int64_t start_index, int64_t number_of_rows) {
    for (int64_t index = start_index; index < start_index + number_of_rows; ++index) {
        int64_t value = index;
        int res = tab_delete_op(db, table, schema, field, COND_EQ, &value);

        if (res == TABLE_FAIL) {
            logger(LL_ERROR, __func__, "Failed to delete row ");
            return;
        }
    }
}

int main(){
    db_t* db = db_init(TEST_DB);
    if(pg_file_size() > 0){
        db_drop();
        db = db_init(TEST_DB);
    }
    FILE* file = fopen(CSV_FILE, "w+");
    fprintf(file, "%s", CSV_HEADER);
    /* Create table */

    schema_t* schema = sch_init();
    sch_add_int_field(schema, "ID");
    sch_add_char_field(schema, "NAME", 10);
    sch_add_float_field(schema, "SCORE");
    sch_add_int_field(schema, "AGE");
    sch_add_bool_field(schema, "PASS");
    table_t* table = tab_init(db, "STUDENT", schema);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            int AGE;
            bool PASS;
    );
    strncpy(row.NAME, "STUDENT", 10);
    row.SCORE = 9.9f;
    row.AGE = 20;
    row.PASS = true;
    uint64_t max_file_size =  1024u * 1024u * 1; // 11 gb

    int64_t rows_inserted = 0;
    int64_t next_insert_start = 0;

    field_t field;
    sch_get_field(schema, "ID", &field);

    while(pg_file_size() < max_file_size){
        insert_rows(table, schema, next_insert_start, ALLOCATION);
        delete_rows(db, table, schema, &field, next_insert_start, DEALLOCATION);
        rows_inserted = rows_inserted + ALLOCATION - DEALLOCATION;
        next_insert_start += ALLOCATION;
        fprintf(file, "%llu;%llu\n", pg_file_size(), rows_inserted);
        fflush(file);
        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), rows_inserted);
    }
    fclose(file);
    db_drop();
}
