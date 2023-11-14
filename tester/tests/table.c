#include "../src/test.h"
#include "backend/io/pager.h"
#include "backend/table/schema.h"
#include "backend/table/table.h"

DEFINE_TEST(create_add_foreach){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    int64_t schidx = sch_init();
    sch_add_char_field(schidx, "NAME", 10);
    sch_add_int_field(schidx, "CREDIT");
    sch_add_float_field(schidx, "DEBIT");
    sch_add_bool_field(schidx, "STUDENT");
    int64_t tablix = tab_init("test", schidx);
    tab_row(
            char* NAME;
            int64_t CREDIT;
            float DEBIT;
            bool STUDENT;
    );
    row.NAME = "Alex";
    row.CREDIT = 10;
    row.DEBIT = 10.5f;
    row.STUDENT = true;
    chblix_t res = tab_insert(tablix, &row);
    row.NAME = "Nick";
    row.CREDIT = 20;
    row.DEBIT = 20.5f;
    row.STUDENT = false;
    res = tab_insert(tablix, &row);
    row.NAME = "John";
    row.CREDIT = 30;
    row.DEBIT = 30.5f;
    row.STUDENT = true;
    res = tab_insert(tablix, &row);
    table_t* table  = tab_load(tablix);
    char* element = malloc(sizeof(row)* 10);
    field_t field;
    sch_get_field(schidx, "NAME", &field);
    tab_for_each_element(table,element, field){
        printf("%s\n", element);
    }
    pg_delete();
}

int main(){
    RUN_SINGLE_TEST(create_add_foreach);
}