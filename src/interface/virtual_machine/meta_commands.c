#include "meta_commands.h"

MetaCommandResult do_meta_command(InputBuffer* inputBuffer, Table* table){
    if(strcmp(inputBuffer->buffer, ".exit") == 0){
        close_input_buffer(inputBuffer);
        db_close(table);
        exit(EXIT_SUCCESS);
    }
    return META_COMMAND_UNRECOGNIZED_COMMAND;
}