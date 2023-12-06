#include "../src/bench.h"
#include "backend/table/table.h"
#include <sys/time.h>
#include <time.h>

int main(){

    db_t* db = db_init("test.db");
    FILE* file = fopen("table-delete.csv", "w+");
    fprintf(file, "Time;Rows\n");

    /* Create table */
    int64_t mtabidx = mtab_init();
    int64_t schidx = sch_init();
    sch_add_char_field(schidx, "NAME", 10);
    sch_add_float_field(schidx, "SCORE");
    sch_add_int_field(schidx, "AGE");
    sch_add_bool_field(schidx, "PASS");
    int64_t tablix = tab_init(db, "STUDENT", schidx);
    tab_row(
            char NAME[10];
            float SCORE;
            int AGE;
            bool PASS;
            );
    strncpy(row.NAME, "STUDENT", 10);
    row.SCORE = 9.9f;
    row.AGE = 20;
    row.PASS = true;
    int64_t iteration = 1000;
    for (int64_t i = 0; i < iteration; ++i) {
        time_t start = time(NULL);
        chblix_t res = tab_insert(tablix, &row);
        time_t end = time(NULL);
        if(chblix_cmp(&res, &CHBLIX_FAIL) == 0){
            logger(LL_ERROR, __func__, "Failed to insert row ");
            return TABLE_FAIL;
        }
        uint64_t delta_us = end - start;
        fprintf(file, "%llu;%llu\n", delta_us, i);
        fflush(file);
        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), i);
    }
    db_close(db);
    pg_delete();
}