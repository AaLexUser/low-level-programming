#include <stdio.h>
#include "table.h"

/**
 * @brief       Initialize table and add it to the metatable
 * @param[in]   mtabidx: index of the metatable
 * @param[in]   name: name of the table
 * @param[in]   schidx: index of the schema
 * @return      index of the table on success, TABLE_FAIL on failure
 */

int64_t tab_init(int64_t mtabidx, const char* name, int64_t schidx){
    int64_t tablix = tab_base_init(name, schidx);
    mtab_add(mtabidx, name, tablix);
    return tablix;
}

/**
 * @brief       Get row by value in column
 * @param       tablix: index of the table
 * @param       field: pointer to the field
 * @param       value: pointer to the value
 * @param       type: type of the value
 * @return      chblix_t of row on success, CHBLIX_FAIL on failure
 */

chblix_t tab_get_row(int64_t tablix, field_t* field, void* value, datatype_t type){
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
    tab_for_each_element(table, tablix, chblix, element, field){
        if(comp_eq(type, element, value)){
            free(element);
            return chblix;
        }
    }
    free(element);
    return CHBLIX_FAIL;
}

/**
 * @brief       Print table
 * @param[in]   tablix: index of the table
 */

void tab_print(int64_t tablix){
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
    tab_for_each_row(table, tablix, chblix,  row, schema){
        sch_for_each(schema, field, sch_chblix, table->schidx){
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
                    vch_get(vch, str);
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
    }
    fflush(stdout);
}

/**
 * @brief       Inner join two tables
 * @param[in]   mtabidx: index of the metatable
 * @param[in]   leftidx: index of the left table
 * @param[in]   rightidx: index of the right table
 * @param[in]   join_field_left: join field of the left table
 * @param[in]   join_field_right: join field of the right table
 * @param[in]   name: name of the new table
 * @return      index of the new table on success, TABLE_FAIL on failure
 */

int64_t tab_join(
        int64_t mtabidx,
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
    sch_for_each(left_schema, left_field, left_chblix, left->schidx){
        if(sch_add_field(schidx, left_field.name, left_field.type, left_field.size) == SCHEMA_FAIL){
            logger(LL_ERROR, __func__, "Failed to add field %s", left_field.name);
            return TABLE_FAIL;
        }
    }
    sch_for_each(right_schema, right_field, right_chblix, right->schidx){
        if(sch_add_field(schidx, right_field.name, right_field.type, right_field.size) == SCHEMA_FAIL){
            logger(LL_ERROR, __func__, "Failed to add field %s", right_field.name);
            return TABLE_FAIL;
        }
    }

    /* Create new table */
    int64_t tablix = tab_init(mtabidx, name, schidx);
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
    tab_for_each_row(left, leftidx, leftt_chblix, left_row, left_schema){
        memcpy(elleft, (char*)left_row + join_field_left_f.offset, join_field_left_f.size);
        tab_for_each_row(right, rightidx, rightt_chblix, right_row, right_schema){
            memcpy(elright, (char*)right_row + join_field_right_f.offset, join_field_right_f.size);
            if(comp_eq(join_field_left_f.type, elleft, elright)){
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