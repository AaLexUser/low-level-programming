#include "db.h"

Table* db_open(const char* filename){
    Pager* pager = pager_open(filename);
    uint32_t num_rows = pager->file_length / ROW_SIZE;
    Table* table = malloc(sizeof(table));
    table->pager = pager;
    table->num_rows = num_rows;
    return table;
}

void db_close(Table* table){
    Pager* pager = table->pager;
    uint32_t num_full_pages = table->num_rows / ROWS_PER_PAGE;
    for (uint32_t i = 0; i < num_full_pages; i++){
        if(vector_get(pager->pages, i) == NULL){
            continue;
        }
        pager_flush(pager, i, PAGE_SIZE);
    }
    vector_free(pager->pages);

    //There may be a partial page to write to the end of the file
    uint32_t num_additional_rows = table->num_rows % ROWS_PER_PAGE;
    if (num_additional_rows > 0){
        uint32_t page_num = num_full_pages;
        if (vector_get(pager->pages, page_num) != NULL){
            pager_flush(pager, page_num, num_additional_rows * ROW_SIZE);
            free(pager->pages[page_num]);
            pager->pages[page_num] = NULL;
        }
    }
    vector_free(pager->pages);
    free(pager);
    free(table);
}