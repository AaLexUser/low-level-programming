#include "schema.h"

/**
 * @brief       Initialize a schema
 * @return      index of the schema on success, SCHEMA_FAIL on failure
 */

int64_t sch_init(){
    int64_t schidx = lb_ppl_init(sizeof(field_t) - sizeof(linked_block_t));
    schema_t* sch = (schema_t*)lb_ppl_load(schidx);
    if(sch == NULL) {
        logger(LL_ERROR, __func__, "Failed to load schema %ld", schidx);
        return SCHEMA_FAIL;
    }
    sch->slot_size = 0;
    return schidx;
}

/**
 * @brief       Add a field
 * @param[in]   schidx: index of the schema
 * @param[in]   name: name of the field
 * @param[in]   type: type of the field
 * @param[in]   size: size of the type
 * @return      SCHEMA_SUCCESS on success, SCHEMA_FAIL on failure
 */

int sch_add_field(int64_t schidx, const char* name, datatype_t type, size_t size){
    schema_t* sch = sch_load(schidx);
    if(sch == NULL) {
        logger(LL_ERROR, __func__, "Failed to load schema %ld", schidx);
        return SCHEMA_FAIL;
    }

    chblix_t fieldix = lb_alloc_m(schidx, sizeof(field_t));
    if(chblix_cmp(&fieldix, &CHBLIX_FAIL) == 0){
        logger(LL_ERROR, __func__, "Failed to allocate field %s", name);
        return SCHEMA_FAIL;
    }
    field_t field;
    if(sch_field_load(schidx, &fieldix, &field) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to load field %s", name);
        return SCHEMA_FAIL;
    }
    strncpy(field.name, name, MAX_NAME_LENGTH);
    field.type = type;
    field.size = size;
    field.offset = sch->slot_size;
    sch->slot_size += size;
    if(sch_field_update(schidx, &fieldix, &field) == LB_FAIL){
        logger(LL_ERROR, __func__, "Failed to update field %s", name);
        return SCHEMA_FAIL;
    }
    return SCHEMA_SUCCESS;
}

/**
 * @brief       Get a field
 * @param[in]   schidx: index of the schema
 * @param[in]   name: name of the field
 * @param[out]  field: pointer to destination field
 * @return
 */

int sch_get_field(int64_t schidx, const char* name, field_t* field){
    schema_t* sch = sch_load(schidx);
    if(sch == NULL) {
        logger(LL_ERROR, __func__, "Failed to load schema %ld", schidx);
        return SCHEMA_FAIL;
    }

    lb_for_each(chblix, (page_pool_t*)sch){
        if(sch_field_load(schidx, &chblix, field) == LB_FAIL){
            logger(LL_ERROR, __func__, "Failed to read field %s", name);
            return SCHEMA_FAIL;
        }
        if(strcmp(field->name, name) == 0){
            return SCHEMA_SUCCESS;
        }
    }
    logger(LL_ERROR, __func__, "Failed to find field %s", name);
    return SCHEMA_NOT_FOUND;
}

/**
 * @brief       Delete a field
 * @warning     This function delete field, but dont touch offsets in other fields in schema.
 * @param[in]   schidx: index of the schema
 * @param[in]   name: name of the field
 * @return      SCHEMA_SUCCESS on success, SCHEMA_FAIL on failure
 */

int sch_delete_field(int64_t schidx, const char* name){
    schema_t* sch = sch_load(schidx);
    if(sch == NULL) {
        logger(LL_ERROR, __func__, "Failed to load schema %ld", schidx);
        return SCHEMA_FAIL;
    }

    sch_for_each(sch, field, schidx){
        if(strcmp(field.name, name) == 0){
            if(lb_dealloc(schidx, &field.lb_header.chblix) == LB_FAIL){
                logger(LL_ERROR, __func__, "Failed to deallocate field %s", name);
                return SCHEMA_FAIL;
            }
            return SCHEMA_SUCCESS;
        }
    }

    logger(LL_ERROR, __func__, "Failed to find field %s", name);
    return SCHEMA_FAIL;
}

