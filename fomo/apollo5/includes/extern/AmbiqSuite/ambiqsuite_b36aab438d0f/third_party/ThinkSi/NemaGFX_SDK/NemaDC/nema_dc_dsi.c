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
#include "nema_dc.h"
#include "nema_dc_regs.h"
#include "nema_dc_mipi.h"
#include "nema_dc_intern.h"

#include "nema_dc_dsi.h"

#include <stdarg.h>
//------------------------------------------------------------------------------
/**
 * Wait NemaDC i/f to become idle
 */
static void wait_NemaDC_dbi_idle(void)
{
    // wait NemaDC to become idle
    //----------------------------------------------------------------
    while((nemadc_reg_read(NEMADC_REG_STATUS)&DC_STATUS_dbi_busy)!=0);
    //----------------------------------------------------------------
}


//------------------------------------------------------------------------------
/**
 * Scanline command and start memory write
 */
void nemadc_dsi_start_frame_transfer(void)
{

    // wait NemaDC idle
    //---------------------
    wait_NemaDC_dbi_idle();
    //---------------------

    // Set data/commands command type
    nemadc_dsi_ct(NemaDC_dt_DCS_long_write,
                     NemaDC_dt_DCS_long_write,
                     NemaDC_dcs_datacmd);

    // Set scan-line (DCS) command
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_write_memory_continue | NemaDC_ext_ctrl | NemaDC_sline_cmd);

    // Send DCS write_memory_start command
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_write_memory_start    | NemaDC_ext_ctrl);

        // wait NemaDC idle
    //---------------------
    wait_NemaDC_dbi_idle();
    //---------------------
}
//------------------------------------------------------------------------------
/**
 * Scanline command and start memory write (generic)
 */
void nemadc_dsi_start_frame_transfer_generic(void)
{

    // wait NemaDC idle
    //---------------------
    wait_NemaDC_dbi_idle();
    //---------------------

    // Set data/commands command type
    nemadc_dsi_ct(NemaDC_dt_generic_long_write,
                     NemaDC_dt_generic_long_write,
                     NemaDC_ge_data);

    // Set scan-line (DCS) command
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_write_memory_continue | NemaDC_ext_ctrl | NemaDC_sline_cmd);

    // Send DCS write_memory_start command
    nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_write_memory_start    | NemaDC_ext_ctrl);

        // wait NemaDC idle
    //---------------------
    wait_NemaDC_dbi_idle();
    //---------------------
}

//------------------------------------------------------------------------------
/**
 * DC DBI interface to DSI
 * @param cmd_type  Command type
 * @param data_type Data (pixel) type
 */
void nemadc_dsi_ct(uint32_t data_type, uint32_t cmd_type, uint32_t type)
{

    // Data CT offset
    const unsigned int dbi_to_dsi_data_offset = (8U);

    // Data/Command CT values
    unsigned int ct_val =  (type|
                            data_type|
                            (cmd_type<<dbi_to_dsi_data_offset));

    // Enable GE/CT signals
    //-----------------------------------------------------
    nemadc_reg_write(NEMADC_REG_FORMAT_CTRL,ct_val);
    //-----------------------------------------------------
}
