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
#ifndef NEMA_DEBUGMON_H__
#define NEMA_DEBUGMON_H__

#include "nema_sys_defs.h"

#ifdef __cplusplus
extern "C" {
#endif


uint32_t nema_ocd_read(void);
uint32_t nema_ocd_read_hi(void);
uint32_t nema_ocd_read_lo(void);
void nema_ocd_write(uint32_t addr, uint32_t value);

void nema_ocd_clear(void);
void nema_ocd_start(void);
void nema_ocd_stop(void);
uint32_t nema_ocd_read_counter(uint32_t counter);

#define BIT(b)  ((uint32_t)1U<<(b))

// Counters
//----------------------------------------------------------------------------------------------------------------------

#define NEMA_OCD_TOP        (0U)
#define NEMA_OCD_C0         (1U)
#define NEMA_OCD_C0_IMEM    (2U)
#define NEMA_OCD_C0_SCHED   (3U)
#define NEMA_OCD_C1         (4U)
#define NEMA_OCD_C1_IMEM    (5U)
#define NEMA_OCD_C1_SCHED   (6U)
#define NEMA_OCD_C2         (7U)
#define NEMA_OCD_C2_IMEM    (8U)
#define NEMA_OCD_C2_SCHED   (9U)
#define NEMA_OCD_C3         (10U)
#define NEMA_OCD_C3_IMEM    (11U)
#define NEMA_OCD_C3_SCHED   (12U)
#define NEMA_OCD_MS         (13U)

typedef enum {
    // Top
    //-----------------------------
                               // linting
    NEMA_OCD_C_TOTAL         = /*NEMA_OCD_TOP|*/BIT(6),
    NEMA_OCD_C_BUSY          = /*NEMA_OCD_TOP|*/BIT(7),
    NEMA_OCD_C_BUSY_FBS      = /*NEMA_OCD_TOP|*/BIT(8),
    NEMA_OCD_C_BUSY_SPLIT    = /*NEMA_OCD_TOP|*/BIT(9),
    NEMA_OCD_C_BUSY_RAST     = /*NEMA_OCD_TOP|*/BIT(10), // Activity Monitor Rasterizer
    NEMA_OCD_C_BUSY_CONF     = /*NEMA_OCD_TOP|*/BIT(11), // Activity Monitor ConfRegist
    NEMA_OCD_C_BUSY_CMD      = /*NEMA_OCD_TOP|*/BIT(12),
    NEMA_OCD_E_COMMANDS_RAST = /*NEMA_OCD_TOP|*/BIT(13), // Rasterizer commands
    NEMA_OCD_E_PRW_CONF      = /*NEMA_OCD_TOP|*/BIT(14), // ConfRegs PR Write
    NEMA_OCD_E_PRR_CONF      = /*NEMA_OCD_TOP|*/BIT(15),
    NEMA_OCD_E_CLW_CONF      = /*NEMA_OCD_TOP|*/BIT(16), // ConfRegs CL Write
    NEMA_OCD_E_RFW_CONF      = /*NEMA_OCD_TOP|*/BIT(17), // ConfRegs RF Write
    NEMA_OCD_E_RFR_CONF      = /*NEMA_OCD_TOP|*/BIT(18), // ConfRegs RF Read
    NEMA_OCD_E_RINGTRIG_CMD  = /*NEMA_OCD_TOP|*/BIT(19), // CmdList Processor Ring Trig
    NEMA_OCD_E_WTRIG_CMD     = /*NEMA_OCD_TOP|*/BIT(20), // CmdList Processor W Trig

    // Core 0
    //-----------------------------

    NEMA_OCD_C0_C_BUSY          = NEMA_OCD_C0 | BIT(6),  // Activity Monitor Core
    NEMA_OCD_C0_C_BUSY_PIPE     = NEMA_OCD_C0 | BIT(7),
    NEMA_OCD_C0_C_BUSY_IMEM     = NEMA_OCD_C0 | BIT(8),
    NEMA_OCD_C0_C_BUSY_RF       = NEMA_OCD_C0 | BIT(9),  // Activity Monitor Regs
    NEMA_OCD_C0_C_BUSY_TEX      = NEMA_OCD_C0 | BIT(10), // Activity Monitor TexMap
    NEMA_OCD_C0_E_PIXELS        = NEMA_OCD_C0 | BIT(11),
    NEMA_OCD_C0_E_INSTRUCTIONS  = NEMA_OCD_C0 | BIT(12), // Execution Pipeline Instructions
    NEMA_OCD_C0_E_PREF_TEX      = NEMA_OCD_C0 | BIT(13), // Texture Map Prefetch
    NEMA_OCD_C0_E_READ_TEX      = NEMA_OCD_C0 | BIT(14), // Texture Map Read
    NEMA_OCD_C0_C_STALL_FB      = NEMA_OCD_C0 | BIT(15),
    NEMA_OCD_C0_C_STALL_IMEM    = NEMA_OCD_C0 | BIT(16),
    NEMA_OCD_C0_C_STALL_TEX     = NEMA_OCD_C0 | BIT(17),
    NEMA_OCD_C0_C_STALL_RF_RAST = NEMA_OCD_C0 | BIT(18),
    NEMA_OCD_C0_C_STALL_RF_TEX  = NEMA_OCD_C0 | BIT(19),
    NEMA_OCD_C0_E_READ_RF       = NEMA_OCD_C0 | BIT(20), // Register File RegFile Read
    NEMA_OCD_C0_E_WRITE_RF      = NEMA_OCD_C0 | BIT(21), // Register File RegFile Write
    NEMA_OCD_C0_E_READ_COORD_RF = NEMA_OCD_C0 | BIT(22), // Register File Coord read
    NEMA_OCD_C0_E_WRITE_COORD_RF= NEMA_OCD_C0 | BIT(23), // Register File Coord write
    NEMA_OCD_C0_C_READY         = NEMA_OCD_C0 | BIT(24), // Core Ready


    NEMA_OCD_C0_E_INSTR_R_IMEM       = NEMA_OCD_C0_IMEM | BIT(6), // Instruction Memory Inst read
    NEMA_OCD_C0_E_INSTR_W_IMEM       = NEMA_OCD_C0_IMEM | BIT(7), // Instruction Memory Inst write


    NEMA_OCD_C0_E_THREAD_WAIT_SCHED  = NEMA_OCD_C0_SCHED | BIT(6),  // Scheduler Wait
    NEMA_OCD_C0_E_THREAD_DONE_SCHED  = NEMA_OCD_C0_SCHED | BIT(7),  // Scheduler Yield
    NEMA_OCD_C0_E_INSTR_DONE_SCHED   = NEMA_OCD_C0_SCHED | BIT(8),  // Scheduler Valid
    NEMA_OCD_C0_E_TEX_DONE_SCHED     = NEMA_OCD_C0_SCHED | BIT(9),  // Scheduler Done
    NEMA_OCD_C0_E_FORK_SCHED         = NEMA_OCD_C0_SCHED | BIT(10), // Scheduler Fork
    NEMA_OCD_C0_C_WREADY_SCHED       = NEMA_OCD_C0_SCHED | BIT(11), // Write Ready
    NEMA_OCD_C0_C_WIDLE_SCHED        = NEMA_OCD_C0_SCHED | BIT(12), // Write Idle
    NEMA_OCD_C0_C_RREADY_SCHED       = NEMA_OCD_C0_SCHED | BIT(13), // Read Ready
    NEMA_OCD_C0_C_RIDLE_SCHED        = NEMA_OCD_C0_SCHED | BIT(14), // Read Idle

    // Core 1
    //-----------------------------

    NEMA_OCD_C1_C_BUSY          = NEMA_OCD_C1 | BIT(6),
    NEMA_OCD_C1_C_BUSY_PIPE     = NEMA_OCD_C1 | BIT(7),
    NEMA_OCD_C1_C_BUSY_IMEM     = NEMA_OCD_C1 | BIT(8),
    NEMA_OCD_C1_C_BUSY_RF       = NEMA_OCD_C1 | BIT(9),
    NEMA_OCD_C1_C_BUSY_TEX      = NEMA_OCD_C1 | BIT(10),
    NEMA_OCD_C1_E_PIXELS        = NEMA_OCD_C1 | BIT(11),
    NEMA_OCD_C1_E_INSTRUCTIONS  = NEMA_OCD_C1 | BIT(12),
    NEMA_OCD_C1_E_PREF_TEX      = NEMA_OCD_C1 | BIT(13),
    NEMA_OCD_C1_E_READ_TEX      = NEMA_OCD_C1 | BIT(14),
    NEMA_OCD_C1_C_STALL_FB      = NEMA_OCD_C1 | BIT(15),
    NEMA_OCD_C1_C_STALL_IMEM    = NEMA_OCD_C1 | BIT(16),
    NEMA_OCD_C1_C_STALL_TEX     = NEMA_OCD_C1 | BIT(17),
    NEMA_OCD_C1_C_STALL_RF_RAST = NEMA_OCD_C1 | BIT(18),
    NEMA_OCD_C1_C_STALL_RF_TEX  = NEMA_OCD_C1 | BIT(19),
    NEMA_OCD_C1_E_READ_RF       = NEMA_OCD_C1 | BIT(20),
    NEMA_OCD_C1_E_WRITE_RF      = NEMA_OCD_C1 | BIT(21),
    NEMA_OCD_C1_E_READ_COORD_RF = NEMA_OCD_C1 | BIT(22),
    NEMA_OCD_C1_E_WRITE_COORD_RF= NEMA_OCD_C1 | BIT(23),
    NEMA_OCD_C1_C_READY         = NEMA_OCD_C1 | BIT(24),


    NEMA_OCD_C1_E_INSTR_R_IMEM       = NEMA_OCD_C1_IMEM | BIT(6),
    NEMA_OCD_C1_E_INSTR_W_IMEM       = NEMA_OCD_C1_IMEM | BIT(7),


    NEMA_OCD_C1_E_THREAD_WAIT_SCHED  = NEMA_OCD_C1_SCHED | BIT(6),
    NEMA_OCD_C1_E_THREAD_DONE_SCHED  = NEMA_OCD_C1_SCHED | BIT(7),
    NEMA_OCD_C1_E_INSTR_DONE_SCHED   = NEMA_OCD_C1_SCHED | BIT(8),
    NEMA_OCD_C1_E_TEX_DONE_SCHED     = NEMA_OCD_C1_SCHED | BIT(9),
    NEMA_OCD_C1_E_FORK_SCHED         = NEMA_OCD_C1_SCHED | BIT(10),
    NEMA_OCD_C1_C_WREADY_SCHED       = NEMA_OCD_C1_SCHED | BIT(11),
    NEMA_OCD_C1_C_WIDLE_SCHED        = NEMA_OCD_C1_SCHED | BIT(12),
    NEMA_OCD_C1_C_RREADY_SCHED       = NEMA_OCD_C1_SCHED | BIT(13),
    NEMA_OCD_C1_C_RIDLE_SCHED        = NEMA_OCD_C1_SCHED | BIT(14),

    // Core 2
    //-----------------------------

    NEMA_OCD_C2_C_BUSY          = NEMA_OCD_C2 | BIT(6),
    NEMA_OCD_C2_C_BUSY_PIPE     = NEMA_OCD_C2 | BIT(7),
    NEMA_OCD_C2_C_BUSY_IMEM     = NEMA_OCD_C2 | BIT(8),
    NEMA_OCD_C2_C_BUSY_RF       = NEMA_OCD_C2 | BIT(9),
    NEMA_OCD_C2_C_BUSY_TEX      = NEMA_OCD_C2 | BIT(10),
    NEMA_OCD_C2_E_PIXELS        = NEMA_OCD_C2 | BIT(11),
    NEMA_OCD_C2_E_INSTRUCTIONS  = NEMA_OCD_C2 | BIT(12),
    NEMA_OCD_C2_E_PREF_TEX      = NEMA_OCD_C2 | BIT(13),
    NEMA_OCD_C2_E_READ_TEX      = NEMA_OCD_C2 | BIT(14),
    NEMA_OCD_C2_C_STALL_FB      = NEMA_OCD_C2 | BIT(15),
    NEMA_OCD_C2_C_STALL_IMEM    = NEMA_OCD_C2 | BIT(16),
    NEMA_OCD_C2_C_STALL_TEX     = NEMA_OCD_C2 | BIT(17),
    NEMA_OCD_C2_C_STALL_RF_RAST = NEMA_OCD_C2 | BIT(18),
    NEMA_OCD_C2_C_STALL_RF_TEX  = NEMA_OCD_C2 | BIT(19),
    NEMA_OCD_C2_E_READ_RF       = NEMA_OCD_C2 | BIT(20),
    NEMA_OCD_C2_E_WRITE_RF      = NEMA_OCD_C2 | BIT(21),
    NEMA_OCD_C2_E_READ_COORD_RF = NEMA_OCD_C2 | BIT(22),
    NEMA_OCD_C2_E_WRITE_COORD_RF= NEMA_OCD_C2 | BIT(23),
    NEMA_OCD_C2_C_READY         = NEMA_OCD_C2 | BIT(24),


    NEMA_OCD_C2_E_INSTR_R_IMEM       = NEMA_OCD_C2_IMEM | BIT(6),
    NEMA_OCD_C2_E_INSTR_W_IMEM       = NEMA_OCD_C2_IMEM | BIT(7),


    NEMA_OCD_C2_E_THREAD_WAIT_SCHED  = NEMA_OCD_C2_SCHED | BIT(6),
    NEMA_OCD_C2_E_THREAD_DONE_SCHED  = NEMA_OCD_C2_SCHED | BIT(7),
    NEMA_OCD_C2_E_INSTR_DONE_SCHED   = NEMA_OCD_C2_SCHED | BIT(8),
    NEMA_OCD_C2_E_TEX_DONE_SCHED     = NEMA_OCD_C2_SCHED | BIT(9),
    NEMA_OCD_C2_E_FORK_SCHED         = NEMA_OCD_C2_SCHED | BIT(10),
    NEMA_OCD_C2_C_WREADY_SCHED       = NEMA_OCD_C2_SCHED | BIT(11),
    NEMA_OCD_C2_C_WIDLE_SCHED        = NEMA_OCD_C2_SCHED | BIT(12),
    NEMA_OCD_C2_C_RREADY_SCHED       = NEMA_OCD_C2_SCHED | BIT(13),
    NEMA_OCD_C2_C_RIDLE_SCHED        = NEMA_OCD_C2_SCHED | BIT(14),

    // Core 3
    //-----------------------------

    NEMA_OCD_C3_C_BUSY          = NEMA_OCD_C3 | BIT(6),
    NEMA_OCD_C3_C_BUSY_PIPE     = NEMA_OCD_C3 | BIT(7),
    NEMA_OCD_C3_C_BUSY_IMEM     = NEMA_OCD_C3 | BIT(8),
    NEMA_OCD_C3_C_BUSY_RF       = NEMA_OCD_C3 | BIT(9),
    NEMA_OCD_C3_C_BUSY_TEX      = NEMA_OCD_C3 | BIT(10),
    NEMA_OCD_C3_E_PIXELS        = NEMA_OCD_C3 | BIT(11),
    NEMA_OCD_C3_E_INSTRUCTIONS  = NEMA_OCD_C3 | BIT(12),
    NEMA_OCD_C3_E_PREF_TEX      = NEMA_OCD_C3 | BIT(13),
    NEMA_OCD_C3_E_READ_TEX      = NEMA_OCD_C3 | BIT(14),
    NEMA_OCD_C3_C_STALL_FB      = NEMA_OCD_C3 | BIT(15),
    NEMA_OCD_C3_C_STALL_IMEM    = NEMA_OCD_C3 | BIT(16),
    NEMA_OCD_C3_C_STALL_TEX     = NEMA_OCD_C3 | BIT(17),
    NEMA_OCD_C3_C_STALL_RF_RAST = NEMA_OCD_C3 | BIT(18),
    NEMA_OCD_C3_C_STALL_RF_TEX  = NEMA_OCD_C3 | BIT(19),
    NEMA_OCD_C3_E_READ_RF       = NEMA_OCD_C3 | BIT(20),
    NEMA_OCD_C3_E_WRITE_RF      = NEMA_OCD_C3 | BIT(21),
    NEMA_OCD_C3_E_READ_COORD_RF = NEMA_OCD_C3 | BIT(22),
    NEMA_OCD_C3_E_WRITE_COORD_RF= NEMA_OCD_C3 | BIT(23),
    NEMA_OCD_C3_C_READY         = NEMA_OCD_C3 | BIT(24),


    NEMA_OCD_C3_E_INSTR_R_IMEM       = NEMA_OCD_C3_IMEM | BIT(6),
    NEMA_OCD_C3_E_INSTR_W_IMEM       = NEMA_OCD_C3_IMEM | BIT(7),


    NEMA_OCD_C3_E_THREAD_WAIT_SCHED  = NEMA_OCD_C3_SCHED | BIT(6),
    NEMA_OCD_C3_E_THREAD_DONE_SCHED  = NEMA_OCD_C3_SCHED | BIT(7),
    NEMA_OCD_C3_E_INSTR_DONE_SCHED   = NEMA_OCD_C3_SCHED | BIT(8),
    NEMA_OCD_C3_E_TEX_DONE_SCHED     = NEMA_OCD_C3_SCHED | BIT(9),
    NEMA_OCD_C3_E_FORK_SCHED         = NEMA_OCD_C3_SCHED | BIT(10),
    NEMA_OCD_C3_C_WREADY_SCHED       = NEMA_OCD_C3_SCHED | BIT(11),
    NEMA_OCD_C3_C_WIDLE_SCHED        = NEMA_OCD_C3_SCHED | BIT(12),
    NEMA_OCD_C3_C_RREADY_SCHED       = NEMA_OCD_C3_SCHED | BIT(13),
    NEMA_OCD_C3_C_RIDLE_SCHED        = NEMA_OCD_C3_SCHED | BIT(14),

    // Memory System
    //-----------------------------

    NEMA_OCD_MS_C_BUSY          = NEMA_OCD_MS | BIT(6),  // Memory System Busy
    NEMA_OCD_MS_C_BUSY_CMD      = NEMA_OCD_MS | BIT(7),  // Memory System Busy cmd
    NEMA_OCD_MS_C_BUSY_FW0      = NEMA_OCD_MS | BIT(8),  // Memory System Busy fw
    NEMA_OCD_MS_C_BUSY_TR01     = NEMA_OCD_MS | BIT(9),  // Memory System Busy cr1
    NEMA_OCD_MS_C_BUSY_TR00     = NEMA_OCD_MS | BIT(10), // Memory System Busy cr0
    NEMA_OCD_MS_C_BUSY_FW1      = NEMA_OCD_MS | BIT(11),
    NEMA_OCD_MS_C_BUSY_TR11     = NEMA_OCD_MS | BIT(12),
    NEMA_OCD_MS_C_BUSY_TR10     = NEMA_OCD_MS | BIT(13),
    NEMA_OCD_MS_C_BUSY_FW2      = NEMA_OCD_MS | BIT(14),
    NEMA_OCD_MS_C_BUSY_TR21     = NEMA_OCD_MS | BIT(15),
    NEMA_OCD_MS_C_BUSY_TR20     = NEMA_OCD_MS | BIT(16),
    NEMA_OCD_MS_C_BUSY_FW3      = NEMA_OCD_MS | BIT(17),
    NEMA_OCD_MS_C_BUSY_TR31     = NEMA_OCD_MS | BIT(18),
    NEMA_OCD_MS_C_BUSY_TR30     = NEMA_OCD_MS | BIT(19),
    NEMA_OCD_MS_E_AR_M0         = NEMA_OCD_MS | BIT(20),
    NEMA_OCD_MS_E_R_M0          = NEMA_OCD_MS | BIT(21),
    NEMA_OCD_MS_E_AR_M1         = NEMA_OCD_MS | BIT(22),
    NEMA_OCD_MS_E_R_M1          = NEMA_OCD_MS | BIT(23),
    NEMA_OCD_MS_E_AW_M0         = NEMA_OCD_MS | BIT(24),
    NEMA_OCD_MS_E_W_M0          = NEMA_OCD_MS | BIT(25),
    NEMA_OCD_MS_E_AW_M1         = NEMA_OCD_MS | BIT(26),
    NEMA_OCD_MS_E_W_M1          = NEMA_OCD_MS | BIT(27),
    NEMA_OCD_MS_E_AR_CL         = NEMA_OCD_MS | BIT(28),
    NEMA_OCD_MS_E_R_CL          = NEMA_OCD_MS | BIT(29)

} nemaocd_counters;

// Counters
//-----------------------------------------------------------------------------------------------------------------------

#define NEMA_OCD_START     (0x01U)
#define NEMA_OCD_STOP      (0x02U)
#define NEMA_OCD_CLEAR     (0x04U)
#define NEMA_OCD_MAP       (0x08U)
#define NEMA_OCD_MAXVAL    (0x10U)
#define NEMA_OCD_EXPTIME   (0x00U)
#define NEMA_OCD_CLEARIRQS (0x03U)

#ifdef __cplusplus
}
#endif

#endif
