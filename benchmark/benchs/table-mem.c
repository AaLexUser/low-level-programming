#include "../src/bench.h"
#include "backend/table/table.h"
#include "utils/logger.h"
#ifdef LOGGER_LEVEL
#undef LOGGER_LEVEL
#endif
#define LOGGER_LEVEL 2

int main(){
    init_db("test.db");
    int fd = out_file("table-mem.csv");
    FILE *file = fdopen(fd, "w");
    fprintf(file, "FileSize;Rows\n");
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
    uint64_t max_file_size =  (uint64_t)1024 * 1024 * 1024 * 11; // 11 gb
    uint64_t allocate_count = 500;
    uint64_t deallocate_count = 400;
    uint64_t block_count = 0;
    while(pg_file_size() < max_file_size){
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
            if(tab_delete(tablix, &blocks[i]) != TABLE_SUCCESS){
                logger(LL_ERROR, __func__, "Failed to delete row ");
                return TABLE_FAIL;
            }
            block_count--;
        }
        fprintf(file, "%llu;%llu\n", pg_file_size(), block_count);
        fflush(file);
        logger(LL_WARN, __func__, "File size: %llu, blocks count: %llu", pg_file_size(), block_count);
    }
    close(fd);
    pg_delete();
}
