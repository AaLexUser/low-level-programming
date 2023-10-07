#pragma once
#include <string.h>
#include "../../backend/io/input_buffer.h"
#include "../../backend/table/table.h"
#include "../../backend/db/db.h"
typedef enum {
    META_COMMAND_SUCCESS,
    META_COMMAND_UNRECOGNIZED_COMMAND
} MetaCommandResult;

MetaCommandResult do_meta_command(InputBuffer* inputBuffer, Table* table);