#include "table_base.h"
#include "utils/logger.h"

/**
 * @brief       Initialize a table
 * @param[in]   name: name of the table
 * @param[in]   schidx: index of the schema
 * @return      index of the table on success, TABLE_FAIL on failure
 */

int64_t tab_base_init(const char* name, int64_t schidx){
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
    schema_t* schema = sch_load(table->schidx);
    if(schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", table->schidx);
        return CHBLIX_FAIL;
    }

    chblix_t rowix = lb_alloc(tablix);

    if(chblix_cmp(&rowix, &CHBLIX_FAIL) == 0){
        logger(LL_ERROR, __func__, "Failed to allocate row");
        return CHBLIX_FAIL;
    }

    if(lb_write(tablix, &rowix, src,schema->slot_size , 0) == LB_FAIL){
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

int tab_select_row(int64_t tablix, chblix_t* rowix, void* dest){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    schema_t* schema = sch_load(table->schidx);
    if(schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", table->schidx);
        return TABLE_FAIL;
    }

    if(lb_read(tablix, rowix, dest, schema->slot_size, 0) == LB_FAIL){
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
 * @param[in]   row: row to be written
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_update_row(int64_t tablix, chblix_t* rowix, void* row){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    schema_t* schema = sch_load(table->schidx);
    if(schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", table->schidx);
        return TABLE_FAIL;
    }

    if(lb_write(tablix, rowix, row, schema->slot_size, 0) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to write row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}

/**
 * @brief       Update an element
 * @param[in]   tablix: index of the table
 * @param[in]   rowix: chblix of the row
 * @param[in]   field: pointer to the field
 * @param[in]   element: pointer to the element to be written
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_update_element(int64_t tablix, chblix_t* rowix, field_t* field, void* element){
    if(lb_write(tablix, rowix, element, (int64_t)field->size, (int64_t)field->offset) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to write row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}


/**
 * @brief       Get an element from row
 * @param[in]   tablix: index of the table
 * @param[in]   rowix: chblix of the row
 * @param[in]   field: pointer to the field
 * @param[out]  element: pointer to the element to be written
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_get_element(int64_t tablix, chblix_t* rowix, field_t* field, void* element){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    schema_t* schema = sch_load(table->schidx);
    if(schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", table->schidx);
        return TABLE_FAIL;
    }

    if(lb_read(tablix, rowix, element, (int64_t)field->size, (int64_t)field->offset) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to read row");
        return TABLE_FAIL;
    }
    return TABLE_SUCCESS;
}





