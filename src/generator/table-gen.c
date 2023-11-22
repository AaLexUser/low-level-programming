#include "table-gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * @brief       Generate empty table
 * @param[in]   mtabidx: meta table index
 * @param[in]   gentabMgr_ptr: pointer to generate table manager
 * @return      return tablix on success, TABLE_FAIL otherwise
 */

int64_t gen_empty_table(int64_t mtabidx, gentab_mgr* gentabMgr_ptr){
    srand(time(NULL));
    int64_t schidx = sch_init();
    int64_t number_of_fields = 1 + rand()%(80);
    for(int64_t i = 0; i < number_of_fields; ++i){
        char* field_name = malloc(7 + i%10);
        char str[100];
        sprintf(str, "FIELD_%llu", i);
        strncpy(field_name, str, (7 + i%10));
        int64_t field_size = 1 + rand()%(100);
        datatype_t type = rand()%(5);
        switch (type) {
            case VARCHAR:
                sch_add_varchar_field(schidx, field_name);
                break;
            case CHAR:
                sch_add_char_field(schidx, field_name, field_size);
                break;
            case INT:
                sch_add_int_field(schidx, field_name);
                break;
            case FLOAT:
                sch_add_float_field(schidx, field_name);
                break;
            case BOOL:
                sch_add_bool_field(schidx, field_name);
                break;
            default:
                break;
        }
        free(field_name);
    }
    char* table_name = malloc(5 + gentabMgr_ptr->next_index % 10);
    char str[128];
    sprintf(str, "GEN_%llu", gentabMgr_ptr->next_index);
    strncpy(table_name, str, (5 + gentabMgr_ptr->next_index % 10));
    ++gentabMgr_ptr;
    int res = tab_init(mtabidx, table_name, schidx);
    free(table_name);
    return res;
}

/**
 * @brief       Generate random element
 * @param[in]   field: pointer to field
 * @return      pointer to element on success, NULL otherwise
 */

void* gen_fill_element(field_t* field){
    void* element = malloc(field->size);
    switch (field->type) {
        case INT: {
            int64_t rand_num = INT16_MIN + rand()%(INT16_MAX - INT16_MIN + 1);
            memcpy(element, &rand_num, sizeof(int64_t));
            break;
        }
        case FLOAT: {
            float rand_float =((float)rand() / (float)RAND_MAX) * (INT16_MAX - INT16_MIN) + INT16_MIN;
            memcpy(element, &rand_float, sizeof(float));
            break;
        }
        case BOOL: {
            int r = rand();
            bool rand_bool = r % 2 == 0;
            memcpy(element, &rand_bool, sizeof(bool));
            break;
        }
        case CHAR: {
            char* rand_char = malloc(field->size);
            for(int64_t i = 0; i < field->size; ++i){
                rand_char[i] = 'a' + rand() % 26;
            }
            memcpy(element, rand_char, field->size);
            free(rand_char);
            break;
        }
        case VARCHAR: {
            size_t size = 1 + rand()%(257);
            char* rand_varchar = malloc(size);
            for(int64_t i = 0; i < size; ++i){
                rand_varchar[i] = 'a' + rand() % 26;
            }
            vch_ticket_t ticket = vch_add(rand_varchar);
            if(chblix_cmp(&ticket.block, &CHBLIX_FAIL) == 0){
                logger(LL_ERROR, __func__, "Failed to add varchar");
                return NULL;
            }

            memcpy(element, &ticket, sizeof(vch_ticket_t));
            free(rand_varchar);
            break;
        }
        default:
            return NULL;
    }
    return element;
}

/**
 * @brief       Generate random row
 * @param       schema: pointer to schema
 * @return      pointer to row on success, NULL otherwise
 */

void* gen_fill_row(schema_t* schema){
    void* row = malloc(schema->slot_size);
    int64_t schidx = schema->ppl_header.lp_header.page_index;
    sch_for_each(schema, field, chblix, schidx){
        void* element = gen_fill_element(&field);
        if(element == NULL){
            logger(LL_ERROR, __func__, "Failed to generate element");
            return NULL;
        }
        memcpy(row + field.offset, element, field.size);
        free(element);
    }
    return row;
}

/**
 * @brief       Generate random rows
 * @param[in]   tablix: index of the table
 * @param[in]   number: number of rows to generate
 * @return      TABLE_SUCCESS on success, TABLE_FAIL otherwise
 */

int gen_fill_rows(int64_t tablix, int64_t number){
    table_t* table = tab_load(tablix);
    schema_t* schema = sch_load(table->schidx);
    for(int64_t i = 0; i < number; ++i){
        void* row = gen_fill_row(schema);
        if(row == NULL){
            logger(LL_ERROR, __func__, "Failed to generate row");
            return TABLE_FAIL;
        }
        chblix_t res = tab_insert(tablix, row);
        if(chblix_cmp(&res, &CHBLIX_FAIL) == 0){
            logger(LL_ERROR, __func__, "Failed to insert row");
            return TABLE_FAIL;
        }
        free(row);
    }
    return TABLE_SUCCESS;
}

int64_t gen_table(int64_t mtabidx, gentab_mgr* gentabMgr_ptr, int64_t min_number_of_rows){
    int64_t tablix = gen_empty_table(mtabidx, gentabMgr_ptr);
    if(tablix == TABLE_FAIL){
        logger(LL_ERROR, __func__, "Failed to generate empty table");
        return TABLE_FAIL;
    }
    int64_t number_of_rows = min_number_of_rows + rand()%(min_number_of_rows * 10 - min_number_of_rows + 1);
    if(gen_fill_rows(tablix, number_of_rows) == TABLE_FAIL){
        logger(LL_ERROR, __func__, "Failed to generate rows");
        return TABLE_FAIL;
    }
    return tablix;
}

