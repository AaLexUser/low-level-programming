#include "../src/test.h"
#include "../../src/backend/varchar/varchar_manager.h"
#include <string.h>

DEFINE_TEST(3b_string){
    VarcharManager* varcharManager = varcharmanager_create();
    char str[] = "hi";
    Chblidx* chblidx =  get_pfchblidx_from_str(varcharManager, str);
    char* str_new = get_str_from_pfbi(varcharManager, chblidx, strlen(str));
    assert(strcmp(str_new, str) == 0);
}

DEFINE_TEST(20b_string){
    VarcharManager* varcharManager = varcharmanager_create();
    char str[] = "hello, how are you?";
    Chblidx* chblidx =  get_pfchblidx_from_str(varcharManager, str);
    char* str_new = get_str_from_pfbi(varcharManager, chblidx, strlen(str));
    assert(strcmp(str_new, str) == 0);
}

int main(){
    RUN_SINGLE_TEST(3b_string);
    RUN_SINGLE_TEST(20b_string);

}