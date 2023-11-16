#include "varchar_mgr.h"

/**
 * @brief       Initialize the varchar manager
 * @return      index of the varchar manager index on success, TABLE_FAIL on failure
 */

int64_t vch_init(){
    vch_vachar_mgr_idx =  lb_ppl_init(VCH_BLOCK_SIZE);
    return vch_vachar_mgr_idx;
}

/**
 * @brief       Add a varchar
 * @param[in]   varchar: string to add
 * @return      vch_ticket_t of varchar on success, CHBLIX_FAIL on failure
 */

vch_ticket_t vch_add(char* varchar){
    vch_ticket_t ticket;
    ticket.block = lb_alloc(VARCHTAB);
    ticket.size = strlen(varchar)+1;
    lb_write(
            VARCHTAB,
            &ticket.block,
            varchar,
            ticket.size,
            0
            );
    return ticket;
}

/**
 * @brief       Get a varchar
 * @param[in]   ticket: ticket of varchar
 * @param[out]  varchar: string destination
 * @return      LB_SUCCESS on success, LB_FAIL on failure
 */

int vch_get(vch_ticket_t* ticket, char* varchar){
    return lb_read(
            VARCHTAB,
            &ticket->block,
            varchar,
            ticket->size,
            0
            );
}

/**
 * @brief       Delete a varchar
 * @param[in]   ticket: ticket of varchar
 * @return      LB_SUCCESS on success, LB_FAIL on failure
 */

int vch_delete(vch_ticket_t* ticket){
    return lb_dealloc(VARCHTAB, &ticket->block);
}


