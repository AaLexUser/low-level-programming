#include "../src/bench.h"
#include "backend/table/table.h"
#include "utils/logger.h"
#include <sys/time.h>
#include <inttypes.h>
#if __linux__
#include <stdio.h>
#include <time.h>

#endif

const char* TEST_DB = "test.db";
const char* CSV_FILE = "table-delete.csv";
const char* CSV_HEADER= "Time;Allocated\n";
const int TEST_TIME = 30; //10 min
const int ALLOCATION = 100;
const int DEALLOCATION = 50;

void insert_rows(int64_t tablix, int64_t start_index, int64_t number_of_rows) {
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
        chblix_t block = tab_insert(tablix, &row);
//            printf("id: %lld\n", j);
        if (chblix_cmp(&block, &CHBLIX_FAIL) == 0) {
            logger(LL_ERROR, __func__, "Failed to insert row ");
            return;
        }
    }
}

void delete_rows(db_t* db, table_t* table, schema_t* schema, field_t* field, int64_t start_index, int64_t number_of_rows) {
    for (int64_t index = start_index; index < start_index + number_of_rows; ++index) {
        int64_t value = index;
        if(value == 724){
            db_drop();
            exit(EXIT_SUCCESS);
        }
        int res = tab_delete_op_nova(db, table, schema, field, COND_EQ, &value);
        if (res == TABLE_FAIL) {
            logger(LL_ERROR, __func__, "Failed to delete row ");
            return;
        }
    }
}


int main(){
    db_t* db = db_init(TEST_DB);
    sleep(30);
    FILE* file = fopen(CSV_FILE, "w+");
    fprintf(file, "%s", CSV_HEADER);
    /* Create table */
    int64_t schidx = sch_init();
    schema_t* schema = sch_load(schidx);
    sch_add_int_field(schidx, "ID");
    sch_add_char_field(schidx, "NAME", 10);
    sch_add_float_field(schidx, "SCORE");
    sch_add_int_field(schidx, "AGE");
    sch_add_bool_field(schidx, "PASS");
    int64_t tablix = tab_init(db, "STUDENT", schidx);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            int64_t AGE;
            bool PASS;
    );
    table_t* table = tab_load(tablix);
    field_t field;
    if(sch_get_field(schidx, "ID", &field) == SCHEMA_FAIL){
        return TABLE_FAIL;
    }
    time_t test_start = time(NULL);
    time_t test_end = time(NULL);
    int64_t i = 0;
    int64_t rows_inserted = 0;
    int64_t next_insert_start = 0;
    while(test_end - test_start < TEST_TIME) {
        insert_rows(tablix, next_insert_start, ALLOCATION);
        printf("Blocks allocated before delete: %"PRId64"\n", rows_inserted + ALLOCATION);
        time_t start = time(NULL);
        delete_rows(db, table, schema, &field, next_insert_start, DEALLOCATION);
        time_t end = time(NULL);
        double delta_us = (double)(end - start) / (double)DEALLOCATION;
        next_insert_start += ALLOCATION;
        rows_inserted = rows_inserted + ALLOCATION - DEALLOCATION;
        printf("Blocks allocated after delete: %"PRId64"\n", rows_inserted);
        fprintf(file, "%f;%"PRId64"\n", delta_us, rows_inserted);
        fflush(file);
//        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), deallocation);
        i++;
        test_end = time(NULL);
    }
    printf("Test time: %jd\n", test_end - test_start);
    fclose(file);
    db_drop();
}