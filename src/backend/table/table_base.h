#pragma once

#include "backend/page_pool/page_pool.h"
#include "schema.h"

typedef struct table {
    page_pool_t ppl_header;
    int64_t schidx; //schema index
    char name[MAX_NAME_LENGTH];
    int64_t mtable_idx;
    int64_t varchar_mgr_idx;
} table_t;

typedef enum {TABLE_SUCCESS = 0, TABLE_FAIL = -1} table_status_t;



/**
 * @brief       Load a table
 * @param[in]   tablix: index of the table
 * @return      pointer to the table on success, NULL on failure
 */

#define tab_load(tablix) (table_t*)lb_ppl_load(tablix)

/**
 * @brief       For each element specific column in a table
 * @param[in]   table: pointer to the table
 * @param[in]   chunk: chunk
 * @param[in]   chblix: chblix of the row
 * @param[in]   element: pointer to the element, must be allocated before calling this macro
 * @param[in]   field: pointer to field of the element
 */

#define tab_for_each_element(table, chunk, chblix, element, field) \
chunk_t* chunk = ppl_load_chunk(table->ppl_header.head);                     \
chblix_t chblix = lb_pool_start(&table->ppl_header, chunk);\
lb_read_nova_5(&table->ppl_header, &chblix, element, (int64_t)(field)->size, (int64_t)(field)->offset);\
for (chblix;\
lb_valid(&table->ppl_header,chunk, chblix) &&\
lb_read_nova_5(&table->ppl_header, &chblix, element, (int64_t)(field)->size, (int64_t)(field)->offset) != LB_FAIL;\
++chblix.block_idx, chblix = lb_nearest_valid_chblix(&table->ppl_header, chblix, &chunk))

/**
 * @brief       For each element specific column in a table
 * @param[in]   table: pointer to the table
 * @param[in]   chunk: chunk
 * @param[in]   chblix: chblix of the row
 * @param[in]   row: pointer to the row, must be allocated before calling this macro
 * @param[in]   schema: pointer to the schema
 */

#define tab_for_each_row(table, chunk, chblix, row, schema) \
chunk_t* chunk = ppl_load_chunk(table->ppl_header.head);   \
chblix_t chblix = lb_pool_start(&table->ppl_header, chunk);\
lb_read_nova_5(&table->ppl_header, &chblix, row, schema->slot_size, 0);\
for (chblix;                                         \
lb_valid(&table->ppl_header, chunk, chblix) &&\
lb_read_nova_5(&table->ppl_header, &chblix, row, schema->slot_size, 0) != LB_FAIL; \
++chblix.block_idx, chblix = lb_nearest_valid_chblix(&table->ppl_header,\
                                                                      chblix, &chunk))

#define tab_row(...) \
    typedef struct __attribute__((packed)){ \
        __VA_ARGS__ \
    } row_t;         \
row_t row

int64_t tab_base_init(const char* name, int64_t schidx);
chblix_t tab_insert(int64_t tablix, void* src);
int tab_select_row(int64_t tablix, chblix_t* rowix, void* dest);
int tab_delete_nova(table_t* table, chunk_t* chunk, chblix_t* rowix);
int tab_delete(int64_t tablix, chblix_t* rowix);
int tab_update_row(int64_t tablix, chblix_t* rowix, void* row);

int tab_update_element(int64_t tablix, chblix_t* rowix, field_t* field, void* element);
int tab_get_element(int64_t tablix, chblix_t* rowix, field_t* field, void* element);
