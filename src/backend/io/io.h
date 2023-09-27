#pragma once
#include <stdio.h>
#include <stdlib.h>
typedef struct {
    char* buffer;
    size_t buffer_length;
    ssize_t input_length;
} InputBuffer;

InputBuffer* new_input_buffer();

void close_input_buffer(InputBuffer* inputBuffer);
void read_input(InputBuffer* inputBuffer);