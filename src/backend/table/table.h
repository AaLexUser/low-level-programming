#pragma once
#include "backend/comporator/comporator.h"
#include "backend/journal/metatab.h"
#include "table_base.h"
#include "backend/db/db.h"


int64_t tab_init(db_t* db, const char* name, int64_t schidx);
chblix_t tab_get_row(db_t* db, int64_t tablix, field_t* field, void* value, datatype_t type);
void tab_print(db_t* db, int64_t tablix);
int64_t tab_join(
        db_t* db,
        int64_t leftidx,
        int64_t rightidx,
        const char* join_field_left,
        const char* join_field_right,
        const char* name);

int64_t tab_select_op(db_t* db,
                      int64_t sel_tabidx,
                      const char* name,
                      const char* select_field,
                      condition_t condition,
                      void* value,
                      datatype_t type);
int tab_drop(db_t* db, int64_t tablix);
int tab_update_row_op(db_t* db,
                      int64_t tablix,
                      void* row,
                      const char* field_name,
                      condition_t condition,
                      void* value,
                      datatype_t type);
int tab_update_element_op(db_t* db,
                          int64_t tablix,
                          void* element,
                          const char* field_name,
                          const char* field_comp,
                          condition_t condition,
                          void* value,
                          datatype_t type);
int tab_delete_op_nova(db_t* db,
                   table_t* table,
                   schema_t* schema,
                   field_t* comp,
                   condition_t condition,
                   void* value);
int tab_delete_op(db_t* db,
                  int64_t tablix,
                  const char* field_comp,
                  condition_t condition,
                  void* value);
