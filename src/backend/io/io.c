#include "io.h"
InputBuffer* new_input_buffer(){
    InputBuffer* inputBuffer = (InputBuffer*)malloc(sizeof(InputBuffer));
    inputBuffer->buffer = NULL;
    inputBuffer->buffer_length = 0;
    inputBuffer->input_length = 0;
    return inputBuffer;
}

void close_input_buffer(InputBuffer* inputBuffer){
    free(inputBuffer->buffer);
    free(inputBuffer);
}

void read_input(InputBuffer* inputBuffer){
    ssize_t bytes_read = getline(&(inputBuffer->buffer),
                                 &(inputBuffer->buffer_length),
                                 stdin);
    if (bytes_read <= 0) {
        printf("Error reading input\n");
        exit(EXIT_FAILURE);
    }
    // Ignore trailing newline
    inputBuffer->input_length = bytes_read - 1;
    inputBuffer->buffer[bytes_read - 1] = 0;
}
