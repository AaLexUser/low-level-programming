#include "cstring.h"

int32_t compare_strings(String* a, String* b) {
    return strcmp(a->data, b->data);
}
