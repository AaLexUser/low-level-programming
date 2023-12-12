#include "../src/bench.h"
#include "backend/table/table.h"
#include <sys/time.h>
#include <time.h>

const char* TEST_DB = "test.db";
const char* CSV_FILE = "table-insert.csv";
const char* CSV_HEADER= "Time;Rows\n";
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
        int res = tab_delete_op_nova(db, table, schema, field, COND_EQ, &value);
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
    field_t field;
    sch_get_field(schema, "ID", &field);

    time_t test_start = time(NULL);
    time_t test_end = time(NULL);
    int64_t rows_inserted = 0;
    int64_t next_insert_start = 0;

    int64_t* times = malloc(sizeof(int64_t) * ALLOCATION * 100);
    int64_t avg = 0;
    int64_t times_sum = 0;
    int64_t times_count = 0;

    struct timespec start, end;

    while(test_end - test_start < TEST_TIME) {
        clock_gettime(CLOCK_UPTIME_RAW, &start);
        insert_rows(table, schema, next_insert_start, ALLOCATION);
        clock_gettime(CLOCK_UPTIME_RAW, &end);
        int64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        delta_us /= ALLOCATION;
        times_count++;
        times_sum += delta_us;
        avg = times_sum / times_count;
        delete_rows(db, table, schema, &field, next_insert_start, DEALLOCATION);
        rows_inserted = rows_inserted + ALLOCATION - DEALLOCATION;
        next_insert_start += ALLOCATION;
        fprintf(file, "%"PRIu64";%llu\n", delta_us, rows_inserted);
        fflush(file);
        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), rows_inserted);
        test_end = time(NULL);
    }
    printf("Test time: %jd\n", test_end - test_start);
    printf("Avg insertion time: %"PRId64"\n", avg);
    fclose(file);
    db_drop();
}