#include "../src/test.h"
#include "core/io/pager.h"
#include "backend/table/schema.h"
#include "backend/table/table.h"
#ifdef LOGGER_LEVEL
#undef LOGGER_LEVEL
#endif
#define LOGGER_LEVEL 2

void insert_data(int64_t tablix, int64_t schidx, int64_t count){
    tab_row(
            char NAME[10];
            char SURNAME[10];
            int64_t CREDIT;
            float DEBIT;
            bool STUDENT;
    );
    for(int i = 0; i < count; i++){
        strncpy(row.NAME, "Alex", 10);
        strncpy(row.SURNAME, "Smith", 10);
        row.CREDIT = 10;
        row.DEBIT = 10.5f;
        row.STUDENT = true;
        chblix_t res = tab_insert(tablix, &row);
        strncpy(row.NAME, "Nick", 10);
        strncpy(row.SURNAME, "Johnson", 10);
        row.CREDIT = 20;
        row.DEBIT = 20.5f;
        row.STUDENT = false;
        res = tab_insert(tablix, &row);
        strncpy(row.NAME, "John", 10);
        strncpy(row.SURNAME, "Doe", 10);
        row.CREDIT = 30;
        row.DEBIT = 30.5f;
        row.STUDENT = true;
        res = tab_insert(tablix, &row);
    }
}

int64_t init_schema(){
    int64_t schidx = sch_init();
    const size_t char_count = 10;
    sch_add_char_field(schidx, "NAME", char_count);
    sch_add_char_field(schidx, "SURNAME", char_count);
    sch_add_int_field(schidx, "CREDIT");
    sch_add_float_field(schidx, "DEBIT");
    sch_add_bool_field(schidx, "STUDENT");
    return schidx;
}

int64_t table_bank(db_t* db, int count){
    int64_t schidx = init_schema();
    int64_t tablix = tab_init(db, "BANK", schidx);
    insert_data(tablix, schidx, count);
    return tablix;
}

int64_t table_student(db_t* db, int count){
    int64_t schidx = sch_init();
    sch_add_int_field(schidx, "ID");
    sch_add_char_field(schidx, "NAME", 10);
    sch_add_float_field(schidx, "SCORE");
    sch_add_bool_field(schidx, "PASS");
    int64_t tablix = tab_init(db, "STUDENTS", schidx);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            bool PASS;
    );
    row.ID = 1;
    strncpy(row.NAME,"John", 10);
    row.SCORE = 10.5f;
    row.PASS = true;
    chblix_t res = tab_insert(tablix, &row);

    row.ID = 2;
    strncpy(row.NAME,"Nick", 10);
    row.SCORE = 20.5f;
    row.PASS = false;
    res = tab_insert(tablix, &row);

    row.ID = 3;
    strncpy(row.NAME,"Alex", 10);
    row.SCORE = 30.5f;
    row.PASS = true;
    res = tab_insert(tablix, &row);

    row.ID = 4;
    strncpy(row.NAME,"Smith", 10);
    row.SCORE = 40.5f;
    row.PASS = false;
    res = tab_insert(tablix, &row);

    return tablix;
}


DEFINE_TEST(create_add_foreach){
    db_t* db = db_init("test.db");
    int64_t schidx = init_schema();
    int64_t tablix = tab_init(db, "test", schidx);
    insert_data(tablix, schidx, 1);
    table_t* table  = tab_load(tablix);
    field_t field;
    sch_get_field(schidx, "CREDIT", &field);
    int64_t* element = malloc(field.size);
    int64_t ans[3];
    int counter = 0;
    tab_for_each_element(table,chunk, chblix, element, (&field)){
        ans[counter++] = *element;
    }
    assert(ans[0] == 10);
    assert(ans[1] == 20);
    assert(ans[2] == 30);
    db_drop(db);
}

DEFINE_TEST(update) {
    db_t* db = db_init("test.db");
    int64_t schidx = init_schema();
    int64_t tablix = tab_init(db, "test", schidx);
    insert_data(tablix, schidx, 1);
    field_t field;
    sch_get_field(schidx, "CREDIT", &field);
    int64_t element = 30;
    chblix_t res = tab_get_row(db, tablix, &field, &element, INT);
    assert(chblix_cmp(&res, &CHBLIX_FAIL) != 0);
    int64_t new_element = 100;
    assert(tab_update_element(tablix, &res, &(field), &new_element) == TABLE_SUCCESS);
    int64_t read_element;
    assert(tab_get_element(tablix, &res, &(field), &read_element) == TABLE_SUCCESS);
    assert(read_element == 100);
    db_drop(db);
}

DEFINE_TEST(delete){
    db_t* db = db_init("test.db");
    int64_t schidx = init_schema();
    int64_t tablix = tab_init(db, "test", schidx);
    insert_data(tablix, schidx, 1);
    field_t field;
    sch_get_field(schidx, "CREDIT", &field);
    int64_t element = 30;
    chblix_t res = tab_get_row(db, tablix, &field, &element, INT);
    assert(chblix_cmp(&res, &CHBLIX_FAIL) != 0);
    assert(tab_delete(tablix, &res) == TABLE_SUCCESS);
    res = tab_get_row(db, tablix, &field, &element, INT);
    assert(chblix_cmp(&res, &CHBLIX_FAIL) == 0);
    db_drop(db);
}

DEFINE_TEST(varchar){
    db_t* db = db_init("test.db");

    int64_t schidx = sch_init();
    sch_add_varchar_field(schidx, "NAME");
    sch_add_varchar_field(schidx, "SURNAME");
    sch_add_varchar_field(schidx, "BIG_STRING");
    int64_t tablix = tab_init(db, "test", schidx);
    tab_row(
            vch_ticket_t NAME;
            vch_ticket_t SURNAME;
            vch_ticket_t BIG_STRING;
    );
    row.NAME = vch_add(db->varchar_mgr_idx, "Alex");
    row.SURNAME = vch_add(db->varchar_mgr_idx, "Smith");
    row.BIG_STRING = vch_add(db->varchar_mgr_idx, "This is a big string\n "
                             "with multiple lines\n "
                             "and it is very long\n"
                             "and it is very long\n");
    chblix_t res = tab_insert(tablix, &row);
    assert(chblix_cmp(&res, &CHBLIX_FAIL) != 0);
    table_t* table = tab_load(tablix);
    field_t field;
    sch_get_field(schidx, "BIG_STRING", &field);
    vch_ticket_t* element = malloc(field.size);
    tab_for_each_element(table, chunk, chblix, element, &field){
        char* str = malloc(element->size);
        vch_get(db->varchar_mgr_idx, element, str);
        printf("%s\n", str);
        free(str);
    }
    db_drop(db);
}

DEFINE_TEST(several_tables){
    db_t* db = db_init("test.db");

    int64_t schidx = sch_init();
    sch_add_varchar_field(schidx, "NAME");
    sch_add_varchar_field(schidx, "SURNAME");
    tab_row(
            vch_ticket_t NAME;
            vch_ticket_t SURNAME;
            vch_ticket_t BIG_STRING;
    );
    sch_add_varchar_field(schidx, "BIG_STRING");
    int64_t tablix1 = tab_init(db, "BIG_STR", schidx);


    int64_t schidx2 = sch_init();
    sch_add_int_field(schidx2, "ID");
    sch_add_varchar_field(schidx2, "NAME");
    sch_add_float_field(schidx2, "SCORE");
    sch_add_bool_field(schidx2, "PASS");
    int64_t tablix2 = tab_init(db, "STUDENTS", schidx2);

    int64_t read_tablix1 = mtab_find_tab(db->meta_table_idx, "BIG_STR");
    assert(read_tablix1 == tablix1);

    int64_t read_tablix2 = mtab_find_tab(db->meta_table_idx, "STUDENTS");
    assert(read_tablix2 == tablix2);

    db_drop(db);
}

DEFINE_TEST(print){
    db_t* db = db_init("test.db");
    int64_t schidx = init_schema();
    int64_t tablix = tab_init(db, "test", schidx);
    insert_data(tablix, schidx, 2);
    tab_print(db, tablix);
    db_drop(db);
}

DEFINE_TEST(join){
    db_t* db = db_init("test.db");
    int64_t left = table_bank(db, 1);
    tab_print(db, left);
    int64_t right = table_student(db, 1);
    tab_print(db, right);
    int64_t join_tablix = tab_join(db, left, right, "NAME", "NAME","JOIN");
    tab_print(db, join_tablix);
    db_drop(db);
}

DEFINE_TEST(select){
    db_t* db = db_init("test.db");
    int64_t tablix = table_student(db, 1);
    float value = 20.0f;
    int64_t sel_tablix = tab_select_op(db, tablix, "SELECT", "SCORE", COND_GT, &value, FLOAT);
    tab_print(db, sel_tablix);
    table_t* sel_table_t = tab_load(sel_tablix);
    field_t* field = malloc(sizeof(field_t));
    assert(sch_get_field(sel_table_t->schidx,  "SCORE", field) == SCHEMA_SUCCESS);
    float element;
    tab_for_each_element(sel_table_t,chunk, chblix, &element, field){
        assert(element > value);
    }
    tab_drop(db, sel_tablix);

    sel_tablix = tab_select_op(db, tablix, "SELECT", "SCORE", COND_GTE, &value, FLOAT);
    sel_table_t = tab_load(sel_tablix);
    assert(sch_get_field(sel_table_t->schidx,  "SCORE", field) == SCHEMA_SUCCESS);
    tab_for_each_element(sel_table_t, chunk2, chblix2, &element, field){
        assert(element >= value);
    }
    tab_drop(db, sel_tablix);

    sel_tablix = tab_select_op(db, tablix, "SELECT", "SCORE", COND_LT, &value, FLOAT);
    sel_table_t = tab_load(sel_tablix);
    assert(sch_get_field(sel_table_t->schidx,  "SCORE", field) == SCHEMA_SUCCESS);
    tab_for_each_element(sel_table_t, chunk3, chblix3, &element, field){
        assert(element < value);
    }
    tab_drop(db, sel_tablix);

    sel_tablix = tab_select_op(db, tablix, "SELECT", "SCORE", COND_LTE, &value, FLOAT);
    sel_table_t = tab_load(sel_tablix);
    assert(sch_get_field(sel_table_t->schidx,  "SCORE", field) == SCHEMA_SUCCESS);
    tab_for_each_element(sel_table_t, chunk4, chblix4, &element, field){
        assert(element <= value);
    }
    tab_drop(db, sel_tablix);

    value = 10.5f;
    sel_tablix = tab_select_op(db, tablix, "SELECT", "SCORE", COND_NEQ, &value, FLOAT);
    sel_table_t = tab_load(sel_tablix);
    assert(sch_get_field(sel_table_t->schidx,  "SCORE", field) == SCHEMA_SUCCESS);
    tab_for_each_element(sel_table_t, chunk5, chblix5, &element, field){
        assert(element != value);
    }

    free(field);

    tab_drop(db, sel_tablix);
    db_drop(db);
}

DEFINE_TEST(update_row_op){
    db_t* db = db_init("test.db");
    assert(db != NULL);
    int64_t mtabidx = mtab_init();
    int64_t tablix = table_student(db, 1);
    float value = 20.5f;
    table_t* table = tab_load(tablix);
    schema_t* schema = sch_load(table->schidx);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            bool PASS;
            );
    row.ID = 10;
    strncpy(row.NAME,"Nick", 10);
    row.SCORE = 10.5f;
    row.PASS = true;
    int res = tab_update_row_op(db, tablix, &row, "SCORE", COND_EQ, &value, FLOAT);
    assert(res == TABLE_SUCCESS);
    field_t field;
    sch_get_field(table->schidx, "SCORE", &field);
    bool flag = false;
    tab_for_each_row(table, chunk, chblix, &row, schema){
        if(row.ID == 10){
            assert(row.SCORE == 10.5f);
            flag = true;
        }
    }
    assert(flag);
    tab_drop(db, tablix);
    db_drop(db);
}

DEFINE_TEST(update_element_op){
    db_t* db = db_init("test.db");
    assert(db != NULL);
    int64_t tablix = table_student(db, 1);
    char* value = malloc(10);
    strncpy(value, "Nick", 10);
    table_t* table = tab_load(tablix);
    schema_t* schema = sch_load(table->schidx);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            bool PASS;
    );
    float element = 10.5f;
    int res = tab_update_element_op(db, tablix, &element, "SCORE", "NAME", COND_EQ, value, CHAR);
    assert(res == TABLE_SUCCESS);
    field_t field;
    sch_get_field(table->schidx, "SCORE", &field);
    bool flag = false;
    tab_for_each_row(table, chunk, chblix, &row, schema){
        if(strcmp(row.NAME, "Nick") == 0){
            assert(row.SCORE == 10.5f);
            flag = true;
        }
    }
    assert(flag);
    free(value);
    tab_drop(db, tablix);
    db_drop(db);
}

DEFINE_TEST(delete_op){
    db_t* db = db_init("test.db");
    assert(db != NULL);
    int64_t tablix = table_student(db, 1);
    char* value = malloc(10);
    strncpy(value, "Nick", 10);
    table_t* table = tab_load(tablix);
    schema_t* schema = sch_load(table->schidx);
    tab_row(
            int64_t ID;
            char NAME[10];
            float SCORE;
            bool PASS;
    );
    int res = tab_delete_op(db, tablix, "NAME", COND_EQ, value);
    assert(res == TABLE_SUCCESS);
    field_t field;
    sch_get_field(table->schidx, "SCORE", &field);
    tab_for_each_row(table, chunk2, chblix, &row, schema){
        if(strcmp(row.NAME, "Nick") == 0){
            assert(false);
        }
    }
    free(value);
    tab_drop(db, tablix);
    db_drop(db);
}




int main(){
    RUN_SINGLE_TEST(create_add_foreach);
    RUN_SINGLE_TEST(update);
    RUN_SINGLE_TEST(delete);
    RUN_SINGLE_TEST(varchar);
    RUN_SINGLE_TEST(several_tables);
    RUN_SINGLE_TEST(print);
    RUN_SINGLE_TEST(join);
    RUN_SINGLE_TEST(select);
    RUN_SINGLE_TEST(update_row_op);
    RUN_SINGLE_TEST(update_element_op);
    RUN_SINGLE_TEST(delete_op);
}