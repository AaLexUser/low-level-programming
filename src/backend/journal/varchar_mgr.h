#pragma once

#include "backend/page_pool/linked_blocks.h"
#include "utils/logger.h"
#include <string.h>

#ifndef VARCHTAB
int64_t vch_vachar_mgr_idx;
#define VARCHTAB vch_vachar_mgr_idx
#endif

#define VCH_BLOCK_SIZE 30

typedef struct vch_ticket{
    chblix_t block;
    int64_t size;
}vch_ticket_t;

int64_t vch_init();
vch_ticket_t vch_add(char* varchar);
int vch_get(vch_ticket_t* ticket, char* varchar);