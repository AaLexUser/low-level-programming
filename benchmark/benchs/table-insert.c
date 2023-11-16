#include "../src/bench.h"
#include "backend/table/table.h"
#include <sys/time.h>

int main(){
    struct timespec start, end;

    init_db("test.db");
    int fd = out_file("table-insert.csv");
    FILE *file = fdopen(fd, "w");
    fprintf(file, "Time;Rows\n");

    /* Create table */
    int64_t mtabidx = mtab_init();
    int64_t schidx = sch_init();
    sch_add_char_field(schidx, "NAME", 10);
    sch_add_float_field(schidx, "SCORE");
    sch_add_int_field(schidx, "AGE");
    sch_add_bool_field(schidx, "PASS");
    int64_t tablix = tab_init(mtabidx, "STUDENT", schidx);
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
    for (int64_t i = 0; i < 1000000; ++i) {
        clock_gettime(CLOCK_UPTIME_RAW, &start);
        chblix_t res = tab_insert(tablix, &row);
        clock_gettime(CLOCK_UPTIME_RAW, &end);
        if(chblix_cmp(&res, &CHBLIX_FAIL) == 0){
            logger(LL_ERROR, __func__, "Failed to insert row ");
            return TABLE_FAIL;
        }
        uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        fprintf(file, "%llu;%llu\n", delta_us, i);
        fflush(file);
        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), i);
    }
    close(fd);
    pg_delete();
}