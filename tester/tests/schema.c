#include "../src/test.h"
#include "backend/io/pager.h"
#include "backend/table/schema.h"

DEFINE_TEST(create_add_foreach_sch){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    int64_t schidx = sch_init();
    sch_add_char_field(schidx, "NAME", 10);
    sch_add_int_field(schidx, "CREDIT");
    sch_add_float_field(schidx, "DEBIT");
    sch_add_bool_field(schidx, "STUDENT");
    schema_t* sch = sch_load(schidx);
    sch_for_each(sch, field, schidx){
        printf("%s\n",field.name);
    }
    pg_delete();
}

DEFINE_TEST(delete_field){
    assert(pg_init("test.db") == PAGER_SUCCESS);
    int64_t schidx = sch_init();
    sch_add_char_field(schidx, "NAME", 10);
    sch_add_int_field(schidx, "CREDIT");
    sch_add_float_field(schidx, "DEBIT");
    sch_add_bool_field(schidx, "STUDENT");
    sch_delete_field(schidx, "STUDENT");
    field_t temp;
    int res = sch_get_field(schidx, "STUDENT", &temp);
    assert(res == SCHEMA_NOT_FOUND);
    res = sch_get_field(schidx, "DEBIT", &temp);
    assert(res == SCHEMA_SUCCESS);
    pg_delete();
}





int main(){
    RUN_SINGLE_TEST(create_add_foreach_sch);
    RUN_SINGLE_TEST(delete_field);
}