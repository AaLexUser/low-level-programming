#include "../src/bench.h"
#include "backend/table/table.h"
#include "utils/logger.h"
#include <sys/time.h>
#if __linux__
#include <bits/types/struct_timespec.h>
#include <stdio.h>
#include <malloc.h>
#include <time.h>

#define _posix_c_source 200809

#endif


int main(){
    int64_t allocation = 100;
    int64_t deallocation = 50;
    int64_t stage_num = 20;
    int64_t test_time = 10*60; //10 min

    db_t* db = db_init("test.db");
    FILE* file = fopen("table-delete.csv", "w+");
    fprintf(file, "Time;Allocated\n");
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
    int64_t allocated = 0;
    int64_t next_id = 0;
    while(test_end - test_start < test_time) {
        for (int64_t j = next_id; j < next_id + allocation; ++j) {
            row.ID = j;
            row.SCORE = 9.9f;
            row.AGE = j;
            row.PASS = true;
            chblix_t block = tab_insert(tablix, &row);
//            printf("id: %lld\n", j);
            if (chblix_cmp(&block, &CHBLIX_FAIL) == 0) {
                logger(LL_ERROR, __func__, "Failed to insert row ");
                return TABLE_FAIL;
            }
        }
        printf("Blocks allocated before delete: %lld\n", allocated + allocation);
        time_t start = time(NULL);
        for (int64_t j = next_id; j < (next_id + deallocation); ++j) {
            int64_t value = j;
            int res = tab_delete_op_nova(db, table, schema, &field, COND_EQ, &value);
            if (res == TABLE_FAIL) {
                logger(LL_ERROR, __func__, "Failed to delete row ");
                return TABLE_FAIL;
            }
        }
        next_id += allocation;
        allocated = allocated + allocation - deallocation;
        printf("Blocks allocated after delete: %lld\n", allocated);
        time_t end = time(NULL);
        double delta_us = (double)(end - start) / (double)deallocation;
        fprintf(file, "%f;%llu\n", delta_us, allocated);
        fflush(file);
//        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), deallocation);
        i++;
        test_end = time(NULL);
    }
    printf("Test time: %ld\n", test_end - test_start);
    fclose(file);
    db_drop();
}