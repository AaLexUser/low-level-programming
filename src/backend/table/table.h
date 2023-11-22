#pragma once
#include "backend/comporator/comporator.h"
#include "backend/journal/metatab.h"
#include "table_base.h"


int64_t tab_init(int64_t mtabidx, const char* name, int64_t schidx);
chblix_t tab_get_row(int64_t tablix, field_t* field, void* value, datatype_t type);
void tab_print(int64_t tablix);
int64_t tab_join(
        int64_t mtabidx,
        int64_t leftidx,
        int64_t rightidx,
        const char* join_field_left,
        const char* join_field_right,
        const char* name);

int64_t tab_select_op(int64_t mtabidx,
                      int64_t sel_tabidx,
                      const char* name,
                      const char* select_field,
                      condition_t condition,
                      void* value,
                      datatype_t type);
int tab_drop(int64_t mtabidx, int64_t tablix);
int tab_update_row_op(int64_t tablix,
                  void* row,
                  const char* field_name,
                  condition_t condition,
                  void* value,
                  datatype_t type);
int tab_update_element_op(int64_t tablix,
                          void* element,
                          const char* field_name,
                          const char* field_comp,
                          condition_t condition,
                          void* value,
                          datatype_t type);
int tab_delete_op(int64_t tablix,
                  const char* field_comp,
                  condition_t condition,
                  void* value);
