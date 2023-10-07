#include "pager.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
const uint32_t PAGE_SIZE = 4096;

void free_pages(void* pages){
    if(pages == NULL){
        return;
    }
    Page* current = pages;
    while (current != NULL)
    {
        Page* next = current->next_page;
        free_page(current);
        current = next;
    }
}

Pager* pager_open(const char* filename){
    int fd = open(filename,
                  O_RDWR | // Read/Write mode
                  O_CREAT, // Create file if it does not exist
                  S_IWUSR | // User write permission
                  S_IRUSR // User read permission
    );
    if (fd == -1){
        printf("Unable to open file\n");
        exit(EXIT_FAILURE);
    }
    off_t file_length = lseek(fd, 0, SEEK_END);
    Pager* pager = malloc(sizeof(Pager));
    pager->file_descriptor = fd;
    pager->file_length = file_length;
    pager->pages = create_vector_with_free(TABLE_MAX_PAGES, sizeof(Page), free_pages);
    return pager;
}

void* get_linked_pages(Pager* pager, uint32_t lpid){
    void *src;
    struct stat statbuf;
    if(fstat(pager->file_descriptor, &statbuf) < 0){
        exit(0); //TODO: Сделать нормальный exception
    }
    if(vector_get(pager->pages, lpid) == NULL){
        //Cache miss. Allocate memory and load from file.
        void* linked_pages = create_linked_list();
        void* page = malloc(PAGE_SIZE);
        linked_list_add_node(linked_pages, page);

        if (lpid <= num_pages){
            lseek(pager->file_descriptor, lpid * PAGE_SIZE, SEEK_SET); //SEEK_SET - The offset is counted from the beginning of the file
            if(fstat(pager->file_descriptor, &statbuf) < 0){
                printf("Error reading file: %d\n", errno);
                exit(EXIT_FAILURE); //TODO: Сделать нормальный exception
            }
            if((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, pager->file_descriptor, 0)) == MAP_FAILED){
                exit(0);//TODO: Сделать нормальный exception
            }
            
            memcpy(page, src + lpid * PAGE_SIZE, PAGE_SIZE);
        }
        vector_push(pager->pages, page);
    }
    return vector_get(pager->pages, lpid);
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size){
    void *dst;
    struct stat statbuf;
    if(vector_get(pager->pages, page_num) == NULL){
        printf("Tried to flush null page\n");
        exit(EXIT_FAILURE);
    }
    if(fstat(pager->file_descriptor, &statbuf) < 0){
        exit(0); //TODO: Сделать нормальный exception
    }

    off_t offset = lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET);

    if (offset == -1){
        printf("Error seeking: %d\n", errno);
        exit(EXIT_FAILURE);
    }

    if ((dst = mmap(0, statbuf.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, pager->file_descriptor, 0)) == MAP_FAILED )
    memcpy(dst + page_num * PAGE_SIZE, vector_get(pager->pages, page_num), size);
}