#include <stdio.h>
#include "table.h"

/**
 * @brief       Initialize table and add it to the metatable
 * @param[in]   db: pointer to db
 * @param[in]   name: name of the table
 * @param[in]   schidx: index of the schema
 * @return      index of the table on success, TABLE_FAIL on failure
 */

int64_t tab_init(db_t* db, const char* name, int64_t schidx){
    int64_t tablix = tab_base_init(name, schidx);
    mtab_add(db->meta_table_idx, name, tablix);
    return tablix;
}

/**
 * @brief       Get row by value in column
 * @param       db: pointer to db
 * @param       tablix: index of the table
 * @param       field: pointer to the field
 * @param       value: pointer to the value
 * @param       type: type of the value
 * @return      chblix_t of row on success, CHBLIX_FAIL on failure
 */

chblix_t tab_get_row(db_t* db, int64_t tablix, field_t* field, void* value, datatype_t type){
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
    void* element = malloc(field->size);
    tab_for_each_element(table, chunk, chblix, element, field){
        if(comp_eq(db, type, element, value)){
            free(element);
            return chblix;
        }
    }
    free(element);
    return CHBLIX_FAIL;
}

/**
 * @brief       Print table
 * @param[in]   db: pointer to db
 * @param[in]   tablix: index of the table
 */

void tab_print(db_t* db, int64_t tablix){
    table_t* table = tab_load(tablix);
    if(table == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return;
    }

    schema_t* schema = sch_load(table->schidx);
    if(schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", table->schidx);
        return;
    }
    void* row = malloc(schema->slot_size);
    tab_for_each_row(table, chunk, chblix,  row, schema){
        sch_for_each(schema,chunk2, field, sch_chblix, table->schidx){
            switch(field.type){
                case INT: {
                    printf("%lld\t", *(int64_t*)((char*)row + field.offset));
                    break;
                }
                case FLOAT: {
                    printf("%f\t", *(float *) ((char *) row + field.offset));
                    break;
                }
                case CHAR: {
                    printf("%s\t", (char*)((char*)row + field.offset));
                    break;
                }
                case BOOL: {
                    printf("%d\t", *(bool*)((char*)row + field.offset));
                    break;
                }
                case VARCHAR: {
                    vch_ticket_t* vch = (vch_ticket_t*)((char*)row + field.offset);
                    char* str = malloc(vch->size);
                    vch_get(db->varchar_mgr_idx, vch, str);
                    printf("%s\t", str);
                    free(str);
                    break;
                }

                default:
                    logger(LL_ERROR, __func__, "Unknown type %d", field.type);
                    break;
            }
        }
        printf("\n");
        fflush(stdout);
    }
    fflush(stdout);
}

/**
 * @brief       Inner join two tables
 * @param[in]   db: pointer to db
 * @param[in]   leftidx: index of the left table
 * @param[in]   rightidx: index of the right table
 * @param[in]   join_field_left: join field of the left table
 * @param[in]   join_field_right: join field of the right table
 * @param[in]   name: name of the new table
 * @return      index of the new table on success, TABLE_FAIL on failure
 */

int64_t tab_join(
        db_t* db,
        int64_t leftidx,
        int64_t rightidx,
        const char* join_field_left,
        const char* join_field_right,
        const char* name){

    /* Load tables */
    table_t* left = tab_load(leftidx);
    if(left == NULL){
        logger(LL_ERROR, __func__, "Failed to load right table %ld", leftidx);
        return TABLE_FAIL;
    }
    table_t* right = tab_load(rightidx);
    if(right == NULL){
        logger(LL_ERROR, __func__, "Failed to load right table %ld", rightidx);
        return TABLE_FAIL;
    }

    /* Load schemas */
    schema_t* left_schema = sch_load(left->schidx);
    if(left_schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load left schema %ld", left->schidx);
        return TABLE_FAIL;
    }
    schema_t* right_schema = sch_load(right->schidx);
    if(right_schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load right schema %ld", right->schidx);
        return TABLE_FAIL;
    }

    /* Create new schema */
    int64_t schidx = sch_init();
    if(schidx == SCHEMA_FAIL){
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return TABLE_FAIL;
    }
    sch_for_each(left_schema, chunk, left_field, left_chblix, left->schidx){
        if(sch_add_field(schidx, left_field.name, left_field.type, left_field.size) == SCHEMA_FAIL){
            logger(LL_ERROR, __func__, "Failed to add field %s", left_field.name);
            return TABLE_FAIL;
        }
    }
    sch_for_each(right_schema,chunk2, right_field, right_chblix, right->schidx){
        if(sch_add_field(schidx, right_field.name, right_field.type, right_field.size) == SCHEMA_FAIL){
            logger(LL_ERROR, __func__, "Failed to add field %s", right_field.name);
            return TABLE_FAIL;
        }
    }

    /* Create new table */
    int64_t tablix = tab_init(db, name, schidx);
    if(tablix == TABLE_FAIL){
        logger(LL_ERROR, __func__, "Failed to create new table");
        return TABLE_FAIL;
    }

    /* Load new schema */
    schema_t* schema = sch_load(schidx);

    /* Create new row */
    void* row = malloc(schema->slot_size);

    void* left_row = malloc(left_schema->slot_size);
    void* right_row = malloc(right_schema->slot_size);

    /* Get join fields */
    field_t join_field_left_f;
    sch_get_field(left->schidx, join_field_left, &join_field_left_f);
    field_t join_field_right_f;
    sch_get_field(right->schidx, join_field_right, &join_field_right_f);



    /* Join */
    void* elleft = malloc(join_field_left_f.size);
    void* elright = malloc(join_field_right_f.size);
    tab_for_each_row(left, left_chunk, leftt_chblix, left_row, left_schema){
        memcpy(elleft, (char*)left_row + join_field_left_f.offset, join_field_left_f.size);
        tab_for_each_row(right, right_chunk,rightt_chblix, right_row, right_schema){
            memcpy(elright, (char*)right_row + join_field_right_f.offset, join_field_right_f.size);
            if(comp_eq(db, join_field_left_f.type, elleft, elright)){
                memcpy(row, left_row, left_schema->slot_size);
                memcpy((char*)row + left_schema->slot_size, right_row, right_schema->slot_size);
                chblix_t rowix = tab_insert(tablix, row);
                if(chblix_cmp(&rowix, &CHBLIX_FAIL) == 0){
                    logger(LL_ERROR, __func__, "Failed to insert row");
                    return TABLE_FAIL;
                }
            }
        }
    }
    free(elleft);
    free(elright);
    free(row);
    free(left_row);
    free(right_row);
    return tablix;
}

/**
 * @brief       Select row form table on condition
 * @param[in]   db: pointer to db
 * @param[in]   sel_tabidx: index of table from which the selection is made
 * @param[in]   name: name of new table that will be created
 * @param[in]   select_field: the field by which the selection is performed
 * @param[in]   condition: comparison condition
 * @param[in]   value: value to compare with
 * @param[in]   type: the type of value to compare with
 * @return
 */

int64_t tab_select_op(db_t* db,
                      int64_t sel_tabidx,
                      const char* name,
                      const char* select_field,
                      condition_t condition,
                      void* value,
                      datatype_t type){

   /* Load table */
    table_t* sel_tab = tab_load(sel_tabidx);
    if(sel_tab == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", sel_tabidx);
        return TABLE_FAIL;
    }

    /* Load schema */
    schema_t* sel_schema = sch_load(sel_tab->schidx);
    if(sel_schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", sel_tab->schidx);
        return TABLE_FAIL;
    }

    /* Load field */
    field_t select_field_f;
    if(sch_get_field(sel_tab->schidx, select_field, &select_field_f) == SCHEMA_FAIL){
        logger(LL_ERROR, __func__, "Failed to get field %s", select_field);
        return TABLE_FAIL;
    }

    /* Create new schema */
    int64_t schidx = sch_init();
    if(schidx == SCHEMA_FAIL){
        logger(LL_ERROR, __func__, "Failed to create new schema");
        return TABLE_FAIL;
    }
    sch_for_each(sel_schema, sch_chunk, field, chblix, sel_tab->schidx){
        if(sch_add_field(schidx, field.name, field.type, field.size) == SCHEMA_FAIL){
            logger(LL_ERROR, __func__, "Failed to add field %s", field.name);
            return TABLE_FAIL;
        }
    }

    /* Create new table */
    int64_t tablix = tab_init(db, name, schidx);
    if(tablix == TABLE_FAIL){
        logger(LL_ERROR, __func__, "Failed to create new table");
        return TABLE_FAIL;
    }

    /* Load new schema */
    schema_t* schema = sch_load(schidx);

    /* Create new row */
    void* row = malloc(schema->slot_size);

    /* Check if datatype of field equals datatype of value */
    if(type != select_field_f.type){
        return TABLE_FAIL;
    }

    void* el_row = malloc(sel_schema->slot_size);
    void* el = malloc(select_field_f.size);
    void* comp_val = malloc(select_field_f.size);
    memcpy(comp_val, value, select_field_f.size);

    /* Select */
    tab_for_each_row(sel_tab, tab_chunk, sel_chblix, el_row, sel_schema){
        memcpy(el, (char*)el_row + select_field_f.offset, select_field_f.size);
        if(comp_compare(db, type, el,comp_val, condition)){
            memcpy(row, el_row, schema->slot_size);
            chblix_t rowix = tab_insert(tablix, row);
            if(chblix_cmp(&rowix, &CHBLIX_FAIL) == 0){
                logger(LL_ERROR, __func__, "Failed to insert row");
                return TABLE_FAIL;
            }
        }
    }
    free(comp_val);
    free(row);
    free(el_row);
    free(el);
    return tablix;
}

/**
 * @brief       Drop a table
 * @param[in]   db: pointer to db
 * @param[in]   tablix: index of the table
 * @return      PPL_SUCCESS on success, PPL_FAIL on failure
 */

int tab_drop(db_t* db, int64_t tablix){
    if (mtab_delete(db->meta_table_idx, tablix) == TABLE_FAIL) {
        logger(LL_ERROR, __func__, "Failed to delete table %ld", tablix);
        return PPL_FAIL;
    }
    return lb_ppl_destroy(tablix);
}

/**
 * @brief       Delete row from table
 * @param[in]   db: pointer to db
 * @param[in]   tablix: index of the table
 * @param[in]   row: row to write
 * @param[in]   field_name: name of the field compare with
 * @param[in]   condition: comparison condition
 * @param[in]   value: value to compare with
 * @param[in]   type: the type of value to compare with
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_update_row_op(db_t* db,
                    int64_t tablix,
                    void* row,
                    const char* field_name,
                    condition_t condition,
                    void* value,
                    datatype_t type){

    /* Load table */
    table_t* upd_tab = tab_load(tablix);
    if(upd_tab == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    /* Load schema */
    schema_t* upd_schema = sch_load(upd_tab->schidx);
    if(upd_schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", upd_tab->schidx);
        return TABLE_FAIL;
    }

    /* Load field */
    field_t upd_field;
    if(sch_get_field(upd_tab->schidx, field_name, &upd_field) == SCHEMA_FAIL){
        logger(LL_ERROR, __func__, "Failed to get field %s", field_name);
        return TABLE_FAIL;
    }

    /* Check if datatype of field equals datatype of value */
    if(type != upd_field.type){
        return TABLE_FAIL;
    }

    void* el_row = malloc(upd_schema->slot_size);
    void* el = malloc(upd_field.size);
    void* comp_val = malloc(upd_field.size);
    memcpy(comp_val, value, upd_field.size);

    /* Update */
    tab_for_each_row(upd_tab, upd_chunk,upd_chblix, el_row, upd_schema){
        memcpy(el, (char*)el_row + upd_field.offset, upd_field.size);
        if(comp_compare(db, type, el, comp_val, condition)){
            memcpy(el_row, row, upd_schema->slot_size);
            if(tab_update_row(tablix, &upd_chblix, el_row) == TABLE_FAIL){
                logger(LL_ERROR, __func__, "Failed to update row");
                return TABLE_FAIL;
            }
        }
    }
    free(comp_val);
    free(el_row);
    free(el);
    return TABLE_SUCCESS;
}

/**
 * @brief       Update element in table
 * @param[in]   db: pointer to db
 * @param[in]   tablix: index of the table
 * @param[in]   element: element to write
 * @param[in]   field_name: name of the element field
 * @param[in]   field_comp: name of the field compare with
 * @param[in]   condition: comparison condition
 * @param[in]   value: value to compare with
 * @param[in]   type: the type of value to compare with
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_update_element_op(db_t* db,
                            int64_t tablix,
                            void* element,
                            const char* field_name,
                            const char* field_comp,
                            condition_t condition,
                            void* value,
                            datatype_t type) {
    /* Load table */
    table_t* upd_tab = tab_load(tablix);
    if(upd_tab == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    /* Load schema */
    schema_t* upd_schema = sch_load(upd_tab->schidx);
    if(upd_schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", upd_tab->schidx);
        return TABLE_FAIL;
    }

    /* Load compare field */
    field_t comp_field;
    if(sch_get_field(upd_tab->schidx, field_comp, &comp_field) == SCHEMA_FAIL){
        logger(LL_ERROR, __func__, "Failed to get field %s", field_comp);
        return TABLE_FAIL;
    }

    /* Load update field */
    field_t upd_field;
    if(sch_get_field(upd_tab->schidx, field_name, &upd_field) == SCHEMA_FAIL){
        logger(LL_ERROR, __func__, "Failed to get field %s", field_comp);
        return TABLE_FAIL;
    }

    /* Check if datatype of field equals datatype of value */
    if(type != comp_field.type){
        return TABLE_FAIL;
    }

    void* el_row = malloc(upd_schema->slot_size);
    void* el = malloc(comp_field.size);
    void* upd_el = malloc(upd_field.size);
    void* comp_val = malloc(comp_field.size);
    memcpy(comp_val, value, comp_field.size);

    /* Update */
    tab_for_each_row(upd_tab, upd_chunk,upd_chblix, el_row, upd_schema){
        memcpy(el, (char*)el_row + comp_field.offset, comp_field.size);
        if(comp_compare(db, type, el, comp_val, condition)){
            memcpy(upd_el, element, upd_field.size);
            if(tab_update_element(tablix, &upd_chblix, &upd_field, upd_el) == TABLE_FAIL){
                logger(LL_ERROR, __func__, "Failed to update row");
                return TABLE_FAIL;
            }
        }
    }

    free(upd_el);
    free(comp_val);
    free(el_row);
    free(el);
    return TABLE_SUCCESS;
}

/**
 * @brief       Delete row from table
 * @param[in]   db: pointer to db
 * @param[in]   table: pointer to table
 * @param[in]   schema: pointer to schema
 * @param[in]   field_comp: pointer to field to compare
 * @param[in]   condition: comparison condition
 * @param[in]   value: value to compare with
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_delete_op_nova(db_t* db,
                   table_t* table,
                   schema_t* schema,
                   field_t* field_comp,
                   condition_t condition,
                   void* value){

    void* el_row = malloc(schema->slot_size);
    void* el = malloc(field_comp->size);
    void* comp_val = malloc(field_comp->size);
    memcpy(comp_val, value, field_comp->size);
    int64_t counter = 0;

    /* Delete */
    tab_for_each_row(table, del_chunk, del_chblix, el_row, schema){
        counter++;
        if(counter == 44){
            printf("here");
        }
        memcpy(el, (char*)el_row + field_comp->offset, field_comp->size);
        if(comp_compare(db, field_comp->type, el, comp_val, condition)){
            chblix_t temp = del_chblix;
            bool flag = false;
            if(del_chunk->num_of_free_blocks + 1 == del_chunk->capacity){
                int64_t next_chunk = del_chunk->next_page;
                temp = (chblix_t){.block_idx = -1, .chunk_idx=next_chunk};
                flag = true;
            }
            if(tab_delete_nova(table, del_chunk, &del_chblix) == TABLE_FAIL){
                logger(LL_ERROR, __func__, "Failed to delete row");
                return TABLE_FAIL;
            }
            if(flag){
                del_chblix = temp;
                del_chunk = ppl_load_chunk(del_chblix.chunk_idx);
            }
        }
    }
    free(comp_val);
    free(el_row);
    free(el);
    int64_t* val =value;
    printf("value %llu, counter %llu\n", *val, counter);
    fflush(stdout);


    return TABLE_SUCCESS;

}


/**
 * @brief       Delete row from table
 * @param[in]   db: pointer to db
 * @param[in]   tablix: index of the table
 * @param[in]   field_comp: name of the field compare with
 * @param[in]   condition: comparison condition
 * @param[in]   value: value to compare with
 * @return      TABLE_SUCCESS on success, TABLE_FAIL on failure
 */

int tab_delete_op(db_t* db,
                int64_t tablix,
                const char* field_comp,
                condition_t condition,
                void* value){
    /* Load table */
    table_t* del_tab = tab_load(tablix);
    if(del_tab == NULL){
        logger(LL_ERROR, __func__, "Failed to load table %ld", tablix);
        return TABLE_FAIL;
    }

    /* Load schema */
    schema_t* del_schema = sch_load(del_tab->schidx);
    if(del_schema == NULL){
        logger(LL_ERROR, __func__, "Failed to load schema %ld", del_tab->schidx);
        return TABLE_FAIL;
    }

    /* Load compare field */
    field_t comp_field;
    if(sch_get_field(del_tab->schidx, field_comp, &comp_field) == SCHEMA_FAIL){
        logger(LL_ERROR, __func__, "Failed to get field %s", field_comp);
        return TABLE_FAIL;
    }
    return tab_delete_op_nova(db, del_tab,del_schema, &comp_field, condition, value );
}

