#include "table.h"
#include "utils/logger.h"

/**
 * @brief       Initialize a table
 * @param[in]   name: name of the table
 * @param[in]   schidx: index of the schema
 * @return      index of the table on success, TABLE_FAIL on failure
 */

int64_t tab_init(const char* name, int64_t schidx){
    schema_t* schema = sch_load(schidx);
    int64_t tablix = lb_ppl_init((int64_t)schema->slot_size);
    if(tablix == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to initialize table %s", name);
        return TABLE_FAIL;
    }
    table_t* table = (table_t*)lb_ppl_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %s", name);
        return TABLE_FAIL;
    }

    table->schidx = schidx;
    strncpy(table->name, name, MAX_NAME_LENGTH);
    return tablix;
}

/**
 * @brief       Insert a row
 * @param       tablix: index of the table
 * @param       src: source
 * @return      chblix_t of row on success, CHBLIX_FAIL on failure
 */

chblix_t tab_insert(int64_t tablix, void* src){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return CHBLIX_FAIL;
    }

    chblix_t rowix = lb_alloc(tablix);

    if(chblix_cmp(&rowix, &CHBLIX_FAIL) == 0){
        logger(LL_ERROR, __func__, "Failed to allocate row");
        return CHBLIX_FAIL;
    }

    if(lb_write(tablix, &rowix, src, table->ppl_header.block_size, 0) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to write row");
        return CHBLIX_FAIL;
    }

    return rowix;

}

/**
 * @brief       Select a row
 * @param[in]   tablix: index of the table
 * @param[in]   rowix: chblix of the row
 * @param[out]  dest: destination
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_select(int64_t tablix, chblix_t* rowix, void* dest){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    if(lb_read(tablix, rowix, dest, table->ppl_header.block_size, 0) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to read row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}

/**
 * @brief       Delete a row
 * @param       tablix: index of the table
 * @param       rowix: chblix of the row
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_delete(int64_t tablix, chblix_t* rowix){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    if(lb_dealloc(tablix, rowix) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to deallocate row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}

/**
 * @brief       Update a row
 * @param[in]   tablix: index of the table
 * @param[in]   rowix: chblix of the row
 * @param[in]   data: data to be written
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_update(int64_t tablix, chblix_t* rowix, void* data){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    if(lb_write(tablix, rowix, data, table->ppl_header.block_size, 0) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to write row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}

chblix_t tab_find_in_column(int64_t tablix, const char* column_name, void* value){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return CHBLIX_FAIL;
    }

    field_t field;
    sch_get_field(table->schidx, column_name, &field);
    lb_for_each(chblix, &table->ppl_header){
        void* row = malloc(table->ppl_header.block_size);
        if(lb_read(tablix, &chblix, row, table->ppl_header.block_size, 0) == LB_FAIL){
            logger(LL_ERROR, __func__, "Failed to read row");
            free(row);
            return CHBLIX_FAIL;
        }

        if(memcmp(row + field.offset, value, field.size) == 0){
            free(row);
            return chblix;
        }
        free(row);
    }
//    void* element = malloc(table->ppl_header.block_size);
//    for (chblix_t chblix = lb_pool_start(&table->ppl_header);
//         chblix_cmp(&chblix, &CHBLIX_FAIL) != 0 &&
//         lb_read(tablix, &chblix, element, (int64_t)field.size, (int64_t)field.offset) != LB_FAIL;
//         ++chblix.block_idx, chblix = lb_nearest_valid_chblix(&table->ppl_header, chblix))
    return CHBLIX_FAIL;
}
