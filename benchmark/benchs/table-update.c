#include "../src/bench.h"
#include "backend/table/table.h"
#include "utils/logger.h"
#include <sys/time.h>

int main(){
    struct timespec start, end;

    init_db("test.db");
    int fd = out_file("table-update.csv");
    FILE *file = fdopen(fd, "w");
    fprintf(file, "Time;Rows\n");
    /* Create table */
    int64_t mtabidx = mtab_init();
    int64_t schidx = sch_init();
    sch_add_int_field(schidx, "ID");
    sch_add_char_field(schidx, "NAME", 10);
    sch_add_float_field(schidx, "SCORE");
    sch_add_int_field(schidx, "AGE");
    sch_add_bool_field(schidx, "PASS");
    int64_t tablix = tab_init(mtabidx, "STUDENT", schidx);
    uint64_t max_file_size =  (uint64_t)100*1024*1024; // 100 mb
    uint64_t allocate_count = 500;
    uint64_t deallocate_count = 400;
    uint64_t block_count = 0;
    uint64_t interations = 1000;
    field_t field;
    for(allocate_count = 1000; allocate_count < 10000; allocate_count+= 1000) {
        if (sch_get_field(schidx, "AGE", &field) == SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to get field ");
            exit(EXIT_FAILURE);
        }
        for (int64_t i = 0; i < interations; i++) {
            tab_row(
                    int64_t ID;
                    char NAME[10];
                    float SCORE;
                    int64_t AGE;
                    bool PASS;
            );
            strncpy(row.NAME, "STUDENT", 10);
            row.ID = i;
            row.SCORE = 9.9f;
            row.AGE = i;
            row.PASS = true;
            chblix_t block = tab_insert(tablix, &row);
            block_count++;
            if (chblix_cmp(&block, &CHBLIX_FAIL) == 0) {
                logger(LL_ERROR, __func__, "Failed to insert row ");
                return TABLE_FAIL;
            }
        }
        field_t field_el;
        if (sch_get_field(schidx, "AGE", &field_el) == SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to get field ");
            exit(EXIT_FAILURE);
        }
        field_t field_upd;
        if (sch_get_field(schidx, "ID", &field_upd) == SCHEMA_FAIL) {
            logger(LL_ERROR, __func__, "Failed to get field ");
            exit(EXIT_FAILURE);
        }
        clock_gettime(CLOCK_UPTIME_RAW, &start);
        for (int64_t i = 0; i < interations; i++) {
            int64_t age = interations - i;
            int res = tab_update_element_op(tablix, &age, "AGE", "ID", COND_EQ, &i, INT);
            if (res == TABLE_FAIL) {
                logger(LL_ERROR, __func__, "Failed to update row ");
                return TABLE_FAIL;
            }
        }
        clock_gettime(CLOCK_UPTIME_RAW, &end);
        uint64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
        delta_us /= interations;
        fprintf(file, "%llu;%llu\n", delta_us, allocate_count);
        fflush(file);
        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), allocate_count);
    }
    close(fd);
    pg_delete();
}