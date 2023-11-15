#include "comporator.h"

/**
 * @brief       Compare two values
 * @param       type: type of the values
 * @param       val1: pointer to the first value
 * @param       val2: pointer to the second value
 * @return      data_t: the result of the comparison
 */

data_t comp_cmp(datatype_t type, void* val1, void* val2){
    data_t data;
    switch (type) {
        case INT: {
            data.int_val = (*(int64_t *) val1 - *(int64_t *) val2);
            break;
        }
        case FLOAT: {
            data.float_val = *(float *) val1 - *(float *) val2;
            break;
        }
        case CHAR: {
            data.int_val = strcmp((char*)val1, (char*)val2);
            break;
        }
        case BOOL: {
            data.int_val =  *(bool *) val1 - *(bool *) val2;
            break;
        }
        case VARCHAR: {
            vch_ticket_t* vch1 = val1;
            vch_ticket_t* vch2 = val2;
            char* str1 = malloc(vch1->size);
            vch_get(vch1, str1);
            char* str2 = malloc(vch2->size);
            vch_get(vch2, str2);
            int res = strcmp(str1, str2);
            free(str1);
            free(str2);
            data.int_val = res;
            break;
        }
        case DATA_TYPE_UNKNOWN:
            data.int_val = 0;
            break;
    }
    return data;
}

/**
 * @brief       Compare two values on equality
 * @param       type: type of the values
 * @param       val1: pointer to the first value
 * @param       val2: pointer to the second value
 * @return      1 if equal, 0 if not
 */

bool comp_eq(datatype_t type, void* val1, void* val2){
    data_t data = comp_cmp(type, val1, val2);
    switch (type) {
        case INT: {
            return data.int_val == 0;
        }
        case FLOAT: {
            return data.float_val == 0;
        }
        case CHAR : {
            return data.int_val == 0;
        }
        case BOOL: {
            return data.int_val == 0;
        }
        case VARCHAR: {
            return data.int_val == 0;
        }
        case DATA_TYPE_UNKNOWN:
            return 0;
    }
}