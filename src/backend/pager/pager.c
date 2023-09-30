#include "pager.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
const uint32_t PAGE_SIZE = 4096;

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
    for(uint32_t i = 0; i < TABLE_MAX_PAGES; i++){
        pager->pages[i] = NULL;
    }
    return pager;
}

void* get_page(Pager* pager, uint32_t page_num){
    void *src;
    struct stat statbuf;
    if(page_num > TABLE_MAX_PAGES){
        printf("Tried to fetch page number out of bounds. %d > %d\n", page_num, TABLE_MAX_PAGES);
        exit(EXIT_FAILURE);
    }
    if(fstat(pager->file_descriptor, &statbuf) < 0){
        exit(0); //TODO: Сделать нормальный exception
    }
    if(pager->pages[page_num] == NULL){
        //Cache miss. Allocate memory and load from file.
        void* page = malloc(PAGE_SIZE);
        uint32_t num_pages = pager->file_length / PAGE_SIZE;
        if(pager->file_length % PAGE_SIZE){
            num_pages += 1;
        }

        if (page_num <= num_pages){
            lseek(pager->file_descriptor, page_num * PAGE_SIZE, SEEK_SET); //SEEK_SET - The offset is counted from the beginning of the file
            if(fstat(pager->file_descriptor, &statbuf) < 0){
                printf("Error reading file: %d\n", errno);
                exit(EXIT_FAILURE); //TODO: Сделать нормальный exception
            }
            if((src = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, pager->file_descriptor, 0)) == MAP_FAILED){
                exit(0);//TODO: Сделать нормальный exception
            }
            
            memcpy(page, src + page_num * PAGE_SIZE, PAGE_SIZE);
        }
        pager->pages[page_num] = page;
    }
    return pager->pages[page_num];
}

void pager_flush(Pager* pager, uint32_t page_num, uint32_t size){
    void *dst;
    struct stat statbuf;
    if(pager->pages[page_num] == NULL){
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
    memcpy(dst + page_num * PAGE_SIZE, pager->pages[page_num], size);
}