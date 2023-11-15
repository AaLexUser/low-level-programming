#pragma once
#include "../data_type.h"
#include "../page_pool/page_pool.h"
#include "backend/page_pool/linked_blocks.h"
#include <stdint.h>
#include <string.h>

#define MAX_NAME_LENGTH 32
typedef struct field{
    linked_block_t lb_header;
    char name[MAX_NAME_LENGTH];
    DATA_TYPE type;
    uint64_t size;
    uint64_t offset;
} field_t;

typedef struct schema{
    page_pool_t ppl_header;
    int64_t slot_size;
} schema_t;

typedef enum {SCHEMA_SUCCESS = 0, SCHEMA_FAIL = -1, SCHEMA_NOT_FOUND = -2} schema_status_t;

/**
 * @brief       Load a schema
 * @param[in]   schidx: index of the schema
 * @return      pointer to the schema on success, NULL on failure
 */

#define sch_load(schidx) ((schema_t*)lb_ppl_load(schidx))

/**
 * @brief       Delete a schema
 * @param[in]   schidx: index of the schema
 * @return      SCHEMA_SUCCESS on success, SCHEMA_FAIL on failure
 */

#define sch_delete(schidx) (lb_ppl_destroy(schidx)))

/**
 * @brief      Load field
 * @param[in]  schidx: index of the schema
 * @param[in]  fieldix: index of the field
 * @param[out] field: pointer to the field
 * @return     LB_SUCCESS on success, LB_FAIL on failure
 */

#define sch_field_load(schidx, fieldix, field) (lb_load((schidx), (fieldix), (linked_block_t*)(field)))

/**
 * @brief      Update field
 * @param[in]  schidx: index of the schema
 * @param[in]  fieldix: index of the field
 * @param[out] field: pointer to the field
 * @return     LB_SUCCESS on success, LB_FAIL on failure
 */

#define sch_field_update(schidx, fieldix, field) (lb_update((schidx), (fieldix), (linked_block_t*)(field)))



#define sch_add_int_field(schidx, name) sch_add_field(schidx, name, INT, sizeof(int64_t))
#define sch_add_char_field(schidx, name, size) sch_add_field(schidx, name, CHAR, size)
#define sch_add_varchar_field(schidx, name) sch_add_field(schidx, name, VARCHAR, sizeof(chblix_t))
#define sch_add_float_field(schidx, name) sch_add_field(schidx, name, FLOAT, sizeof(float))
#define sch_add_bool_field(schidx, name) sch_add_field(schidx, name, BOOL, sizeof(bool))


#define sch_for_each(sch, field) \
    field_t field;\
    chblix_t sch_chblix = lb_pool_start((page_pool_t*)sch);\
    sch_field_load(schidx, &sch_chblix, &field);\
    for(sch_chblix;\
    lb_valid((page_pool_t*)sch, sch_chblix) &&\
    sch_field_load(schidx, &sch_chblix, &field) != LB_FAIL; \
    ++sch_chblix.block_idx,  sch_chblix = lb_nearest_valid_chblix((page_pool_t*)sch, sch_chblix))

int64_t sch_init();
int sch_add_field(int64_t schidx, const char* name, DATA_TYPE type, size_t size);
int sch_get_field(int64_t schidx, const char* name, field_t* field);
int sch_delete_field(int64_t schidx, const char* name);
