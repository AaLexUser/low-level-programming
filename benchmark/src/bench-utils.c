#include "backend/db/db.h"

void init_db(const char* filename){
    /* Init new db file */
    if(db_init(filename) != 0){
        printf("Failed to init file\n");
        exit(EXIT_FAILURE);
    }
    if(pg_file_size() != 0){
        if(pg_delete() != 0){
            printf("Failed to delete file\n");
            exit(EXIT_FAILURE);
        }
        if(db_init(filename) != 0){
            printf("Failed to init file\n");
            exit(EXIT_FAILURE);
        }
    }
}

int out_file(const char* filename){
    int fd = open(filename,
                  O_RDWR | //Read Write mode
                  O_CREAT | // Create file if it does not exist
                  O_TRUNC, // Truncate file
                  S_IWUSR|  // User write permission
                  S_IRUSR // User read permission
    );
    if(fd == -1){
        printf("Failed to open file %s %d %s.", filename, errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return fd;
}