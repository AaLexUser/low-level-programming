#include "page.h"

const uint32_t PAGE_SIZE = 4096;

Page* create_page_with_size(const uint32_t size){
    Page* page = malloc(sizeof(Page));
    page->data = malloc(size);
    page->is_dirty = false;
    page->next_page = NULL;
    return page;
}

Page* create_page(){
    return create_page_with_size(PAGE_SIZE);
}

void free_page(Page* page){
    if(page == NULL){
        return;
    }
    free(page->data);
    free(page);
}
