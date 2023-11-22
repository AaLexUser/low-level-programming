#include "../src/bench.h"
#include "backend/table/table.h"
#include "utils/logger.h"
#include <sys/time.h>

int main(){
    struct timespec start, end;
    int64_t allocation = 500;
    int64_t deallocation = 300;
    int64_t stage_num = 10;

    init_db("test.db");
    int fd = out_file("table-delete.csv");
    FILE *file = fdopen(fd, "w");
    fprintf(file, "Time;Stage\n");
    /* Create table */
    int64_t mtabidx = mtab_init();
    int64_t schidx = sch_init();
    sch_add_int_field(schidx, "ID");
    sch_add_char_field(schidx, "NAME", 10);
    sch_add_float_field(schidx, "SCORE");
    sch_add_int_field(schidx, "AGE");
    sch_add_bool_field(schidx, "PASS");
    int64_t tablix = tab_init(mtabidx, "STUDENT", schidx);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            int AGE;
            bool PASS;
    );
    for(int64_t i = 0; i < stage_num; i++) {
        for(int j = 0; j < allocation; ++j){
            row.ID = j;
            row.SCORE = 9.9f;
            row.AGE = j;
            row.PASS = true;
            chblix_t block = tab_insert(tablix, &row);
            if (chblix_cmp(&block, &CHBLIX_FAIL) == 0) {
                logger(LL_ERROR, __func__, "Failed to insert row ");
                return TABLE_FAIL;
            }
        }
        clock_gettime(CLOCK_UPTIME_RAW, &start);
        for(int j = 0; j < deallocation; ++j){
            int64_t value = j;
            if(j == 41){
                printf("here");
            }
            int res = tab_delete_op(tablix, "ID", COND_EQ, &value);
            if (res == TABLE_FAIL) {
                logger(LL_ERROR, __func__, "Failed to delete row ");
                return TABLE_FAIL;
            }
        }
        clock_gettime(CLOCK_UPTIME_RAW, &end);
        uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        delta_us /= deallocation;
        fprintf(file, "%llu;%llu\n", delta_us, deallocation);
        fflush(file);
        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), deallocation);
    }
    close(fd);
    pg_delete();
}