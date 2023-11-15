#include "../src/test.h"
#include "backend/io/pager.h"
#include "backend/table/schema.h"
#include "backend/table/table.h"

DEFINE_TEST(create_add_foreach){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    int64_t schidx = sch_init();
    const size_t char_count = 10;
    sch_add_char_field(schidx, "NAME", char_count);
    sch_add_char_field(schidx, "SURNAME", char_count);
    sch_add_int_field(schidx, "CREDIT");
    sch_add_float_field(schidx, "DEBIT");
    sch_add_bool_field(schidx, "STUDENT");
    int64_t tablix = tab_init("test", schidx);
    tab_row(
            char NAME[char_count];
            char SURNAME[char_count];
            int64_t CREDIT;
            float DEBIT;
            bool STUDENT;
    );
    strncpy(row.NAME, "Alex", char_count);
    strncpy(row.SURNAME, "Smith", char_count);
    row.CREDIT = 10;
    row.DEBIT = 10.5f;
    row.STUDENT = true;
    chblix_t res = tab_insert(tablix, &row);
    strncpy(row.NAME, "Nick", char_count);
    strncpy(row.SURNAME, "Johnson", char_count);
    row.CREDIT = 20;
    row.DEBIT = 20.5f;
    row.STUDENT = false;
    res = tab_insert(tablix, &row);
    strncpy(row.NAME, "John", char_count);
    strncpy(row.SURNAME, "Doe", char_count);
    row.CREDIT = 30;
    row.DEBIT = 30.5f;
    row.STUDENT = true;
    res = tab_insert(tablix, &row);
    table_t* table  = tab_load(tablix);
    field_t field;
    sch_get_field(schidx, "CREDIT", &field);
    int64_t* element = malloc(field.size);
    int64_t ans[3];
    int counter = 0;
    tab_for_each_element(table,element, field){
        ans[counter++] = *element;
    }
    assert(ans[0] == 10);
    assert(ans[1] == 20);
    assert(ans[2] == 30);
    pg_delete();
}

int main(){
    RUN_SINGLE_TEST(create_add_foreach);
}