#include "../src/bench.h"
#include "backend/table/table.h"
#include "utils/logger.h"
#include <sys/time.h>
#include <stdlib.h>

int fill_table(int64_t tablix){
    for(int i = 0; i < 1000000; i++){
        tab_row(
                char NAME[10];
                float SCORE;
                int AGE;
                bool PASS;
        );
        strncpy(row.NAME, "STUDENT", 10);
        row.SCORE = 9.9f;
        row.AGE = i;
        row.PASS = true;
        chblix_t rowix = tab_insert(tablix, &row);
        if(chblix_cmp(&rowix, &CHBLIX_FAIL) == 0){
            logger(LL_ERROR, __func__, "Failed to insert row ");
            return TABLE_FAIL;
        }
    }
    return TABLE_SUCCESS;
}

int main(){
    struct timespec start, end;

    init_db("test.db");
    int fd = out_file("table-update.csv");
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
    if(fill_table(tablix) == TABLE_FAIL){
        logger(LL_ERROR, __func__, "Failed to fill table ");
        return TABLE_FAIL;
    }
    uint64_t max_file_size =  (uint64_t)100*1024*1024; // 100 mb
    field_t field;
    if(sch_get_field(schidx, "AGE", &field) == SCHEMA_FAIL){
        logger(LL_ERROR, __func__, "Failed to get field ");
        exit(EXIT_FAILURE);
    }
    while(pg_file_size() < max_file_size){
        tab_select_op(mtabidx, tablix, "SELECT", )

        chblix_t blocks[allocate_count];
        for(int64_t i = 0; i < allocate_count; i++){
            chblix_t block = tab_insert(tablix, &row);
            blocks[i] = block;
            block_count++;
            if(chblix_cmp(&block, &CHBLIX_FAIL) == 0){
                logger(LL_ERROR, __func__, "Failed to insert row ");
                return TABLE_FAIL;
            }
        }
        for(int64_t i = 0; i < deallocate_count; i++){
            int64_t element = block_count;
            clock_gettime(CLOCK_UPTIME_RAW, &start);
            int res = tab_update_element(tablix, &blocks[i], &field, &element);
            clock_gettime(CLOCK_UPTIME_RAW, &end);
            if(res) {
                logger(LL_ERROR, __func__, "Failed to delete row ");
                return TABLE_FAIL;
            }
            block_count--;
        }
        uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        fprintf(file, "%llu;%llu\n", delta_us, block_count);
        fflush(file);
        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), block_count);
    }
    close(fd);
    pg_delete();
}