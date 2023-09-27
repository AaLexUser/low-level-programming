#include "custom_print.h"
#include <stdio.h>

void print_prompt(){
    fputs("db > ", stdout);
}

void print_row(Row* row){
    printf("(%d, %s, %s)\n", row->id, row->username, row->email);
}