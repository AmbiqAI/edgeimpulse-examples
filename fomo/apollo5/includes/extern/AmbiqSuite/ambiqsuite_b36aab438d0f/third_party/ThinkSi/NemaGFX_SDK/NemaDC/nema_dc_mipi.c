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

#include <stdarg.h>

static int qspi_enabled    = 0;
static int dbib_enabled    = 0;
static int ext_ctrl_flag   = 0;

//------------------------------------------------------------------------------
/**
 * Wait NemaDC i/f to become idle
 */
static void nemadc_wait_dbi_idle(void)
{
    // wait NemaDC to become idle
    //----------------------------------------------------------------
    while((nemadc_reg_read(NEMADC_REG_STATUS)&DC_STATUS_dbi_busy)!=0);
    //----------------------------------------------------------------
}

//------------------------------------------------------------------------------
/**
 * Send through (Q)SPI interface DCS commands
 * @param command dcs command
 */
void nemadc_qspi_cmd(int cmd)
{
    uint32_t dbi_cfg = nemadc_reg_read(NEMADC_REG_INTERFACE_CFG);

    nemadc_MIPI_CFG_out(dbi_cfg|MIPICFG_SPI_HOLD);

    if ((cmd != MIPI_write_memory_start) && (cmd != MIPI_write_memory_continue)){
        nemadc_MIPI_out    (NemaDC_DBI_cmd|NemaDC_mask_qspi|CMD1_DATA1);
    } else {
        nemadc_MIPI_out    (NemaDC_DBI_cmd|NemaDC_mask_qspi|CMD1_DATA4);
    }

    nemadc_MIPI_out    (NemaDC_DBI_cmd|NemaDC_mask_qspi|NemaDC_wcmd24|
                                                        (cmd<<CMD_OFFSET));

    if ((cmd != MIPI_write_memory_start) && (cmd != MIPI_write_memory_continue))
    {
        nemadc_MIPI_out    (NemaDC_mask_qspi|0x00);
        nemadc_MIPI_CFG_out(dbi_cfg);
    }

}

//------------------------------------------------------------------------------
/**
 * Send through (Q)SPI interface DCS commands
 * @param command   dcs command
 * @param n_params  number of parameters
 */
void nemadc_qspi_cmd_params(int cmd, int n_params,...)
{
    uint32_t dbi_cfg = nemadc_reg_read(NEMADC_REG_INTERFACE_CFG);
    nemadc_MIPI_CFG_out(dbi_cfg|MIPICFG_SPI_HOLD);
    nemadc_MIPI_out    (NemaDC_DBI_cmd|NemaDC_mask_qspi|CMD1_DATA1);
    nemadc_MIPI_out    (NemaDC_DBI_cmd|NemaDC_mask_qspi|NemaDC_wcmd24|
                                                 (cmd<<CMD_OFFSET));
    // push parameters
    if (n_params > 0)
    {
        unsigned int prm;
        va_list vl;
        va_start(vl,n_params);
        for(int i = 0; i < n_params; i++)
        {
            prm = va_arg(vl,int);
            nemadc_MIPI_out(NemaDC_mask_qspi|prm);
        }
        va_end(vl);
    }
    else
    {
        nemadc_MIPI_out(NemaDC_mask_qspi|0x00);
    }
    nemadc_MIPI_CFG_out(dbi_cfg);
}

//@function nemadc_MIPI_out
//@brief send command or data to MIPI Interface
//@param int cmd desc: command
//@return void desc:void
//-----------------------------------------------------------------------
void nemadc_MIPI_out(int cmd)
{
    while( (nemadc_reg_read(NEMADC_REG_STATUS)&DC_STATUS_dbi_pending_cmd) != 0)  { } //make sure command is idle
    nemadc_reg_write(NEMADC_REG_INTERFACE_CMD, cmd | ext_ctrl_flag);
}

//-----------------------------------------------------------------------
//@function nemadc_MIPI_CFG_out
//@brief send command or data to MIPI Interface
//@param int cfg desc: configuration mode
//@return void desc:void
//-----------------------------------------------------------------------
void nemadc_MIPI_CFG_out(int cfg)
{
    while( (nemadc_reg_read(NEMADC_REG_STATUS)&DC_STATUS_dbi_pending_cmd) != 0)  { } //make sure command is idle
    nemadc_reg_write(NEMADC_REG_INTERFACE_CFG, cfg);

    if ( cfg & (MIPICFG_QSPI | MIPICFG_QSPI_DDR)) {
        qspi_enabled = 1;
        dbib_enabled = 0;
    } else if ((cfg & MIPICFG_DBI_EN) && !(cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) ) {
        qspi_enabled = 0;
        dbib_enabled = 1;
    } else {
        qspi_enabled = 0;
        dbib_enabled = 0;
    }

    if ( cfg & MIPICFG_EXT_CTRL ) {
        ext_ctrl_flag = NemaDC_ext_ctrl;
    } else {
        ext_ctrl_flag = 0;
    }
}
//@function nemadc_MIPI_in
//@brief read data from MIPI Interface
//@param void desc:void
//@return void desc:void
//-----------------------------------------------------------------------
int nemadc_MIPI_in(void)
{
    // wait NemaDC idle
    //---------------------
    nemadc_wait_dbi_idle();
    //---------------------
    return nemadc_reg_read(NEMADC_REG_INTERFACE_RDAT);
}

void nemadc_MIPI_cmd(int cmd) {
    if ( qspi_enabled != 0 ) {
        nemadc_qspi_cmd(cmd);
    }
    else {
        nemadc_MIPI_out(MIPI_DBIB_CMD | cmd);
    }
}

void nemadc_MIPI_cmd_params(int cmd, int n_params,...)
{
    uint32_t dbi_cfg = nemadc_reg_read(NEMADC_REG_INTERFACE_CFG);
    uint32_t mask = 0;
    if ( qspi_enabled != 0 ) {
        nemadc_MIPI_CFG_out(dbi_cfg|MIPICFG_SPI_HOLD);
        nemadc_MIPI_out    (NemaDC_DBI_cmd|NemaDC_mask_qspi|CMD1_DATA1);
        nemadc_MIPI_out    (NemaDC_DBI_cmd|NemaDC_mask_qspi|NemaDC_wcmd24|
                                                     (cmd<<CMD_OFFSET));

        mask = NemaDC_mask_qspi;
    }
    else {
        nemadc_MIPI_cmd(cmd);
    }

    // push parameters
    if (n_params > 0)
    {
        unsigned int prm;
        va_list vl;
        va_start(vl,n_params);
        for(int i = 0; i < n_params; i++)
        {
            prm = va_arg(vl,int);
            nemadc_MIPI_out(mask | prm);
        }
        va_end(vl);
    }


    if ( qspi_enabled != 0 ) {
        if (n_params < 1) {
            nemadc_MIPI_out(NemaDC_mask_qspi|0x00);
        }
        nemadc_MIPI_CFG_out(dbi_cfg);
    }
}

//@function nemadc_MIPI_updateregion
//@brief Does Partial Update in MIPI
//@param int start_x desc: start x coordinate
//@param int start_y desc: start y coordinate
//@param int end_x   desc: end x coordinate
//@param int end_y   desc: end y coordinate
//@param int mode    desc: mode of operation
//@return void desc:void
//-----------------------------------------------------------------------
int nemadc_MIPI_updateregion(int start_x, int start_y,
                             int end_x,   int end_y)
{
    nemadc_reg_write(NEMADC_REG_STARTXY,((unsigned)start_x << 16U)| ((unsigned)start_y & 0xffffU));
    /*nemadc_timing(end_x, 0, 10, 0, end_y, 0, 10, 0);*/
    return 1;
}
//-----------------------------------------------------------------------


void
nemadc_MIPI_enable(void)
{
    nemadc_MIPI_cmd(MIPI_exit_sleep_mode);
    nemadc_MIPI_cmd(MIPI_set_display_on);
}

void
nemadc_MIPI_disable(void)
{
    nemadc_MIPI_cmd(MIPI_set_display_off);
    nemadc_MIPI_cmd(MIPI_enter_sleep_mode);
}

void
nemadc_MIPI_set_pixel_format(int pixel_format)
{
    nemadc_MIPI_cmd_params(MIPI_set_pixel_format, 1, pixel_format);
}

void
nemadc_MIPI_set_position(int minx, int miny, int maxx, int maxy)
{
    nemadc_MIPI_cmd_params( MIPI_set_column_address, 4,
                            (unsigned)minx >>   8U,
                            (unsigned)minx & 0xFFU,
                            (unsigned)maxx >>   8U,
                            (unsigned)maxx & 0xFFU);

    nemadc_MIPI_cmd_params( MIPI_set_page_address, 4,
                            (unsigned)miny   >> 8U,
                            (unsigned)miny & 0xFFU,
                            (unsigned)maxy   >> 8U,
                            (unsigned)maxy & 0xFFU);

}

void
nemadc_MIPI_set_partial_mode(int minx, int miny, int maxx, int maxy)
{
    nemadc_MIPI_cmd_params( MIPI_set_partial_columns, 4,
                            (unsigned)minx >>   8U,
                            (unsigned)minx & 0xFFU,
                            (unsigned)maxx >>   8U,
                            (unsigned)maxx & 0xFFU);

    nemadc_MIPI_cmd_params( MIPI_set_partial_rows, 4,
                            (unsigned)miny >>   8U,
                            (unsigned)miny & 0xFFU,
                            (unsigned)maxy >>   8U,
                            (unsigned)maxy & 0xFFU);


    nemadc_MIPI_cmd(MIPI_enter_partial_mode);
}

void
nemadc_MIPI_start_frame_transfer(void) {
    nemadc_MIPI_cmd(MIPI_write_memory_start);
}

static void
nemadc_MIPI_qspi_read_cmd(int cmd, unsigned int dc_cmd_field) {
    // wait NemaDC idle
    //---------------------
    nemadc_wait_dbi_idle();
    //---------------------

    // Set NemaDC in command hold mode
    //--------------------------------------------------------------------------
    uint32_t dbi_cfg = nemadc_reg_read(NEMADC_REG_INTERFACE_CFG);
    nemadc_reg_write(NEMADC_REG_INTERFACE_CFG, dbi_cfg|MIPICFG_SPI_HOLD);
    //--------------------------------------------------------------------------

    // Load QSPI command
    nemadc_reg_write(NEMADC_REG_INTERFACE_CMD,NemaDC_DBI_cmd|NemaDC_mask_qspi|CMD1_RDAT1);
    nemadc_reg_write(NEMADC_REG_INTERFACE_CMD,NemaDC_DBI_cmd|NemaDC_mask_qspi|NemaDC_wcmd16|cmd);
    nemadc_reg_write(NEMADC_REG_INTERFACE_CMD,NemaDC_DBI_cmd|NemaDC_mask_qspi|NemaDC_DBI_read|dc_cmd_field);
    //--------------------------------------------------------------------------

    // Disable command hold mode (send command)
    nemadc_reg_write(NEMADC_REG_INTERFACE_CFG, dbi_cfg);
}


//------------------------------------------------------------------------------
/**
 * MIPI DBI Type-B read parameters
 * @param  cmd      MIPI DCS command
 * @param  n_params Number of parameters to read (max: 3 In DBIB is limited depending on DBIB read width)
 * @return          Parameters {Param 0,Param 1,Param 2,Param 3}
 */
unsigned nemadc_MIPI_read(int cmd, int n_params)
{
    uint32_t dc_cmd_field, mask;
    uint32_t cfg = nemadc_reg_read(NEMADC_REG_INTERFACE_CFG);

    switch(n_params)
    {
        case 0 : dc_cmd_field = (NemaDC_DBI_read);               mask=0xff;       break;
        case 1 : dc_cmd_field = (NemaDC_DBI_read|NemaDC_rcmd16); mask=0xffff;     break;
        case 2 : dc_cmd_field = (NemaDC_DBI_read|NemaDC_rcmd24); mask=0xffffff;   break;
        case 3 : dc_cmd_field = (NemaDC_DBI_read|NemaDC_rcmd32); mask=0xffffffff; break;
       default : dc_cmd_field = (NemaDC_DBI_read);               mask=0xff;       break;
    }

    //---------------------
    // send command
    //--------------------------------------------------------

    if (qspi_enabled == 1) {

        nemadc_MIPI_qspi_read_cmd(cmd, dc_cmd_field);

    } else if (dbib_enabled == 1) {

        uint32_t rdat_dbib_width, read_bits;
        uint8_t rdx_cycles, max_rdx_cycles, shift_bits;

        // If NemaDC is before NemaDC 22.03.01 release
        if(nemadc_get_ipversion() == 0x00210000U){
        // DBIB read data is limited on the lower 8-bit of DBIB_DBi bus.
            rdat_dbib_width = 0;
        } else{
        // Else choose read DBIB based on configuration of the bus.
            rdat_dbib_width = ( cfg & 0xc0) << 24;
        }

        switch(rdat_dbib_width >> 30)
        {
            case  0 : max_rdx_cycles = 4; read_bits=0xff;   shift_bits =  8; mask = 0; break;
            case  1 : max_rdx_cycles = 3; read_bits=0x1ff;  shift_bits =  9; mask = 0; break;
            case  2 : max_rdx_cycles = 2; read_bits=0xffff; shift_bits = 16; mask = 0; break;
            default : max_rdx_cycles = 4; read_bits=0xff;   shift_bits =  8; mask = 0; break;
        }

        rdx_cycles = ( (n_params+1) > max_rdx_cycles) ? max_rdx_cycles : (n_params+1);


        // Wait for interface to become idle in order to send Read command.
        while((nemadc_reg_read(NEMADC_REG_STATUS)&DC_STATUS_dbi_rd_wr_on)!=0);

        // Force Chip select Low.
        // This will keep CS low from command phase to read data phase.
        nemadc_reg_write(NEMADC_REG_INTERFACE_CFG, cfg | MIPICFG_FRC_CSX_0);


        for (int i = 0; i < rdx_cycles; i++){

            mask <<= shift_bits;
            mask |= read_bits;

            while((nemadc_reg_read(NEMADC_REG_STATUS)&DC_STATUS_dbi_rd_wr_on)!=0);
            //---------------------
            if (i == 0){
                nemadc_MIPI_cmd( (rdat_dbib_width >> 2) | NemaDC_DBI_read | cmd );
            }else {
                nemadc_reg_write(NEMADC_REG_INTERFACE_RDAT, rdat_dbib_width );
            }
        }

    } else {

        nemadc_MIPI_cmd(dc_cmd_field | cmd);

    }
    //--------------------------------------------------------

    // wait NemaDC idle
    //---------------------
    while((nemadc_reg_read(NEMADC_REG_STATUS)&DC_STATUS_dbi_rd_wr_on)!=0);

    // Release Chip select.
    // This will release CS since read operation has finished Applicable only for DBIB read case.
    nemadc_reg_write(NEMADC_REG_INTERFACE_CFG, cfg);

    // return read parameters
    return nemadc_reg_read(NEMADC_REG_INTERFACE_RDAT)&mask;
}
