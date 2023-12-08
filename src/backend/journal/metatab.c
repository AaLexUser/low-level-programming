#include "metatab.h"
#include "backend/table/schema.h"
#include "utils/logger.h"

/**
 * @brief       Initialize the metatable
 * @return      index of the metatable on success, TABLE_FAIL on failure
 */

int64_t mtab_init(void){
    int64_t schidx = sch_init();
    sch_add_char_field(schidx, "NAME", MAX_NAME_LENGTH);
    sch_add_int_field(schidx, "INDEX");
    int64_t tablix = tab_base_init("METATABLE", schidx);
    tab_row(
            char NAME[MAX_NAME_LENGTH];
            int64_t INDEX;
            );
    strncpy(row.NAME, "METATABLE", MAX_NAME_LENGTH);
    row.INDEX = tablix;
    chblix_t res = tab_insert(tablix, &row);
    if(chblix_cmp(&res, &CHBLIX_FAIL) == 0){
        logger(LL_ERROR, __func__, "Failed to insert row ");
        return TABLE_FAIL;
    }
    return tablix;
}

/**
 * @brief       Find table index by name
 * @param       mtabidx: index of the metatable
 * @param       name: name of the table
 * @return      index of the table on success, TABLE_FAIL on failure
 */

int64_t mtab_find_tab(int64_t mtabidx, const char* name){
    tab_row(
            char NAME[MAX_NAME_LENGTH];
            int64_t INDEX;
            );
    table_t* table = tab_load(mtabidx);
    schema_t* schema = sch_load(table->schidx);
    tab_for_each_row(table, chunk, chblix, &row, schema){
        if(strcmp(name,row.NAME) == 0){
            return row.INDEX;
        }
    }
    return TABLE_FAIL;
}

/**
 * @brief       Add a table to the metatable
 * @param[in]   mtabidx: index of the metatable
 * @param[in]   name: name of the table
 * @param[in]   index: index of the table
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int mtab_add(int64_t mtabidx, const char* name, int64_t index){
    tab_row(
            char NAME[MAX_NAME_LENGTH];
            int64_t INDEX;
    );
    strncpy(row.NAME, name, MAX_NAME_LENGTH);
    row.INDEX = index;
    chblix_t res = tab_insert(mtabidx, &row);
    if(chblix_cmp(&res, &CHBLIX_FAIL) == 0){
        logger(LL_ERROR, __func__, "Failed to insert row ");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}

/**
 * @brief       Delete a table from the metatable
 * @param[in]   mtabidx: index of the metatable
 * @param[in]   index: index of the table
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int mtab_delete(int64_t mtabidx, int64_t index){
    tab_row(
            char NAME[MAX_NAME_LENGTH];
            int64_t INDEX;
    );
    table_t* table = tab_load(mtabidx);
    schema_t* schema = sch_load(table->schidx);
    tab_for_each_row(table, chunk, rowix, &row, schema) {
        if(row.INDEX == index){
            if (tab_delete(mtabidx, &rowix) == TABLE_FAIL) {
                logger(LL_ERROR, __func__, "Failed to delete row ");
                return TABLE_FAIL;
            }
        }
    }
    return TABLE_SUCCESS;
}
