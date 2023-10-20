#include "../src/test.h"
#include "../src/backend/io/file_manager.h"
#include <stdio.h>
DEFINE_TEST(write_and_read){
    FileManager* fileManager = create_file_manager_by_filename("test.db");
    char str[] = "12345678";
    write(fileManager->file_descriptor, str, 8);
    char* read_str = read_file(fileManager, 0, 8);
    assert(strcmp(str, read_str) == 0);
    close_file_manager(fileManager);
    free(read_str);
    remove("test.db");
}

int main(){
    RUN_SINGLE_TEST(write_and_read);
}