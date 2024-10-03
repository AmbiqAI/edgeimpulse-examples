/* TSI 2023.gen */
// -----------------------------------------------------------------------------
// Copyright (c) 2008-23 Think Silicon Single Member PC
// Think Silicon Single Member PC Confidential Proprietary
// -----------------------------------------------------------------------------
//     All Rights reserved - Unpublished -rights reserved under
//         the Copyright laws of the European Union
//
//  This file includes the Confidential information of Think Silicon Single
//  Member PC The receiver of this Confidential Information shall not disclose
//  it to any third party and shall protect its confidentiality by using the
//  same degree of care, but not less then a reasonable degree of care, as the
//  receiver uses to protect receiver's own Confidential Information. The entire
//  notice must be reproduced on all authorized copies and copies may only be
//  made to the extent permitted by a licensing agreement from Think Silicon
//  Single Member PC.
//
//  The software/data is provided 'as is', without warranty of any kind,
//  expressed or implied, including but not limited to the warranties of
//  merchantability, fitness for a particular purpose and noninfringement. In no
//  event shall Think Silicon Single Member PC be liable for any claim, damages
//  or other liability, whether in an action of contract, tort or otherwise,
//  arising from, out of or in connection with the software.
//
//  For further information please contact:
//
//                    Think Silicon Single Member PC
//                    http://www.think-silicon.com
//                    Patras Science Park
//                    Rion Achaias 26504
//                    Greece
// -----------------------------------------------------------------------------
#include "nema_hal.h"
#include "nema_regs.h"
#include "nema_ocd.h"


void nema_ocd_write(uint32_t addr, uint32_t value) { // instant effect (not syncing with CL)
  //while( (nema_reg_read(NEMA_DBG_STATUS) & 0x2) ) {aprint("Waiting TxFIFO to become nonfull...\n");}
    nema_reg_write(NEMA_DBG_DATA, value);
    nema_reg_write(NEMA_DBG_ADDR, addr);
}

uint32_t nema_ocd_read(void) {
    while( nema_reg_read(NEMA_DBG_STATUS) == 0U ) {

    }

    uint32_t value = nema_reg_read(NEMA_DBG_DATA);
    (void)nema_reg_read(NEMA_DBG_ADDR);
    return(value);
}
uint32_t nema_ocd_read_hi(void) {
    //while(!nema_reg_read(NEMA_DBG_STATUS));
    return nema_reg_read(NEMA_DBG_DATA);
}

uint32_t nema_ocd_read_lo(void) {
    //while(!nema_reg_read(NEMA_DBG_STATUS));
    return nema_reg_read(NEMA_DBG_ADDR);
}


uint32_t nema_ocd_read_counter(uint32_t counter) {
    nema_ocd_write(NEMA_OCD_MAP, counter);
    return nema_ocd_read();
}

static void nema_ocd_broadcast(uint32_t cmd) {    //misra
    // misra
    uint32_t val = 0x007fffU;
    val <<= 6;
    nema_ocd_write(cmd, val);
//  nema_ocd_write(cmd, NEMA_OCD_TOP      | (0x007fffU << 6));
    nema_ocd_write(cmd, NEMA_OCD_C0       | (0x07ffffU << 6));
    nema_ocd_write(cmd, NEMA_OCD_C0_IMEM  | (0x000003U << 6));
    nema_ocd_write(cmd, NEMA_OCD_C0_SCHED | (0x0001ffU << 6));
    nema_ocd_write(cmd, NEMA_OCD_MS       | (0xffffffU << 6));
}

//clear counters
void nema_ocd_clear(void)
{
    nema_ocd_broadcast(NEMA_OCD_CLEAR);
}

//stop counters
void nema_ocd_stop(void)
{
    nema_ocd_broadcast(NEMA_OCD_STOP);
}

//start counters
void nema_ocd_start(void)
{
    nema_ocd_broadcast(NEMA_OCD_START);
}
