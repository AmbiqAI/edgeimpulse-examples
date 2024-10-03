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
#include "nema_sys_defs.h"
#include "nema_hal.h"
#include "nema_regs.h"
#include "nema_ringbuffer.h"
#include "nema_cmdlist.h"
#include "nema_error.h"
#include "nema_error_intern.h"

#define RINGSTOP_NOFLUSH    (0x1U)
#define RINGSTOP_NOTRIGGER  (0x2U)
#define RINGSTOP_ENABLE     (0x4U)

//variable pointing to shmem
static nema_ringbuffer_t *ring;

//ring buffer mapped to process virtual space
static uint32_t *ring_base_virt;

//process local copy
static uintptr_t  ring_base_phys;
static int        ring_size;
static int        ring_size_bytes = 0;

int
nema_rb_init(nema_ringbuffer_t *rb, int reset)
{
    ring_size_bytes = rb->bo.size;
    if ( ring_size_bytes <= 0 ) {
        nema_set_error(NEMA_ERR_RB_INIT_FAILURE);
        return -1;
    }

    ring           = rb;
    ring_base_virt = (uint32_t *)rb->bo.base_virt;
    ring_base_phys = rb->bo.base_phys;
#ifdef ADDR64BIT
    ring_size      = (ring_size_bytes/4)-6;
#else
    ring_size      = (ring_size_bytes/4)-4;
#endif
    if (reset != 0) {
        nema_rb_reset();
    }
    return 0;
}

void
nema_rb_reset(void)
{
    ring->offset     = 0;
    ring->last_submission_id = 0;
#ifdef ADDR64BIT
    ring_base_virt[ring_size  ] = 0xff000000U | NEMA_CMDADDR_H;
    ring_base_virt[ring_size+1] = ((uint32_t)(ring_base_phys >> 32));
    ring_base_virt[ring_size+2] = 0xff000000U | NEMA_CMDADDR; //TODO: check virtual addr? 32-bit because of CPU/OS ?
    ring_base_virt[ring_size+3] = (uint32_t)ring_base_phys;
    ring_base_virt[ring_size+4] = 0xff000000U | NEMA_CMDSIZE;
    ring_base_virt[ring_size+5] = (uint32_t)ring_size_bytes;
#else
    ring_base_virt[ring_size  ] = 0xff000000U | NEMA_CMDADDR; //TODO: check virtual addr? 32-bit because of CPU/OS ?
    ring_base_virt[ring_size+1] = (uint32_t)ring_base_phys;
    ring_base_virt[ring_size+2] = 0xff000000U | NEMA_CMDSIZE;
    ring_base_virt[ring_size+3] = (uint32_t)ring_size_bytes;
#endif

    nema_buffer_flush(&ring->bo);

    nema_reg_write(NEMA_CLID, 0);

    // Ring buffer allocation
#ifdef ADDR64BIT
    nema_reg_write(NEMA_CMDRINGSTOP_H, ((uint32_t)(ring_base_phys >> 32)));
    nema_reg_write(NEMA_CMDADDR_H,     ((uint32_t)(ring_base_phys >> 32)));
#endif
    nema_reg_write(NEMA_CMDRINGSTOP, ((uint32_t)ring_base_phys) | RINGSTOP_NOTRIGGER | RINGSTOP_ENABLE);
    nema_reg_write(NEMA_CMDADDR,     ((uint32_t)ring_base_phys));
    nema_reg_write(NEMA_CMDSIZE,     (uint32_t)ring_size_bytes);
}
//--------------------------------------------------------------------------

static inline void
nema_rb_single_write(uint32_t data)
{
    ring_base_virt[ring->offset] = data;
    ++ring->offset;

    if(ring->offset >= ring_size) {
        ring->offset = 0;
    }
}
//--------------------------------------------------------------------------

static inline void
nema_rb_update_ringstop(void)
{
    nema_buffer_flush(&ring->bo);
#ifdef ADDR64BIT
    uintptr_t val = ring_base_phys;
    val += (uint32_t)ring->offset*4U;

    uint32_t val_l = (uint32_t)val;
    uint32_t val_h = (uint32_t)(val>>32);

    nema_reg_write(NEMA_CMDRINGSTOP_H, val_h);
    nema_reg_write(NEMA_CMDRINGSTOP, val_l | RINGSTOP_ENABLE);
#else
    uint32_t val = (uint32_t)ring_base_phys;
    val += (uint32_t)ring->offset*4U;
    nema_reg_write(NEMA_CMDRINGSTOP, val | RINGSTOP_ENABLE);
#endif
}

//--------------------------------------------------------------------------
int32_t
nema_rb_submit_cmdlist2(uintptr_t base_phys, int size)
{
    if (size == 0) {
        return -1;
    }

    if (ring->offset+4 >= ring_size) {
        do {
            nema_rb_single_write(CL_NOP);
        } while( ring->offset != 0);
    }

    //Jumps to command lists must be aligned to 8 bytes
    while( ring->offset%4 != 0) {
        nema_rb_single_write(CL_NOP);
    }

#ifdef ADDR64BIT
    nema_rb_single_write(NEMA_CMDADDR_H);
    nema_rb_single_write((uint32_t)(base_phys >> 32));
#endif
    nema_rb_single_write(NEMA_CMDADDR);
    nema_rb_single_write((uint32_t)base_phys);
    nema_rb_single_write(NEMA_CMDSIZE | CL_PUSH);
    nema_rb_single_write((uint32_t)size);

    nema_rb_update_ringstop();

    ring->last_submission_id = ring->last_submission_id + 1;
    if (ring->last_submission_id > SUBMISSION_ID_MASK) {
        //TODO: we should probably wait for CL with id SUBMISSION_ID_MASK to finish
        ring->last_submission_id = 0;
    }

    return ring->last_submission_id;
}

int32_t
nema_rb_submit_cmdlist(nema_buffer_t *bo, int size)
{
    return nema_rb_submit_cmdlist2(bo->base_phys, size);
}

//--------------------------------------------------------------------------
void
nema_rb_inline_cmd(uint32_t reg, uint32_t data)
{
    if (ring->offset+2 > ring_size) {
        do {
            nema_rb_single_write(CL_NOP);
        } while( ring->offset != 0);
    }

    nema_rb_single_write(reg);
    nema_rb_single_write(data);


    if ( (reg&0xff000000U) != 0U ) {
        nema_rb_update_ringstop();
    }
}

//--------------------------------------------------------------------------
//force flush by jumping
void
nema_rb_force_flush(void)
{
    if (ring->offset+4 >= ring_size) {
        do {
            nema_rb_single_write(CL_NOP);
        } while( ring->offset != 0);
    }

    //Jumps to command lists must be aligned to 8 bytes
    while( ring->offset%4 != 0) {
        nema_rb_single_write(CL_NOP);
    }

    uint32_t phys_addr_to_jump = (uint32_t)ring_base_phys+((uint32_t)ring->offset+4U)*4U;

#if 0
    nema_rb_inline_cmd(NEMA_CMDADDR, phys_addr_to_jump);
    nema_rb_inline_cmd(NEMA_CMDSIZE | 0xff000000U, ring_size_bytes);
#else
    nema_rb_single_write(NEMA_CMDADDR | 0xff000000U); //TODO: 64-bit feature support?
    nema_rb_single_write(phys_addr_to_jump);

    nema_rb_single_write(NEMA_CMDSIZE | 0xff000000U);
    nema_rb_single_write((uint32_t)ring_size_bytes);

    nema_rb_update_ringstop();
#endif
}

void
nema_rb_submit_cl_id(int cl_id)  {
    if (cl_id > 0) {
        // (void)nema_mutex_lock(MUTEX_RB);
        nema_rb_inline_cmd(NEMA_CLID, (uint32_t)cl_id);
        nema_rb_inline_cmd(NEMA_INTERRUPT, 1U);
        nema_rb_force_flush();
        // (void)nema_mutex_unlock(MUTEX_RB);
    }
}

#ifdef UNIT_TEST
void
nema_rb_add_NOP()
{
    nema_rb_single_write(CL_NOP);
}
#endif //UNIT_TEST
