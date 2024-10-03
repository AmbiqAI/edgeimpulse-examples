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
#include "nema_regs.h"
#include "nema_hal.h"
#include "nema_ringbuffer.h"
#include "nema_cmdlist.h"
#include "nema_error.h"
#include "nema_error_intern.h"
#include "nema_programHW.h"

#if 0

#include <assert.h>
#define CL_ASSERT assert

#else
#define CL_ASSERT(...)
#endif

//no debug prints
#define PRINT_D(...)
#define PRINT_E(...)

//enable debug prints
//#include <stdio.h>
//#define CMDLIST_DEBUG
//#define PRINT_D printf
//#define PRINT_E printf

#ifndef NEMA_MEM_POOL_CL
#define NEMA_MEM_POOL_CL 0
#endif

static TLS_VAR nema_cmdlist_t *bound_cl = NULL;

#ifndef CL_SIZE_BYTES
#define CL_SIZE_BYTES 1024
#endif

#ifdef ADDR64BIT
#define JUMP_CMD_SIZE 3
#else
#define JUMP_CMD_SIZE 2
#endif

#ifndef CL_ALMOST_FULL_LIMIT
#define CL_ALMOST_FULL_LIMIT 20
#endif

#define CL_INJECT       ( 0x1U) //(1U<<0)
#define CL_EXPANDABLE   ( 0x2U) //(1U<<1)
#define CL_HAS_CHILD    ( 0x4U) //(1U<<2)
#define CL_FLUSHED      ( 0x8U) //(1U<<3)
#define CL_PREALLOC     (0x10U) //(1U<<4)
#define CL_CIRCULAR     (0x20U) //(1U<<5)

#define CL_ADD_CMD(_reg_, _data_) do {                      \
    int off = bound_cl->offset;                             \
    ((uint32_t *)bound_cl->bo.base_virt)[off] = (uint32_t)(_reg_);    \
    ++off;                                                            \
    ((uint32_t *)bound_cl->bo.base_virt)[off] = (uint32_t)(_data_);   \
    bound_cl->offset = off+1;                                         \
    CL_ASSERT(bound_cl->offset <= bound_cl->size); \
    } while(false)

#if defined(NEMA_MULTI_PROCESS) || defined(NEMA_MULTI_THREAD)

#define MUTEX_LOCK \
    if (nema_context.explicit_submission == 1 && nema_context.implicit_submission_lock == 0U) \
    { \
        (void)nema_mutex_lock(MUTEX_RB);\
    }\
    else if (nema_context.explicit_submission == 0 && nema_context.implicit_submission_lock == 0U) \
    { \
        nema_context.implicit_submission_lock = 1;\
        (void)nema_mutex_lock(MUTEX_RB);\
    }\

#define MUTEX_UNLOCK \
    if (nema_context.explicit_submission == 1 && nema_context.implicit_submission_lock == 0U) \
    { \
        (void)nema_mutex_unlock(MUTEX_RB); \
    }\
    else if (nema_context.explicit_submission == 1 && nema_context.implicit_submission_lock == 1U) \
    { \
        nema_context.implicit_submission_lock = 0;\
        (void)nema_mutex_unlock(MUTEX_RB);\
    }\

#else

#define MUTEX_LOCK
#define MUTEX_UNLOCK

#endif

inline static void
nema_cl_set_flag(nema_cmdlist_t *cl, uint32_t flag) {

    if (cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return;
    }
    cl->flags |= flag;
}

inline static void
nema_cl_unset_flag(nema_cmdlist_t *cl, uint32_t flag) {
    if (cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return;
    }
    cl->flags &= ~flag;
}

inline static bool
nema_cl_flag_is_set(nema_cmdlist_t *cl, uint32_t flag) {
    if (cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return false;
    }
    return (cl->flags & flag) != 0U;
}

inline static int
nema_cl_space_left(nema_cmdlist_t *cl) {

    if (cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return -1;
    }
    int space_left = 0;
    if ( nema_cl_flag_is_set(cl, CL_CIRCULAR) ) {

        int ccl_size =  (cl->size/4)*2;
        int ccl_offset = cl->offset >= ccl_size
                         ? cl->offset - ccl_size
                         : cl->offset;

        space_left = (ccl_size - ccl_offset)/2;
    } else {
        space_left = (cl->size - cl->offset)/2;
    }

    return space_left;
}

inline static void
nema_cl_flush(nema_cmdlist_t *cl) {
    if ( cl != NULL) {
        PRINT_D("Try CL Flush @ 0x%08x\r\n", cl->bo.base_phys);
    }

    if ( (cl != NULL) && (cl->flags & CL_FLUSHED) == 0U ) {
        PRINT_D("CL Flush @ 0x%08x\r\n", cl->bo.base_phys);
        nema_buffer_flush(&cl->bo);
        nema_cl_set_flag(cl, CL_FLUSHED);
    }
}

nema_cmdlist_t
nema_cl_create_prealloc(nema_buffer_t *bo) {

    nema_cmdlist_t cmdlist = {0};

    if (bo == NULL) {
        nema_set_error(NEMA_ERR_INVALID_BO);
    }else{
        cmdlist.bo        = *bo;
        cmdlist.size      = bo->size/8*2;
        cmdlist.offset    = 0;
        cmdlist.next      = NULL;
        cmdlist.root      = NULL;
        cmdlist.flags     = CL_PREALLOC;
        cmdlist.submission_id = -1;
        PRINT_D("CL Create @ 0x%08x\r\n", bo->base_phys);
    }
    return cmdlist;
}

nema_cmdlist_t
nema_cl_create_sized(int size_bytes)
{
    nema_buffer_t bo;

    bo = nema_buffer_create_pool(NEMA_MEM_POOL_CL, size_bytes);
    (void)nema_buffer_map(&bo);

    nema_cmdlist_t cmdlist = nema_cl_create_prealloc(&bo);
    cmdlist.flags = 0U;

    return cmdlist;
}

nema_cmdlist_t
nema_cl_create(void) {
    nema_cmdlist_t cmdlist = nema_cl_create_sized(CL_SIZE_BYTES);
    cmdlist.flags     = CL_EXPANDABLE;
    return cmdlist;
}

void
nema_cl_destroy(nema_cmdlist_t *cl)
{
    nema_cmdlist_t *cur_cl = cl;

    CL_ASSERT(cur_cl->offset <= cur_cl->size);
    if (cur_cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return;
    }
    if ( (cur_cl->root != NULL) && ( (cur_cl->flags & CL_CIRCULAR) == 0U ) ) {
        cur_cl = cur_cl->root;
    }
    do {
        if (bound_cl == cur_cl) {
            nema_cl_unbind();
        }

        if ( (cur_cl->flags & CL_PREALLOC) != 0U) {
            //this was created in preallocated space.
            //don't deallocate it
            cur_cl = cur_cl->next;
            continue;
        }

        nema_buffer_unmap(&cur_cl->bo);
        nema_buffer_destroy(&cur_cl->bo);

        //if current cl has root (it's not root itself)
        //also free the corresponding struct
        nema_cmdlist_t *_next = cur_cl->next;
        if (cur_cl->root != NULL) {
            nema_host_free(cur_cl);
        }
        cur_cl = _next;

    } while (cur_cl != NULL);
}

void
nema_cl_rewind(nema_cmdlist_t *cl)
{
    nema_cmdlist_t *cur_cl = cl;

    if (cur_cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return;
    }
    if (cur_cl->root != NULL) {
        cur_cl = cur_cl->root;
    }
    do {
        if ( cur_cl->root != NULL && (cur_cl == bound_cl) ){
            bound_cl = cur_cl->root;
        }
        cur_cl->offset = 0;
        nema_cl_unset_flag(cur_cl, CL_HAS_CHILD);
        cur_cl = cur_cl->next;
    } while (cur_cl != NULL);
}

void
nema_cl_bind(nema_cmdlist_t *cl)
{
    nema_cmdlist_t *cur_cl = cl;

    if (cur_cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return;
    }

    if ((cur_cl->bo.base_phys & CL_ALIGNMENT_MASK) != 0u) {
        nema_set_error(NEMA_ERR_INVALID_CL_ALIGMENT);
        return;
    }

    if (bound_cl != NULL) {
        nema_cl_unbind();
    }


    while ((cur_cl->flags & CL_HAS_CHILD) != 0U) {
        cur_cl = cur_cl->next;
    }

    bound_cl = cur_cl;
}

void
nema_cl_bind_circular(nema_cmdlist_t *cl)
{
    nema_cmdlist_t *cur_cl = cl;

    if (cur_cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return;
    }

    if ((cur_cl->bo.base_phys & CL_ALIGNMENT_MASK) != 0u) {
        nema_set_error(NEMA_ERR_INVALID_CL_ALIGMENT);
        return;
    }

    //make sure we use the root cl
    if ( cur_cl->root != NULL ) {
        cur_cl = cur_cl->root;
    }

    if (bound_cl != NULL) {
        nema_cl_unbind();
    }

    nema_cl_set_flag(cur_cl, CL_CIRCULAR);

    bound_cl = cur_cl;
}

void
nema_cl_unbind(void)
{
    //add explicit return after the end
    if (bound_cl != NULL && ((bound_cl->offset + 2) <= bound_cl->size) ) {
        uint32_t *cl_base_virt = (uint32_t *) bound_cl->bo.base_virt;

        int off = bound_cl->offset;
        cl_base_virt[off] = CL_NOP | CL_RETURN;
        ++off;
        cl_base_virt[off] = 0U;

        nema_cl_unset_flag(bound_cl, CL_FLUSHED);
    }

    if (bound_cl != NULL) {
        nema_cl_unset_flag(bound_cl, CL_CIRCULAR);
    }

    // nema_cl_flush(bound_cl);

    bound_cl = NULL;
}

nema_cmdlist_t *
nema_cl_get_bound(void)
{
    return bound_cl;
}

static int
nema_extend_bound_cl(int cmd_no) {

    if (bound_cl == NULL) {
        nema_set_error(NEMA_ERR_NO_BOUND_CL);
        return -1;
    }

    //check if cmdlist has enough space
    //always leave enough space for a jump cmd
    if ( !nema_cl_flag_is_set(bound_cl, CL_EXPANDABLE) ) {
        PRINT_E("##### NON EXPANDABLE CMDLIST FULL !!!!!\n");
        nema_set_error(NEMA_ERR_NON_EXPANDABLE_CL_FULL);
        return -1;
    }

    int space_left = nema_cl_space_left(bound_cl);


    for (int i = 0; i < (space_left-JUMP_CMD_SIZE); ++i) {
        CL_ADD_CMD(CL_NOP, 0);
    }

    //expand CL if needed and add rest of commands
    nema_cmdlist_t *cl = bound_cl->next;

    if (cl == NULL) {

        CL_ASSERT( !nema_cl_flag_is_set(bound_cl, CL_CIRCULAR) );
        PRINT_D("Expanding CL %d\r\n", __LINE__);

        cl = (nema_cmdlist_t *)nema_host_malloc(sizeof(nema_cmdlist_t));
        if (cl == NULL) {
            PRINT_E("Cannot Expand CL! %s:%d\n", __FILE__, __LINE__);
            nema_set_error(NEMA_ERR_CL_EXPANSION);
            return -1;
        }

        if ( ((cmd_no+JUMP_CMD_SIZE)*2*4) > CL_SIZE_BYTES) {
            *cl = nema_cl_create_sized((cmd_no+JUMP_CMD_SIZE)*2*4);
        }
        else {
            *cl = nema_cl_create();
        }

        if (cl->bo.base_virt == NULL) {
            PRINT_E("Cannot Expand CL! %s:%d\n", __FILE__, __LINE__);
            nema_set_error(NEMA_ERR_CL_EXPANSION);
            nema_host_free(cl);
            return -1;
        }

        //inherit root
        cl->root   = bound_cl->root != NULL ? bound_cl->root : bound_cl;
        //inherit flags
        cl->flags  = bound_cl->flags;
        nema_cl_unset_flag(cl, CL_HAS_CHILD);
        nema_cl_unset_flag(cl, CL_FLUSHED);
    }

    nema_cl_set_flag(bound_cl, CL_HAS_CHILD);

    //make bound_cl jump to cl
#ifdef ADDR64BIT
    CL_ADD_CMD(NEMA_CMDADDR_H, (uint32_t)(cl->bo.base_phys >> 32));
#endif
    CL_ADD_CMD(NEMA_CMDADDR, (uint32_t)(cl->bo.base_phys));
    CL_ADD_CMD(NEMA_CMDSIZE, cl->size);

    bound_cl->next = cl;
    nema_cl_bind(cl);

    return 0;
}

static void
cl_submit_(nema_cmdlist_t *cl) {

    if (cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return;
    }

    if (cl->offset == 0) {
        return;
    }

    if ( nema_cl_flag_is_set(cl, CL_CIRCULAR) ) {
        int ccl_size  = (cl->size/4)*2;
       // this circular CL points to the second sub-CL but it's empty
        if (cl->offset == ccl_size) {
            return;
        }
    }

    MUTEX_LOCK
    nema_cl_submit_no_irq(cl);
    nema_rb_submit_cl_id(cl->submission_id);
    MUTEX_UNLOCK

    if (cl->submission_id == SUBMISSION_ID_MASK) {
        //submission_id is going to wrap to zero. Should wait current cl to finish
        (void)nema_cl_wait(cl);
    }
    // else if (cl->submission_id == 0) {
    //     while (completed_cl > (SUBMISSION_ID_MASK/2) ) {
    //         nema_cl_wait(cl);
    //     }
    // } else {

    // }
}

//---------------------------------------------------------------------------
/**
   Adds a command to the bound command list

   @param
   reg the address of the register
   data the data to write on the register
*/
void
nema_cl_add_cmd(uint32_t reg, uint32_t data)
{
    if (bound_cl == NULL) {
        (void)nema_mutex_lock(MUTEX_RB);
        nema_rb_inline_cmd(reg, data);
        (void)nema_mutex_unlock(MUTEX_RB);
        return;
    }

    nema_cl_unset_flag(bound_cl, CL_FLUSHED);

    int space_left = nema_cl_space_left(bound_cl);

    if ( nema_cl_flag_is_set(bound_cl, CL_CIRCULAR) ) {
        //this is a Circular CL
        if ( space_left < 2 ) {
            // Implicit submission
            nema_context.explicit_submission = 0;
            cl_submit_(bound_cl);
        }
    }
    else if ( !nema_cl_flag_is_set(bound_cl, CL_EXPANDABLE) ) {
        if ( (bound_cl->offset+2) > bound_cl->size ) {
            PRINT_E("##### NON EXPANDABLE CMDLIST FULL !!!!!\n");
            nema_set_error(NEMA_ERR_NON_EXPANDABLE_CL_FULL);
            return;
        }
    }
    else if ( (bound_cl->offset+JUMP_CMD_SIZE*2) >= bound_cl->size) {
        int ret = nema_extend_bound_cl(0);

        if (ret < 0) {
            return;
        }
    }
    else {
        //no action required
    }

    CL_ADD_CMD(reg, data);

    CL_ASSERT( (bound_cl->offset+JUMP_CMD_SIZE*2) <= bound_cl->size);
    return;
}

//---------------------------------------------------------------------------
/**
   Adds a command to the bound command list

   @param
   reg the address of the register
   data the data to write on the register
*/
int
nema_cl_add_multiple_cmds(int cmd_no, uint32_t *cmd)
{
    if (bound_cl == NULL) {
        (void)nema_mutex_lock(MUTEX_RB);
        for (int i = 0; i < cmd_no; i++) {
            nema_rb_inline_cmd(cmd[i*2], cmd[(i*2)+1]);
        }
        (void)nema_mutex_unlock(MUTEX_RB);
        nema_set_error(NEMA_ERR_NO_BOUND_CL);

        return 0;
    }

    nema_cl_unset_flag(bound_cl, CL_FLUSHED);

    int space_left = nema_cl_space_left(bound_cl);
    int cmds_that_fit = cmd_no;

    //check if cmdlist has enough space

    // if CL is not expandable
    bool is_expandable = nema_cl_flag_is_set(bound_cl, CL_EXPANDABLE);
    bool is_circular   = nema_cl_flag_is_set(bound_cl, CL_CIRCULAR);

    if ( (!is_expandable) && (!is_circular) ) {
        PRINT_D("Non Expandable\r\n");
        if (space_left < cmd_no) {
            PRINT_E("##### NON EXPANDABLE CMDLIST FULL !!!!!\n");
            nema_set_error(NEMA_ERR_NON_EXPANDABLE_CL_FULL);
            return -1;
        }
    }
    else if ( (space_left-JUMP_CMD_SIZE) < cmd_no) {
            cmds_that_fit = space_left-JUMP_CMD_SIZE;
    }
    else {
        //misra
    }

    int cmd_id = 0;

    PRINT_D("Adding commands that fit\r\n");
    //add cmds that already fit in bound CL
    while (cmd_id < cmds_that_fit) {
        CL_ADD_CMD(cmd[cmd_id*2], cmd[ (cmd_id*2) + 1 ]);
        ++cmd_id;
    }

    //expand CL if needed and add rest of commands
    if (cmd_id < cmd_no) {
        if ( is_circular ) {
            // Implicit submission
            nema_context.explicit_submission = 0;
            cl_submit_(bound_cl);
        } else {
            int ret = nema_extend_bound_cl(cmd_no);

            if (ret < 0) {
                return ret;
            }
        }

        PRINT_D("Adding rest of commands\r\n");
        while (cmd_id < cmd_no) {
            CL_ADD_CMD(cmd[cmd_id*2], cmd[(cmd_id*2)+1]);
            ++cmd_id;
        }
    }

    CL_ASSERT( (bound_cl->offset+JUMP_CMD_SIZE*2) <= bound_cl->size);
    return 0;
}

//---------------------------------------------------------------------------
/**
   Return pointer in bound_cl to directly add commands

   @param
   cmd_no number of commands to write
*/
uint32_t *
nema_cl_get_space(int cmd_no)
{
    if (bound_cl == NULL) {
        PRINT_D("No Bound CL\r\n");
        nema_set_error(NEMA_ERR_NO_BOUND_CL);
        return NULL;
    }

    nema_cl_unset_flag(bound_cl, CL_FLUSHED);

    int space_left = nema_cl_space_left(bound_cl);

    if ( nema_cl_flag_is_set(bound_cl, CL_CIRCULAR) ) {
        //this is a Circular CL
        if ( space_left <= cmd_no+1 ) {
            // Implicit submission
            nema_context.explicit_submission = 0;
            cl_submit_(bound_cl);
            // space_left = nema_cl_space_left(bound_cl);
        }
    }
    //check if cmdlist has enough space
    //always leave enough space for a jump cmd
    else if ( (space_left-JUMP_CMD_SIZE) < cmd_no ) {
        int ret = nema_extend_bound_cl(cmd_no);

        if (ret < 0) {
            return NULL;
        }
    }
    else {
        // misra
    }

    uint32_t *ret_ptr = &(((uint32_t *)bound_cl->bo.base_virt)[bound_cl->offset  ]);
    bound_cl->offset += 2*cmd_no;

    CL_ASSERT(bound_cl->offset <= bound_cl->size);
    return ret_ptr;
}

//---------------------------------------------------------------------------
void
nema_cl_submit_no_irq(nema_cmdlist_t *cl)
{
    if (cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return;
    }

    if (cl->offset == 0) {
        return;
    }

    CL_ASSERT(cl->offset <= cl->size);
    if ( nema_cl_flag_is_set(cl, CL_CIRCULAR) ) {
        int ccl_size  = (cl->size/4)*2;
        int ccl_offset = cl->offset >= ccl_size
                         ? cl->offset - ccl_size
                         : cl->offset;

        // this circular CL points to the second sub-CL but it's empty
        if (cl->offset == ccl_size) {
            return;
        }

        //submit sub-CL
        uintptr_t base_phys = cl->bo.base_phys;
        if (cl->offset >= ccl_size) {
            base_phys += (uintptr_t)(uint32_t)ccl_size*4U;
        }

        nema_cl_flush(cl);
        (void)nema_cl_wait(cl);

        cl->submission_id = nema_rb_submit_cmdlist2(base_phys, ccl_offset);

#if 1
        cl->offset = cl->offset >= ccl_size
                     ? 0
                     : ccl_size;
#else
        nema_rb_submit_cl_id(cl->submission_id);

        (void)nema_cl_wait(cl);
        nema_cl_rewind(cl);
#endif
        return;
    }

    nema_cmdlist_t *cur_cl = cl;

    PRINT_D("------- ROOT CL ------\r\n");

    do {
        uint32_t *cl_base_virt = (uint32_t *) cur_cl->bo.base_virt;
        nema_cmdlist_t *next_cl = cur_cl->next;

        PRINT_D("CL: %10p, BO: 0x%08x, SIZE: %5u, OFF: %5u\r\n", (void *)cur_cl, cur_cl->bo.base_phys, cur_cl->size, cur_cl->offset);

        #ifdef CMDLIST_DEBUG
        for (int32_t i = 0; i < cur_cl->offset; i += 2) {
            PRINT_D("\t\t0x%08x\t0x%08x\r\n", ((uint32_t *)cur_cl->bo.base_virt)[i], ((uint32_t *)cur_cl->bo.base_virt)[i + 1]);
        }
        #endif

        if ( (cur_cl->flags & CL_HAS_CHILD) == 0U) {

            nema_cl_flush(cur_cl);
            break;
        }

        if ( next_cl != NULL ) {
            cl_base_virt[cur_cl->size-1] = (uint32_t)next_cl->offset;
        }

        nema_cl_unset_flag(cur_cl, CL_FLUSHED);

        PRINT_D("| NEXT: 0x%08x, SIZE: %5u\r\n", cl_base_virt[cur_cl->size-3], cl_base_virt[cur_cl->size-1]);

        nema_cl_flush(cur_cl);
        cur_cl = cur_cl->next;
    } while(cur_cl != NULL);

    PRINT_D("\n");

    // (void)nema_mutex_lock(MUTEX_RB);
    cl->submission_id = nema_rb_submit_cmdlist(&cl->bo, cl->offset);
    // (void)nema_mutex_unlock(MUTEX_RB);
}

// static int completed_cl = -1;

void
nema_cl_submit(nema_cmdlist_t *cl) {

    nema_context.explicit_submission = 1;
    cl_submit_(cl);
}

int
nema_cl_wait(nema_cmdlist_t *cl) {

    if (cl == NULL) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return -1;
    }

    if ( cl->submission_id >= 0 ) {
        int ret = nema_wait_irq_cl(cl->submission_id);
        if (ret < 0) {
            return 0;
        }
    }
    cl->submission_id = -1;

    return 0;
}

inline static void nema_cl_jump_or_branch(nema_cmdlist_t *cl, uint32_t flag) {

    if ( (cl == NULL) || (cl->offset == 0) || (bound_cl == NULL) ) {
        nema_set_error(NEMA_ERR_INVALID_CL);
        return;
    }

    //flush the cl we are jumping to
    nema_cmdlist_t *cur_cl = cl;
    do {
        uint32_t *cl_base_virt = (uint32_t *) cur_cl->bo.base_virt;
        nema_cmdlist_t *next_cl = cur_cl->next;

        if ( (cur_cl->flags & CL_HAS_CHILD) == 0U) {
            nema_cl_flush(cur_cl);
            break;
        }

        if ( next_cl!= NULL ) {
            cl_base_virt[cur_cl->size-1] = (uint32_t)next_cl->offset;
        }
        nema_cl_unset_flag(cur_cl, CL_FLUSHED);

        nema_cl_flush(cur_cl);
        cur_cl = cur_cl->next;
    } while(cur_cl != NULL);

    uint32_t *cmd_array;

    // get space for jump command
    // if 64bit address -> we need 3 commands
    // if 32bit address -> we need 2 commands
    cmd_array = (uint32_t *)nema_cl_get_space(JUMP_CMD_SIZE);
    if (cmd_array == (NULL)) {
        return;
    }

    int i = 0;

#ifdef ADDR64BIT
    // set MSBits of address
    cmd_array[i++] = NEMA_CMDADDR_H;
    cmd_array[i++] = (uint32_t)(cl->bo.base_phys >> 32);
#endif
    // set LSBits of address
    cmd_array[i] = NEMA_CMDADDR                ; i++;
    cmd_array[i] = (uint32_t)(cl->bo.base_phys); i++;
    // set Command List size (how full is it)
    cmd_array[i] = NEMA_CMDSIZE | flag; i++;
    cmd_array[i] = (uint32_t)cl->offset;

}
//--------------------------------------------------------------------------
void
nema_cl_branch(nema_cmdlist_t *cl)
{
    nema_cl_jump_or_branch(cl, CL_PUSH);
}

//--------------------------------------------------------------------------
void
nema_cl_jump(nema_cmdlist_t *cl)
{
    nema_cl_jump_or_branch(cl, 0);
}

//---------------------------------------------------------------------------
void
nema_cl_return(void)
{
    if (bound_cl != NULL) {
        nema_cl_add_cmd(CL_NOP | CL_RETURN, 0x00);
    }
}

//---------------------------------------------------------------------------
int
nema_cl_almost_full(nema_cmdlist_t *cl)
{
    int space_left = nema_cl_space_left(cl);

    if ( space_left < CL_ALMOST_FULL_LIMIT ) {
        return 1;
    } else {
        return 0;
    }
}

int nema_cl_enough_space(int cmd_no)
{
    if (bound_cl == NULL) {
        nema_set_error(NEMA_ERR_NO_BOUND_CL);
        return -2;
    }

    bool is_circular   = nema_cl_flag_is_set(bound_cl, CL_CIRCULAR);
    if (is_circular) {
            return 0;
    }

    int space_left = nema_cl_space_left(bound_cl);
    bool is_expandable = nema_cl_flag_is_set(bound_cl, CL_EXPANDABLE);

    // Check if cmdlist has enough space
    // There is enough space
    if (space_left > cmd_no)  {
        return 0;
    }
    else { // There is not enough space. Check if cl can be expanded
        if (is_expandable) {
            int ret = nema_extend_bound_cl(cmd_no);
            if(ret < 0){
                nema_set_error(NEMA_ERR_CL_EXPANSION);
                return -1;
            }
            else{
                return 0;
            }
        }
        else{
            nema_set_error(NEMA_ERR_NON_EXPANDABLE_CL_FULL);
            return -1;
        }
    }
}
