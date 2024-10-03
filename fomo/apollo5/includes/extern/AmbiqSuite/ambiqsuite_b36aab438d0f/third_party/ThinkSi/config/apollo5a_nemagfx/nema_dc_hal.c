/*******************************************************************************
 * Copyright (c) 2022 Think Silicon S.A.
 *
   Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this header file and/or associated documentation files to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Materials, and to permit persons to whom the Materials are furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * NEMAGFX API. THE UNMODIFIED, NORMATIVE VERSIONS OF THINK-SILICON NEMAGFX
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT:
 *   https://think-silicon.com/products/software/nemagfx-api
 *
 *  The software is provided 'as is', without warranty of any kind, express or
 *  implied, including but not limited to the warranties of merchantability,
 *  fitness for a particular purpose and noninfringement. In no event shall
 *  Think Silicon S.A. be liable for any claim, damages or other liability, whether
 *  in an action of contract, tort or otherwise, arising from, out of or in
 *  connection with the software or the use or other dealings in the software.
 ******************************************************************************/

#include "stdint.h"
#include "nema_dc_regs.h"
#include "nema_dc_hal.h"
#include "nema_dc_mipi.h"
#include "nema_dc_intern.h"
#include "nema_dc.h"
#include "nema_dc_dsi.h"
#include "nema_ringbuffer.h"
#include "nema_sys_defs.h"
#include "am_mcu_apollo.h"
#include "nema_math.h"
// #### INTERNAL BEGIN ####
//#define CAY_DSI_WA
// #### INTERNAL END ####
#define CAY_DSI_WA_CYSV_111

#ifdef BAREMETAL

#  ifndef WAIT_IRQ_POLL
#    define WAIT_IRQ_POLL              1
#  endif

#  ifdef WAIT_IRQ_BINARY_SEMAPHORE
#    warning semaphore will not work under baremetal
#    undef WAIT_IRQ_BINARY_SEMAPHORE
#    define WAIT_IRQ_BINARY_SEMAPHORE  0
#  endif

#else  // BAREMETAL

#  ifdef SYSTEM_VIEW
#    include "SEGGER_SYSVIEW_FreeRTOS.h"
#  endif

#  include "FreeRTOS.h"
#  include "task.h"

#  ifndef WAIT_IRQ_POLL
#    define WAIT_IRQ_POLL              0
#  endif

#  ifndef WAIT_IRQ_BINARY_SEMAPHORE
#    define WAIT_IRQ_BINARY_SEMAPHORE  1
#  endif

#  if WAIT_IRQ_BINARY_SEMAPHORE
#    include "semphr.h"
     static SemaphoreHandle_t xSemaphore_vsync = NULL;
     static SemaphoreHandle_t xSemaphore_TE = NULL;
#  endif

#endif // BAREMETAL

#ifndef NEMADC_BASEADDR
#include "apollo5a.h"
#define NEMADC_BASEADDR       DC_BASE
#endif

// IRQ number
#ifndef NEMADC_IRQ
#define NEMADC_IRQ           ((IRQn_Type)29U)
#endif

static uintptr_t nemadc_regs = 0;


//
// declaration of display controller interrupt callback function.
//

nema_dc_interrupt_callback  nemadc_te_cb = NULL;
nema_dc_interrupt_callback  nemadc_vsync_cb = NULL;
static void* vsync_arg;
static bool bNeedLaunchInTe = false;


//
// The arrays for restoring configuration
//
static uint32_t ui32DCRegBlock0[12];    // offset from 0x00 to 0x2C
static uint32_t ui32DCRegBlock1[4];     // offset from 0x1A0 to 0x1AC
static bool bDCRegBackup = false;
//*****************************************************************************
//
//! @brief Backup the key registers
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_backup_registers(void)
{
    ui32DCRegBlock0[NEMADC_REG_MODE/4]          = nemadc_reg_read(NEMADC_REG_MODE);
    ui32DCRegBlock0[NEMADC_REG_CLKCTRL/4]       = nemadc_reg_read(NEMADC_REG_CLKCTRL);
    ui32DCRegBlock0[NEMADC_REG_BGCOLOR/4]       = nemadc_reg_read(NEMADC_REG_BGCOLOR);

    ui32DCRegBlock0[NEMADC_REG_RESXY/4]         = nemadc_reg_read(NEMADC_REG_RESXY);
    ui32DCRegBlock0[NEMADC_REG_FRONTPORCHXY/4]  = nemadc_reg_read(NEMADC_REG_FRONTPORCHXY);
    ui32DCRegBlock0[NEMADC_REG_BLANKINGXY/4]    = nemadc_reg_read(NEMADC_REG_BLANKINGXY);
    ui32DCRegBlock0[NEMADC_REG_BACKPORCHXY/4]   = nemadc_reg_read(NEMADC_REG_BACKPORCHXY);
    ui32DCRegBlock0[NEMADC_REG_STARTXY/4]       = nemadc_reg_read(NEMADC_REG_STARTXY);

    ui32DCRegBlock0[NEMADC_REG_INTERFACE_CFG/4] = nemadc_reg_read(NEMADC_REG_INTERFACE_CFG);
    ui32DCRegBlock0[NEMADC_REG_GPIO/4]          = nemadc_reg_read(NEMADC_REG_GPIO);

    ui32DCRegBlock1[0] = nemadc_reg_read(NEMADC_REG_FORMAT_CTRL);
    ui32DCRegBlock1[1] = nemadc_reg_read(NEMADC_REG_FORMAT_CTRL2);
    ui32DCRegBlock1[2] = nemadc_reg_read(NEMADC_REG_CLKCTRL_CG);
    ui32DCRegBlock1[3] = nemadc_reg_read(NEMADC_REG_FORMAT_CTRL3);
    bDCRegBackup = true;
}
//*****************************************************************************
//
//! @brief Restore the registers after power up
//!
//! this function should be called after nemadc_init() to restore the original
//! configuration.
//!
//! @return true if the registers have been backup,otherwise return false.
//
//*****************************************************************************
bool
nemadc_restore_registers(void)
{
    int resx, fpx, blx, bpx, resy, fpy, bly, bpy;
    
    if (!bDCRegBackup)
    {
        return false;
    }
    
    resx = ui32DCRegBlock0[NEMADC_REG_RESXY/4] >> 16;
    resy = ui32DCRegBlock0[NEMADC_REG_RESXY/4] & 0xFFFF;
    fpx = ui32DCRegBlock0[NEMADC_REG_FRONTPORCHXY/4] >> 16;
    fpy = ui32DCRegBlock0[NEMADC_REG_FRONTPORCHXY/4] & 0xFFFF;
    blx = ui32DCRegBlock0[NEMADC_REG_BLANKINGXY/4] >> 16;
    bly = ui32DCRegBlock0[NEMADC_REG_BLANKINGXY/4] & 0xFFFF;
    bpx = ui32DCRegBlock0[NEMADC_REG_BACKPORCHXY/4] >> 16;
    bpy = ui32DCRegBlock0[NEMADC_REG_BACKPORCHXY/4] & 0xFFFF;
    bpx -= blx;
    bpy -= bly;
    blx -= fpx;
    bly -= fpy;
    fpx -= resx;
    fpy -= resy;

    //
    // An internal used struct will be initialized in function nemadc_timing(), so we have to call this function instead of resoring related registers directly.
    //
    nemadc_timing(resx, fpx, blx, bpx, resy, fpy, bly, bpy);
    
    nemadc_reg_write(NEMADC_REG_MODE, ui32DCRegBlock0[NEMADC_REG_MODE/4]);
    nemadc_reg_write(NEMADC_REG_CLKCTRL, ui32DCRegBlock0[NEMADC_REG_CLKCTRL/4]);
    nemadc_reg_write(NEMADC_REG_BGCOLOR, ui32DCRegBlock0[NEMADC_REG_BGCOLOR/4]);
    nemadc_reg_write(NEMADC_REG_INTERFACE_CFG, ui32DCRegBlock0[NEMADC_REG_INTERFACE_CFG/4]);
    nemadc_reg_write(NEMADC_REG_GPIO, ui32DCRegBlock0[NEMADC_REG_GPIO/4]);

    nemadc_reg_write(NEMADC_REG_FORMAT_CTRL, ui32DCRegBlock1[0]);
    nemadc_reg_write(NEMADC_REG_FORMAT_CTRL2, ui32DCRegBlock1[1]);
    nemadc_reg_write(NEMADC_REG_FORMAT_CTRL3, ui32DCRegBlock1[3]);
    nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, ui32DCRegBlock1[2]);
    return true;
}
//*****************************************************************************
//
//! @brief Register a dedicated TE callback function
//!
//! @param  fnTECallback                - DC TE interrupt callback function
//!
//! This function registers an extra custom callback function for TE interrupt.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_set_te_interrupt_callback(nema_dc_interrupt_callback fnTECallback)
{
    nemadc_te_cb = fnTECallback;
}

//*****************************************************************************
//
//! @brief Register a dedicated Vsync callback function
//!
//! @param  fnVsyncCallback - DC Vsync interrupt callback function
//! @param  arg             - DC Vsync interrupt callback argument
//!
//! This function registers an extra custom callback function for Vsync interrupt.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_set_vsync_interrupt_callback(nema_dc_interrupt_callback fnVsyncCallback, void* arg)
{
    nemadc_vsync_cb = fnVsyncCallback;
    vsync_arg = arg;
}

//*****************************************************************************
//
//! @brief wait NemaDC to become idle
//!
//! @param ui32Mask   - Mask for the status change.
//! @param ui32Value  - Target value for the status change.
//!
//! This function will delay for approximately the given number of microseconds
//! while checking for a status change, exiting when either the given time has
//! expired or the status change is detected.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//*****************************************************************************
static uint32_t
wait_dbi_idle(uint32_t ui32Mask, uint32_t ui32Value)
{
    uint32_t ui32usMaxDelay = 100000;
    uint32_t ui32Status;
// #### INTERNAL BEGIN ####
#ifdef APOLLO5_FPGA
    //
    // The FPGA duration is preferably 96/APOLLO5_FPGA times that of the Silicon
    //
    ui32usMaxDelay *= 96/APOLLO5_FPGA;
#endif
#ifdef DC_DSI_RW_DEBUG
    while ((nemadc_reg_read(NEMADC_REG_STATUS) & ui32Mask) != ui32Value);
    return AM_HAL_STATUS_SUCCESS;
#endif //DC_DSI_RW_DEBUG
// #### INTERNAL END ####
    //
    // wait NemaDC to become idle
    //
    return am_hal_delay_us_status_change(ui32usMaxDelay, (uint32_t)&DC->STATUS, ui32Mask, ui32Value);
}

//*****************************************************************************
//
//! @brief Configure NemaDC.
//!
//! @param  psDCConfig     - NemaDC configuration structure.
//!
//! This function configures NemaDC display interface, output color format,
//! clock settings, TE setting and timing.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_configure(nemadc_initial_config_t *psDCConfig)
{
    uint32_t cfg = 0;
    if(psDCConfig->eInterface == DISP_INTERFACE_DBI || psDCConfig->eInterface == DISP_INTERFACE_DBIDSI)
    {
        //
        // Program NemaDC MIPI interface
        //
        if (psDCConfig->eInterface == DISP_INTERFACE_DBIDSI)
        {
            //
            // Limit the pixel clock frequecy is less than or equal to 96MHz.(this is DC limitation)
            //
            if(CLKGEN->DISPCLKCTRL_b.DISPCLKSEL == CLKGEN_DISPCLKCTRL_DISPCLKSEL_HFRC192)
            {
                //
                // Set the primary divider ratio to 2 to make sure the pixel clock isn't greater than 96MHz and bypass predivider
                //
                nemadc_clkdiv(2, 1, 4, 0);
            }
            else
            {
                //
                // Set the primary divider ratio to 0 or 1,it's bypass the primary divider.the swap feature is invalid in this situation.
                // After this configuration, both pixel_clk and format_clk's frequencies are from pll_clk through predivider if its value is equal to 0 or 1;
                // they are from pll_clk directly when its value is 0 or 1.
                //
                nemadc_clkdiv(1, 1, 4, 0);
            }
            //
            // Disable feature DBI_CLK as pixel_clk(enable format_clk as DBI_CLK sources),This option can be overruled by NEMADC_REG_FORMAT_CTRL2[31:30] bits.
            //
            cfg = MIPICFG_DBI_EN | MIPICFG_RESX | MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN | MIPICFG_EN_STALL | psDCConfig->ui32PixelFormat;

            //
            // Setting the DBIB_CLK clock frequency is the half of the format clock, that is 96MHz.(This is the limitation of the DSI host)
            //
            nemadc_reg_write(NEMADC_REG_FORMAT_CTRL2,0x2U << 30);
        }
        else
        {
            int i32PreDivider;
            float fPLLCLKFreq;
// #### INTERNAL BEGIN ####   
#ifdef APOLLO5_FPGA
            fPLLCLKFreq = APOLLO5_FPGA;
#else  
// #### INTERNAL END ####
            //
            // Calculated the present clock source frequency.
            //
            fPLLCLKFreq = 3 * (2 << CLKGEN->DISPCLKCTRL_b.DISPCLKSEL);
// #### INTERNAL BEGIN ####   
#endif  
// #### INTERNAL END ####
            i32PreDivider = nema_ceil(fPLLCLKFreq / psDCConfig->fCLKMaxFreq);

            //
            // The value of the predivider should be less than 32 on Apollo5A.
            //
            if(i32PreDivider > 31)
            {
                return;
            }
            nemadc_clkdiv( 1, i32PreDivider, 4, 0);

            cfg = MIPICFG_DBI_EN | MIPICFG_RESX | psDCConfig->ui32PixelFormat;
        }
        if (psDCConfig->bTEEnable)
        {
            cfg |= MIPICFG_DIS_TE;
        }
        nemadc_MIPI_CFG_out(cfg);
        //
        // Program NemaDC to transfer a resx*resy region
        //
        psDCConfig->ui32FrontPorchX = 1;
        psDCConfig->ui32BlankingX = 10;
        psDCConfig->ui32BackPorchX = 1;
        psDCConfig->ui32FrontPorchY = 1;
        psDCConfig->ui32BlankingY = 1;
        psDCConfig->ui32BackPorchY = 1;
    }
    else if ((psDCConfig->eInterface == DISP_INTERFACE_QSPI) ||
             (psDCConfig->eInterface == DISP_INTERFACE_DSPI) ||
             (psDCConfig->eInterface == DISP_INTERFACE_SPI4))
    {
        cfg = MIPICFG_SPI_CSX_V | MIPICFG_DBI_EN | MIPICFG_RESX | MIPICFG_SPI4 | psDCConfig->ui32PixelFormat;
        if (psDCConfig->eInterface == DISP_INTERFACE_QSPI)
        {
            cfg |= MIPICFG_QSPI;
        }
        else if (psDCConfig->eInterface == DISP_INTERFACE_DSPI)
        {
            cfg |= MIPICFG_DSPI;
        }
        
        if(psDCConfig->bTEEnable)
        {
            cfg |= MIPICFG_DIS_TE;
        }
        nemadc_MIPI_CFG_out(cfg);

        // Program NemaDC to transfer a resx*resy region
        psDCConfig->ui32FrontPorchX = 1;
        psDCConfig->ui32BlankingX = 1;
        psDCConfig->ui32BackPorchX = 1;
        psDCConfig->ui32FrontPorchY = 1;
        psDCConfig->ui32BlankingY = 1;
        psDCConfig->ui32BackPorchY = 1;
    }
    else if (psDCConfig->eInterface == DISP_INTERFACE_JDI)
    {
        int i32PreDivider;
        float fPLLCLKFreq;
// #### INTERNAL BEGIN ####   
#ifdef APOLLO5_FPGA
        fPLLCLKFreq = APOLLO5_FPGA;
#else  
// #### INTERNAL END ####
        fPLLCLKFreq = 3 * (2 << CLKGEN->DISPCLKCTRL_b.DISPCLKSEL);
// #### INTERNAL BEGIN ####   
#endif  
// #### INTERNAL END ####
        //
        // To reach a higher performance,we should make sure the frequencies of BCK(its frequency is a quarter of format_clk)
        //
        i32PreDivider = nema_ceil(fPLLCLKFreq / (4 * psDCConfig->fHCKBCKMaxFreq));

        //
        // The value of the predivider should be less than 32 on Apollo5A. Please decrease the clock source to get a low BCK frequency.
        //
        if(i32PreDivider > 31)
        {
            return;
        }
        nemadc_clkdiv( 1, i32PreDivider, 4, 0);
        //
        // Configurate JDI interface argurments of VST(GSP),HST(BSP),ENB(GEN) and XRST(INTB).
        //
        nemadc_reg_write(NEMADC_REG_FORMAT_CTRL, 
                        ((psDCConfig->ui32VSTAssertion & 0x3FF) << 3) | 
                        ((psDCConfig->ui32VSTWidth & 0x3FF) << 13) | 
                        ((psDCConfig->ui32HSTOffset & 0x7) <<23) | 
                        ((psDCConfig->ui32HSTWidth & 0x7) <<26));

        nemadc_reg_write(NEMADC_REG_FORMAT_CTRL2, 
                        (psDCConfig->ui32ENBAssertion & 0x3FF) | 
                        ((psDCConfig->ui32ENBHighPulse & 0x3FF) <<10) | 
                        ((psDCConfig->ui32XRSTAssertion & 0x3FF) <<20));

        nemadc_reg_write(NEMADC_REG_FORMAT_CTRL3, psDCConfig->ui32XRSTHighPulse & 0x3FF);
    }
    else if (psDCConfig->eInterface == DISP_INTERFACE_DPI)
    {
        int i32PreDivider;
        float fPLLCLKFreq;
// #### INTERNAL BEGIN ####   
#ifdef APOLLO5_FPGA
        fPLLCLKFreq = APOLLO5_FPGA;
#else  
// #### INTERNAL END ####
        fPLLCLKFreq = 3 * (2 << CLKGEN->DISPCLKCTRL_b.DISPCLKSEL);
// #### INTERNAL BEGIN ####   
#endif  
// #### INTERNAL END ####
        i32PreDivider = nema_ceil(fPLLCLKFreq / psDCConfig->fCLKMaxFreq);

        //
        // The value of the predivider should be less than 32 on Apollo5A.
        //
        if(i32PreDivider > 31)
        {
            return;
        }
        nemadc_clkdiv( 1, i32PreDivider, 4, 0);

        //
        // Configure DPI(RGB) interface color coding, Hsync and Vsync polarity.
        //
        nemadc_reg_write(NEMADC_REG_MODE, NEMADC_NEG_V | NEMADC_NEG_H | psDCConfig->ui32PixelFormat);
    }

    nemadc_timing(psDCConfig->ui16ResX, 
                  psDCConfig->ui32FrontPorchX, 
                  psDCConfig->ui32BlankingX, 
                  psDCConfig->ui32BackPorchX,
                  psDCConfig->eInterface == DISP_INTERFACE_JDI ? psDCConfig->ui16ResY * 2 : psDCConfig->ui16ResY, 
                  psDCConfig->ui32FrontPorchY, 
                  psDCConfig->ui32BlankingY, 
                  psDCConfig->ui32BackPorchY);
    //
    // Enable clock divider.
    //
    if (psDCConfig->eInterface == DISP_INTERFACE_DPI)
    {
        //
        // Invert clock polarity
        //
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_inv | NemaDC_clkctrl_cg_clk_en);
    }
    else
    {
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
    }
}

//*****************************************************************************
//
//! @brief Prepared operations before sending frame
//!
//! @param  bAutoLaunch    - true:launch transfer in DC TE interrupt implicitly.
//!
//! This function configures clock gating, sends MIPI_write_memory_start
//! command before sending frame. If DBIDSI interface is selected, this function
//! also configures HS/LP mode and data/command type of DSI.
//! Note: bLaunchInTE taks effect in the DC TE interrupt handler, which means
//! if GPIO TE is used or TE signal is ignored, setting this parameter to true or false
//! makes no difference, user still need to call nemadc_transfer_frame_launch manually.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_transfer_frame_prepare(bool bAutoLaunch)
{
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    uint32_t ui32MemWrCmd = MIPI_write_memory_start;
    //
    // Check present interface is DBI/DSI or not.
    //
    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        //
        // Bitfields MIPICFG_EXT_CTRL,MIPICFG_BLANKING_EN only configured for the DSI interface.
        //
        if (ui32Cfg & (MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN))
        {
            nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
            //
            // Enable clock divider and bypass layer0~3 bus clock gaters and pixel clock gaters.
            //
            nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, 0xFFFFFFF0U | NemaDC_clkctrl_cg_clk_en);
    
            wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U);
            //
            // Set data/commands command type
            //
            nemadc_dsi_ct((uint32_t)0, // Unused parameter
                          (uint32_t)0, // Unused parameter
                          NemaDC_dcs_datacmd);
// #### INTERNAL BEGIN ####
#ifdef CAY_DSI_WA
            //
            // Set scan-line (DCS) command
            //
            nemadc_MIPI_out(MIPI_DBIB_CMD | MIPI_write_memory_continue | NemaDC_sline_cmd);
            nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
            //
            // Send DCS write_memory_start command
            //
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui32MemWrCmd);
#else
// #### INTERNAL END ####
            nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
            //
            // Send DCS write_memory_start command
            //
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui32MemWrCmd);
            wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U);
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
        }
        else
        {
            nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
            nemadc_MIPI_out(MIPI_DBIB_CMD | NemaDC_wcmd24 | (ui32MemWrCmd << 8));
        }
    }
    else if ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI)) != 0)
    {
        //
        // disable clock gates
        //
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en);
        if ((ui32Cfg & MIPICFG_QSPI) != 0)
        {
            //
            // QSPI interface
            //
            nemadc_MIPI_CFG_out( ui32Cfg | MIPICFG_SPI_HOLD);
            nemadc_MIPI_out( MIPI_DBIB_CMD | MIPI_MASK_QSPI | CMD1_DATA4);
            nemadc_MIPI_out( MIPI_DBIB_CMD | MIPI_MASK_QSPI | MIPI_CMD24 |
                            (ui32MemWrCmd << CMD_OFFSET));
        }
        else if((ui32Cfg & MIPICFG_DSPI) != 0)
        {
            //
            // DSPI interface
            //
            nemadc_MIPI_CFG_out( ui32Cfg & (~MIPICFG_DSPI));
            // Start MIPI Panel Memory Write
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui32MemWrCmd);
            nemadc_MIPI_CFG_out(((ui32Cfg & (~MIPICFG_SPI4)) | MIPICFG_SPI3) | MIPICFG_SPIDC_DQSPI | MIPICFG_SPI_HOLD);
        }
        else
        {
            //
            // SPI4 interface
            //
            nemadc_MIPI_CFG_out( ui32Cfg | MIPICFG_SPI_HOLD);
            // Start MIPI Panel Memory Write
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui32MemWrCmd);
        }
    }
    //
    // turn DC TE interrupt
    //
    if (ui32Cfg & MIPICFG_DIS_TE)
    {
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 3);
    }

    bNeedLaunchInTe = bAutoLaunch;
}

//*****************************************************************************
//
//! @brief Launch frame transfer
//!
//! This function enables DC frame end interrupt and launches frame transfer.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_transfer_frame_launch()
{
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Check present interface is DBI/DSI or not.
    //
    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        //
        // Enable frame end interrupt for DBI/DSI interface.
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4);
        nemadc_set_mode(NEMADC_ONE_FRAME | NEMADC_OUTP_OFF);
    }
    else if (((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX)) != 0))
    {
        //
        // Enable spi frame end interrupt for SPI4/DSPI/QSPI interface.
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 5);
        //
        // Send One Frame
        //
        nemadc_set_mode(NEMADC_ONE_FRAME);
    }
    else if (0x3FF & nemadc_reg_read(NEMADC_REG_FORMAT_CTRL3))
    {
        //
        // Enable frame end interrupt for JDI interface.
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4);
        nemadc_set_mode(NEMADC_ONE_FRAME | NEMADC_JDIMIP | NEMADC_SCANDOUBLE);
    }
    else
    {
        //
        // Enable frame end interrupt for DPI(RGB) interface.
        //
        nemadc_reg_write(NEMADC_REG_INTERRUPT, 1 << 4);

        nemadc_set_mode(NEMADC_ONE_FRAME | nemadc_reg_read(NEMADC_REG_MODE));
    }
}

//*****************************************************************************
//
//! @brief Required operations after completing frame transfer
//!
//! This function sets NemaDC back to normal mode after completing frame
//! transfer. This function should be called in frame end interrupt.
//!
//! @return None.
//
//****************************************************************************
static void
nemadc_transfer_frame_end()
{
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Check present interface is DBI/DSI or not.
    //
    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        //
        // Bitfields MIPICFG_EXT_CTRL,MIPICFG_BLANKING_EN only configured for the DSI interface.
        //
        if (ui32Cfg & (MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN))
        {
// #### INTERNAL BEGIN ####
#ifdef CAY_DSI_WA
            nemadc_MIPI_CFG_out(ui32Cfg & (~MIPICFG_FRC_CSX_0));
#else
// #### INTERNAL END ####
            nemadc_MIPI_CFG_out(ui32Cfg & (~MIPICFG_SPI_HOLD));
// #### INTERNAL BEGIN ####
#endif
// #### INTERNAL END ####
            nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, NemaDC_clkctrl_cg_clk_en); // enable clock gating
            nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x1); // LP
        }
        else
        {
            nemadc_MIPI_CFG_out(ui32Cfg & (~MIPICFG_FRC_CSX_0));
        }
    }
    else if ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI)) != 0)
    {
        nemadc_MIPI_CFG_out((ui32Cfg | MIPICFG_SPI4) & (~(MIPICFG_SPI3 | MIPICFG_SPIDC_DQSPI | MIPICFG_SPI_HOLD)));
        //
        // enable clock gating
        //
        nemadc_reg_write(NEMADC_REG_CLKCTRL_CG, 0);
    }
    else if (0x3FF & nemadc_reg_read(NEMADC_REG_FORMAT_CTRL3))
    {
        //
        // Clear mode register
        //
        nemadc_set_mode(0);
    }
    else
    {
        //
        // Frame end operation for DPI(RGB) interface.
        //
    }

}

//*****************************************************************************
//
//! @brief  DBI write
//!
//! @param  ui8Cmd        - command
//! @param  pui8Para      - pointer of parameters to be sent
//! @param  ui8ParaLen    - length of parameters to be sent
//!
//! This function sends DBI commands to panel.
//!
//! @return None.
//
//****************************************************************************
static void
dbi_write(uint8_t ui8Cmd, uint8_t *pui8Para, uint8_t ui8ParaLen)
{
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
    //
    // the maximum length of write is 8 bytes.(1 command + 7 parameters)
    //
    if (ui8ParaLen > 7)
    {
        ui8ParaLen = 7;
    }

    nemadc_MIPI_out(NemaDC_DBI_cmd | NemaDC_wcmd24 | (ui8Cmd << 8));
    for (uint8_t ui8Index = 0; ui8Index < ui8ParaLen; ui8Index++)
    {
        nemadc_MIPI_out(pui8Para[ui8Index]);
    }

    nemadc_MIPI_CFG_out(ui32Cfg);
}

//*****************************************************************************
//
//! @brief  DBI read
//!
//! @param  ui8Cmd        - command
//! @param  ui8ParaLen    - length of parameters to be read
//! @param  ui32Received  - received data.
//!
//! This function sends DBI commands to panel.
//!
//! @return AM_HAL_STATUS_SUCCESS.
//
//****************************************************************************
static uint32_t
dbi_read(uint8_t ui8Cmd, uint8_t ui8ParaLen, uint32_t *ui32Received)
{
    uint32_t ui32ReadValue,ui32Mask = 0;

    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Force Chip Select line low.
    //
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
    //
    // the maximum length of this kind of read is 4 bytes.
    //
    if (ui8ParaLen > 4)
    {
        ui8ParaLen = 4;
    }

    //
    // Send read command
    //
    nemadc_MIPI_cmd(NemaDC_wcmd24 | (ui8Cmd << 8));
    //
    // wait NemaDC to become idle
    //
    while ((nemadc_reg_read(NEMADC_REG_STATUS) & DC_STATUS_dbi_pending_trans)!=0);
    //
    // trigger dbi-read operation for 8bits,first 8bits is invalid data
    //
    nemadc_reg_write(NEMADC_REG_DBIB_RDAT, (0x0 << 30));

    while (ui8ParaLen-- != 0)
    {
        //
        // trigger dbi-read operation for 8bits
        //
        nemadc_reg_write(NEMADC_REG_DBIB_RDAT, (0x0 << 30));
        ui32Mask <<= 8;
        ui32Mask |= 0xFF;
    }

    //
    // wait NemaDC to become idle
    //
    while ((nemadc_reg_read(NEMADC_REG_STATUS) & DC_STATUS_dbi_pending_trans)!=0);
    nemadc_MIPI_CFG_out(ui32Cfg);

    *ui32Received = nemadc_reg_read(NEMADC_REG_DBIB_RDAT) & ui32Mask;

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief DBI-B interface read the frame buffer
//!
//! @param ui8ReceiveBuffer     - pointer of receive buffer
//! @param ui32ParaLen          - the read count
//!
//! @note During a read cycle the host processor reads data from the display module  
//! via the interface. The Type B interface utilizes D/CX, RDX and WRX signals as  
//! well as all eight (D[7:0]) information signals.RDX is driven from high to low 
//! then allowed to be pulled back to high during the read cycle. The display module 
//! provides information to the host processor during the read cycle while the host 
//! processor reads the display module information on the rising edge of RDX. D/CX 
//! is driven high during the read cycle.
//!
//! @return pointer.
//
//*****************************************************************************
void
dbi_frame_read(uint8_t *ui8ReceiveBuffer, uint32_t ui32ParaLen)
{
// #### INTERNAL BEGIN ####   
#if 1
// #### INTERNAL END ####

    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Force Chip Select line low.
    //
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
    //
    // Send command to read memory.
    //
    nemadc_MIPI_cmd(NemaDC_wcmd24 | (MIPI_read_memory_start << 8));

    while ((nemadc_reg_read(NEMADC_REG_STATUS) & DC_STATUS_dbi_pending_trans)!=0);

    for (uint32_t index = 0; index < ui32ParaLen; index++)
    {
        //
        // trigger dbi-read operation for 8bits
        //
        nemadc_reg_write(NEMADC_REG_DBIB_RDAT, (0x0 << 30));

        while ((nemadc_reg_read(NEMADC_REG_STATUS) & DC_STATUS_dbi_pending_trans)!=0);

        ui8ReceiveBuffer[index] = (uint8_t)(nemadc_reg_read(NEMADC_REG_DBIB_RDAT) & 0xff);
    }

    nemadc_MIPI_CFG_out(ui32Cfg);
// #### INTERNAL BEGIN ####   
#else
    //
    // memory read case 2
    //
    //
    // Send command to read memory and read 8-bit data
    //
    nemadc_MIPI_read(MIPI_read_memory_start, 0);

    for (uint32_t i = 1; i < ui32ParaLen; i++)
    {
        //
        // Send command to read memory and read 8-bit data
        //
        ui8ReceiveBuffer[i] = (uint8_t)nemadc_MIPI_read(MIPI_read_memory_continue, 0);
    }
#endif
// #### INTERNAL END ####
}

//*****************************************************************************
//
//! @brief  Send DSI DCS(Display Command Set) write commands
//!
//! @param  ui8Cmd        - command
//! @param  pui8Para      - pointer of parameters to be sent
//! @param  ui8ParaLen    - length of parameters to be sent
//! @param  bHS           - high speed mode or low power mode (escape mode)
//!
//! This function sends DSI DCS commands.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
static uint32_t
dsi_dcs_write(uint8_t ui8Cmd, uint8_t* pui8Para, uint8_t ui8ParaLen, bool bHS)
{
    uint32_t ui32Cfg = 0;

    if (bHS == true) // high speed mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
    }
    else // low power mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x1);
    }

    nemadc_dsi_ct((uint32_t)0, // Unused parameter
                  (uint32_t)0, // Unused parameter
                   NemaDC_dcs_datacmd);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    //
    // enable command/parameters packing
    //
    ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
    //
    // Download command & parameter to DBI i/f
    //
    nemadc_MIPI_out(NemaDC_DBI_cmd | ui8Cmd);
    for (uint8_t ui8Index = 0; ui8Index < ui8ParaLen; ui8Index++)
    {
        nemadc_MIPI_out(pui8Para[ui8Index]);
    }
    //
    // Send command-parameter packet.
    //
    nemadc_MIPI_CFG_out(ui32Cfg);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Send DSI generic write command.
//!
//! @param  pui8Para        - pointer of parameters to be sent
//! @param  ui8ParaLen      - length of parameters to be sent,it should be greater than zero.
//! @param  bHS             - high speed mode or low power mode (escape mode)
//!
//! This function sends DSI generic commands.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
static uint32_t
dsi_generic_write(uint8_t* pui8Para, uint8_t ui8ParaLen, bool bHS)
{
    uint32_t ui32Cfg = 0;
    nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x6)); // set vc_no to 00.

    if (bHS == true) //high speed mode
    {
// #### INTERNAL BEGIN ####   
        //
        // This is a temporary workaround for Cayenne RevA, For more information please refer to the first issue of Jira CYSV-111
        //
// #### INTERNAL END ####
#ifdef CAY_DSI_WA_CYSV_111
        ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_FRC_CSX_0);
        //
        // Transmit in HS mode
        //
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
        nemadc_MIPI_CFG_out(ui32Cfg & ~MIPICFG_FRC_CSX_0);
#else
        //
        // Transmit in HS mode
        //
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
#endif

    }
    else // low power mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x1);
    }

    nemadc_dsi_ct((uint32_t)0,  // Unused parameter
                  (uint32_t)0,  // Unused parameter
                   NemaDC_ge_datacmd);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    //
    // enable command/parameters packing
    //
    ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
    //
    // Download command & parameter to DBI i/f
    //
    if ( ui8ParaLen == 0 )
    {
        nemadc_MIPI_out(NemaDC_DBI_cmd | 0x00);
    }
    else
    {
        for (uint8_t ui8Index = 0; ui8Index < ui8ParaLen; ui8Index++)
        {
            if ( ui8ParaLen < 3 )
            {
                nemadc_MIPI_out(NemaDC_DBI_cmd | pui8Para[ui8Index]);
            }
            else
            {
                nemadc_MIPI_out(pui8Para[ui8Index]);
            }
        }
    }
    //
    // Send command-parameter packet.
    //
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_pending_cmd, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    nemadc_MIPI_CFG_out(ui32Cfg);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Send SPI4/DSPI/QSPI/DSI command via Display Controller(DC)
//!
//! @param  ui8Command      - command
//! @param  p_ui8Para       - pointer of parameters to be sent
//! @param  ui8ParaLen      - length of parameters to be sent
//! @param  bDCS            - DCS command or generic command
//! @param  bHS             - high speed mode or low power mode (escape mode)
//!
//! This function sends commands to display drive IC.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
uint32_t
nemadc_mipi_cmd_write(uint8_t ui8Command,
                      uint8_t* p_ui8Para,
                      uint8_t ui8ParaLen,
                      bool bDCS,
                      bool bHS)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Check present interface is DBI/DSI or not.
    //
    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        //
        // Bitfields MIPICFG_EXT_CTRL,MIPICFG_BLANKING_EN only configured for the DSI interface.
        //
        if (ui32Cfg & (MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN))
        {
            if (bDCS)
            {
                ui32Status = dsi_dcs_write(ui8Command, p_ui8Para, ui8ParaLen, bHS);
            }
            else
            {
                ui32Status = dsi_generic_write(p_ui8Para, ui8ParaLen, bHS);
            }
        }
        else
        {
            dbi_write(ui8Command, p_ui8Para, ui8ParaLen);
        }
    }
    else if ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI)) != 0)
    {
        //
        // SPI4,DSPI and QSPI interfaces.
        //
        uint32_t ui32Cmd = 0;

        //
        // Return directly if timeout
        //
        if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_pending_cmd, 0x0U))
        {
            return AM_HAL_STATUS_TIMEOUT;
        }

        if ((ui32Cfg & MIPICFG_QSPI) != 0)
        {
            //
            // QSPI interface.
            //
            nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
            ui32Cmd = MIPI_DBIB_CMD | MIPI_MASK_QSPI;
            nemadc_MIPI_out(ui32Cmd | CMD1_DATA1);
            nemadc_MIPI_out(ui32Cmd | MIPI_CMD24 | (ui8Command << CMD_OFFSET));
            //
            // maximum value of ui8ParaLen isn't greater than 18.
            //
            if (ui8ParaLen > 18)
            {
                ui8ParaLen = 18;
            }
        }
        else
        {
            if ((ui32Cfg & MIPICFG_DSPI) != 0)
            {
                //
                // DSPI interface
                //
                nemadc_MIPI_CFG_out(ui32Cfg & (~MIPICFG_DSPI));
            }
            else
            {
                //
                // SPI4 interface
                // 
                nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_SPI_HOLD);
            }
            nemadc_MIPI_out(MIPI_DBIB_CMD | ui8Command);
            //
            // maximum value of ui8ParaLen isn't greater than 21.
            //
            if (ui8ParaLen > 21)
            {
                ui8ParaLen = 21;
            }
        }

        for (uint8_t i = 0; i < ui8ParaLen/3*3; i+= 3 )
        {
            nemadc_MIPI_out(ui32Cmd | MIPI_CMD24 | p_ui8Para[i] << 16 | p_ui8Para[i+1] << 8 | p_ui8Para[i+2]);
        }

        if (ui8ParaLen % 3 == 2)
        {
            nemadc_MIPI_out(ui32Cmd | MIPI_CMD16 | p_ui8Para[ui8ParaLen-2] << 8 | p_ui8Para[ui8ParaLen-1]);
        }
        else if (ui8ParaLen % 3 == 1)
        {
            nemadc_MIPI_out(ui32Cmd | p_ui8Para[ui8ParaLen-1]);
        }
        //
        // Return directly if timeout
        //
        if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_pending_cmd, 0x0U))
        {
            return AM_HAL_STATUS_TIMEOUT;
        }
        nemadc_MIPI_CFG_out(ui32Cfg);
    }
    return ui32Status;
}

//*****************************************************************************
//
//! @brief  mask valid bitfields
//!
//! @param  ui8DataLen    - length of data to be read
//!
//! @return uint32_t.
//
//****************************************************************************
static inline uint32_t
mask_valid(uint8_t ui8DataLen)
{
    uint32_t mask = 0;
    switch ( ui8DataLen )
    {
        case 1 :
            mask = 0xff;
            break;
        case 2 :
            mask = 0xffff;
            break;
        case 3 :
            mask = 0xffffff;
            break;
        case 4 :
            mask = 0xffffffff;
            break;
        default :
            mask = 0xff;
            break;
    }
    return mask;
}

//*****************************************************************************
//
//! @brief  Send DSI DCS(Display Command Set) read commands
//!
//! @param  ui8Cmd        - command
//! @param  ui8DataLen    - length of data to be read,it should be greater than zero.
//! @param  ui32Received  - received data.
//! @param  bHS           - high speed mode or low power mode (escape mode)
//!
//! This function sends DSI DCS read commands.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
static uint32_t
dsi_dcs_read(uint8_t ui8Cmd, uint8_t ui8DataLen, uint32_t* ui32Received, bool bHS)
{
    uint32_t ui32Cfg = 0;
    //
    // set return packet size (bytes)
    //
    am_hal_dsi_set_return_size(ui8DataLen, bHS);

    if (bHS == true) // high speed mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
    }
    else // low power mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x1);
    }

    nemadc_dsi_ct(NemaDC_dt_DCS_read_param_no,
                  NemaDC_dt_DCS_read_param_no,
                  NemaDC_dcs_datacmd);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
    ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
// #### INTERNAL BEGIN ####
    //
    // More information for this software workaround,please refer to the second issue of Jira CYSV-95
    //
// #### INTERNAL END ####
    if(ui8DataLen == 1)
    {
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_EN_DVALID | MIPICFG_SPI_HOLD);

        nemadc_MIPI_cmd(NemaDC_DBI_read | ui8Cmd);
        //
        // Send command-parameter packet.
        //
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_EN_DVALID);
    }
    else
    {
        //
        // Send command-parameter packet.
        //
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_EN_DVALID);

        //
        // Return directly if timeout
        //
        if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
        {
            return AM_HAL_STATUS_TIMEOUT;
        }
        // Force Chip select Low.
        // This will keep CS low from command phase to read data phase.
        nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_EN_DVALID | MIPICFG_FRC_CSX_0);

        nemadc_MIPI_cmd(NemaDC_DBI_read | ui8Cmd);

        for (uint8_t i = 0; i < ui8DataLen - 1; i++)
        {
            //
            // Return directly if timeout
            //
            if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
            {
                return AM_HAL_STATUS_TIMEOUT;
            }
            nemadc_reg_write(NEMADC_REG_INTERFACE_RDAT, 0);
        }
    }
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
    //
    // Release Chip select.
    // This will release CS since read operation has finished Applicable only for DBIB read case.
    //
    nemadc_MIPI_CFG_out(ui32Cfg);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    *ui32Received = nemadc_reg_read(NEMADC_REG_DBIB_RDAT) & mask_valid(ui8DataLen);
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief  Send DSI generic read command.
//!
//! @param  p_ui8Para           - pointer of parameters to be sent
//! @param  ui8ParaLen          - length of parameters to be sent
//! @param  ui8DataLen          - length of data to be read,it should be greater than zero.
//! @param  ui32Received        - received data.
//! @param  bHS                 - high speed mode or low power mode (escape mode)
//!
//! This function sends DSI generic read command.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
static uint32_t
dsi_generic_read(uint8_t *p_ui8Para, uint8_t ui8ParaLen, uint8_t ui8DataLen, uint32_t* ui32Received, bool bHS)
{
    uint32_t ui32Cfg = 0;
    //
    // set return packet size (bytes)
    //
    am_hal_dsi_set_return_size(ui8DataLen, bHS);

    nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x6)); // set vc_no to 00.

    if (bHS == true) // high speed mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) & (~0x1));
    }
    else // low power mode
    {
        nemadc_reg_write(NEMADC_REG_GPIO, nemadc_reg_read(NEMADC_REG_GPIO) | 0x1);
    }

    nemadc_dsi_ct((uint32_t)0,  // Unused parameter
                  (uint32_t)0,  // Unused parameter
                   NemaDC_ge_datacmd);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    //
    // send command
    //
    ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
// #### INTERNAL BEGIN ####
    //
    // The Generic command and accompanying parameter bytes are transferred using bits 0 through 7 of dbi_data_in bus regardless of the interface width.
    //
// #### INTERNAL END ####
    nemadc_MIPI_CFG_out(ui32Cfg | MIPICFG_EN_DVALID | MIPICFG_FRC_CSX_0);
    if (ui8ParaLen == 0) // Generic read, no parameter
    {
        nemadc_MIPI_cmd(NemaDC_DBI_read);
    }
    else
    {
        for (uint8_t i = 0; i < ui8ParaLen; i++)
        {
            nemadc_MIPI_cmd(NemaDC_DBI_read | p_ui8Para[i]);
        }
    }

    if (ui8DataLen == 1) 
    {
        //
        // Return directly if timeout
        //
        if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
        {
            return AM_HAL_STATUS_TIMEOUT;
        }
        nemadc_reg_write(NEMADC_REG_DBIB_RDAT, 0);
    }
    else
    {
        for (uint8_t i = 0; i < ui8DataLen - 1; i++)
        {
            //
            // Return directly if timeout
            //
            if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
            {
                return AM_HAL_STATUS_TIMEOUT;
            }
            nemadc_reg_write(NEMADC_REG_DBIB_RDAT, 0);
        }
    }

    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_rd_wr_on, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }
    nemadc_MIPI_CFG_out(ui32Cfg);
    //
    // Return directly if timeout
    //
    if (AM_HAL_STATUS_TIMEOUT == wait_dbi_idle(DC_STATUS_dbi_busy, 0x0U))
    {
        return AM_HAL_STATUS_TIMEOUT;
    }

    //
    // return read parameters
    //
    *ui32Received = nemadc_reg_read(NEMADC_REG_DBIB_RDAT) & mask_valid(ui8DataLen);
    return AM_HAL_STATUS_SUCCESS;
}

//*****************************************************************************
//
//! @brief Send SPI4/DSPI/QSPI/DSI read command via Display Controller(DC)
//!
//! @param  ui8Command          - command
//! @param  p_ui8Para           - pointer of parameters to be sent
//! @param  ui8ParaLen          - length of parameters to be sent
//! @param  p_ui32Data          - pointer of data to be read
//! @param  ui8DataLen          - length of data to be read (number of bytes)
//! @param  bDCS                - DCS command or generic command
//! @param  bHS                 - high speed mode or low power mode (escape mode)
//!
//! This function sends read commands to display drive IC.
//!
//! @return AM_HAL_STATUS_SUCCESS or AM_HAL_STATUS_TIMEOUT.
//
//****************************************************************************
uint32_t
nemadc_mipi_cmd_read(uint8_t ui8Command,
                     uint8_t* p_ui8Para,
                     uint8_t ui8ParaLen,
                     uint32_t* p_ui32Data,
                     uint8_t ui8DataLen,
                     bool bDCS,
                     bool bHS)
{
    uint32_t ui32Status = AM_HAL_STATUS_SUCCESS;
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Check present interface is DBI/DSI or not.
    //
    if (((ui32Cfg & MIPICFG_DBI_EN) != 0) &&
        ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI | MIPICFG_DSPI_SPIX )) == 0))
    {
        //
        // Bitfields MIPICFG_EXT_CTRL,MIPICFG_BLANKING_EN only configured for the DSI interface.
        //
        if (ui32Cfg & (MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN))
        {
            if (bDCS)
            {
                ui32Status = dsi_dcs_read(ui8Command, ui8DataLen, p_ui32Data, bHS);
            }
            else
            {
                ui32Status = dsi_generic_read(p_ui8Para, ui8ParaLen, ui8DataLen, p_ui32Data, bHS);
            }
        }
        else
        {
            ui32Status = dbi_read(ui8Command, ui8DataLen, p_ui32Data);
        }
    }
    else if ((ui32Cfg & (MIPICFG_SPI3 | MIPICFG_SPI4 | MIPICFG_DSPI | MIPICFG_QSPI)) != 0)
    {   
        //
        // QSPI/DSPI/SPI4 interface.
        //
        return AM_HAL_STATUS_INVALID_OPERATION;
    }
    return ui32Status;
}


//*****************************************************************************
//
//! @brief DC's Vsync interrupt handle function
//!
//! @param  pvUnused            - invalid  parameter
//!
//! this function used to clear Vsync interrupt status bit.and execute callback.
//!
//! @return None.
//
//*****************************************************************************
static void
prvVsyncInterruptHandler(void *pvUnused)
{

    /* Clear the interrupt */
    nemadc_reg_write(NEMADC_REG_INTERRUPT, nemadc_reg_read(NEMADC_REG_INTERRUPT) & (~(3UL << 4)));

    //
    // DC workarounds after transfer is finsihed
    //
    nemadc_transfer_frame_end();

    //
    // vsync interrupt callback function
    //
    if (nemadc_vsync_cb != NULL)
    {
        nemadc_vsync_cb(vsync_arg, 0);
    }
    else
    {
#  if WAIT_IRQ_BINARY_SEMAPHORE
        xSemaphoreGiveFromISR(xSemaphore_vsync, NULL);
#  endif
    }
}
//*****************************************************************************
//
//! @brief DC's TE interrupt handle function
//!
//! @param  pvUnused            - invalid  parameter
//!
//! this function used to clear TE interrupt status bit.and execute callback.
//!
//! @return None.
//
//*****************************************************************************
static void
prvTEInterruptHandler(void *pvUnused)
{
    if (bNeedLaunchInTe)
    {
        nemadc_transfer_frame_launch();
        bNeedLaunchInTe = false;
    }

    /* Clear the interrupt */
    nemadc_reg_write(NEMADC_REG_INTERRUPT, nemadc_reg_read(NEMADC_REG_INTERRUPT) & (~(1U << 3)));

    //
    // TE interrupt callback function
    //
    if (nemadc_te_cb != NULL)
    {
        nemadc_te_cb(NULL, 0);
    }
    else
    {
#if WAIT_IRQ_BINARY_SEMAPHORE
        xSemaphoreGiveFromISR(xSemaphore_TE, NULL);
#endif
    }
}

//*****************************************************************************
//
//! @brief Display Controller interrupt service function.
//!
//! this function will be called automatically when DC's interrupt(s) arrived.
//!
//! @return None.
//
//*****************************************************************************
void
am_disp_isr()
{
#ifdef SYSTEM_VIEW
        traceISR_ENTER();
#endif
    //
    // spi frame end interrupt or frame end interrupt
    //
    if ((nemadc_reg_read(NEMADC_REG_INTERRUPT) & (3UL << 4)) != 0)
    {
        prvVsyncInterruptHandler(NULL);
    }
    if ((nemadc_reg_read(NEMADC_REG_INTERRUPT) & (1UL << 3)) != 0)
    {
        prvTEInterruptHandler(NULL);
    }

#ifdef SYSTEM_VIEW
    traceISR_EXIT();
#endif
}

//*****************************************************************************
//
//! @brief Display Controller initialize function.
//!
//! @return None.
//
//*****************************************************************************
int32_t
nemadc_sys_init(void)
{
    // xil_printf( "nemadc_sys_init()\r\n" );
    nemadc_regs = (uintptr_t)NEMADC_BASEADDR;

    /* Clear the interrupt */
    nemadc_reg_write(NEMADC_REG_INTERRUPT, 0);

    /* Install Interrupt Handler */
    NVIC_SetPriority(NEMADC_IRQ, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(NEMADC_IRQ);

#if WAIT_IRQ_BINARY_SEMAPHORE
    if ((nemadc_vsync_cb == NULL) && (xSemaphore_vsync == NULL))
    {
        xSemaphore_vsync = xSemaphoreCreateBinary();
    }
    if (xSemaphore_TE == NULL)
    {
        xSemaphore_TE = xSemaphoreCreateBinary();
    }
#endif

    return 0;
}

//*****************************************************************************
//
//! @brief Wait for Vsync interrupt.
//!
//! this function invokes to wait Vsync interrupt.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_wait_vsync(void)
{
    /* Wait for the interrupt */
#if WAIT_IRQ_POLL == 1
    uint32_t ui32usMaxDelay = 3000000; // 1 sec
    uint32_t ui32Status;
// #### INTERNAL BEGIN ####
#ifdef APOLLO5_FPGA
    //
    // The FPGA duration is preferably 96/APOLLO5_FPGA times that of the Silicon
    //
    ui32usMaxDelay *= 96/APOLLO5_FPGA;
#endif
// #### INTERNAL END ####
    //
    // waiting TE interrupt bit reset,if it not zero.
    //
    ui32Status = am_hal_delay_us_status_change(50000, (uint32_t)&DC->INTERRUPT, 1UL << 3, 0);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return;
    }
    //irq_handler sets NEMADC_REG_INTERRUPT to 0. Poll until this happens

    //
    // bit 4 is frame end interrupt(JDI,DPI,DBI) or bit 5 is spi frame end interrupt(SPI4,DSPI,QSPI)
    //
    ui32Status = am_hal_delay_us_status_change(ui32usMaxDelay, (uint32_t)&DC->INTERRUPT, 3UL << 4, 0);

    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return;
    }
    uint32_t ui32Cfg = nemadc_reg_read(NEMADC_REG_DBIB_CFG);
    //
    // Check present interface is DSI or not.
    //
    if ((ui32Cfg & MIPICFG_DBI_EN) && (ui32Cfg & (MIPICFG_EXT_CTRL | MIPICFG_BLANKING_EN)))
    {
        //
        // wait lane 0 return stop state
        //
        ui32Status = am_hal_dsi_wait_stop_state(0);
        if (ui32Status != AM_HAL_STATUS_SUCCESS)
        {
            return;
        }
    }

#endif // WAIT_IRQ_POLL

#if WAIT_IRQ_BINARY_SEMAPHORE == 1
    xSemaphoreTake( xSemaphore_vsync, portMAX_DELAY );
#endif
}

//*****************************************************************************
//
//! @brief Wait for TE interrupt.
//!
//! this function invokes to Wait TE interrupt.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_wait_te(void)
{
    /* Wait for the interrupt */
#if WAIT_IRQ_POLL == 1
    uint32_t ui32usMaxDelay = 100000;
    uint32_t ui32Status;
// #### INTERNAL BEGIN ####
#ifdef APOLLO5_FPGA
    //
    // The FPGA duration is preferably 96/APOLLO5_FPGA times that of the Silicon
    //
    ui32usMaxDelay *= 96/APOLLO5_FPGA;
#endif
// #### INTERNAL END ####
    //irq_handler sets NEMADC_REG_INTERRUPT to 0. Poll until this happens
    ui32Status = am_hal_delay_us_status_change(ui32usMaxDelay, (uint32_t)&DC->INTERRUPT, 1UL << 3, 0);
    if (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        return;
    }
#endif // WAIT_IRQ_POLL

#if WAIT_IRQ_BINARY_SEMAPHORE == 1
    xSemaphoreTake( xSemaphore_TE, portMAX_DELAY );
#endif // WAIT_IRQ_BINARY_SEMAPHORE
}

//*****************************************************************************
//
//! @brief Read NemaDC Hardware Register
//!
//! @param  reg            - Register address
//!
//! this function invokes to Read DC register.
//!
//! @return value          - Register Value
//
//*****************************************************************************
uint32_t
nemadc_reg_read(uint32_t reg)
{
    volatile uint32_t *ptr = (volatile uint32_t *)(nemadc_regs + reg);
    return *ptr;
}

//*****************************************************************************
//
//! @brief Write NemaDC Hardware Register
//!
//! @param  reg            - Register address
//! @param  value            - Register Value
//!
//! this function invokes to write DC register.
//!
//! @return None.
//
//*****************************************************************************
void
nemadc_reg_write(uint32_t reg, uint32_t value)
{
    volatile uint32_t *ptr = (volatile uint32_t *)(nemadc_regs + reg);
    *ptr = value;
}

#if 0
//*****************************************************************************
//
//! @brief nemadc_get_vsync
//!
//!
//! @return 32-bit am_hal_status_e status
//! @return AM_HAL_STATUS_IN_USE means NemaDC is still in processing
//! @return AM_HAL_STATUS_SUCCESS means NemaDC is complete all operation
//
//*****************************************************************************
am_hal_status_e
nemadc_get_vsync(void)
{
    if (nemadc_reg_read(NEMADC_REG_INTERRUPT) & 1<<5)
    {
        return AM_HAL_STATUS_IN_USE;
    }
    else
    {
        return AM_HAL_STATUS_SUCCESS;
    }
}

//*****************************************************************************
//
//! @brief nemadc_get_te
//!
//!
//! @return 32-bit am_hal_status_e status
//! @return AM_HAL_STATUS_IN_USE means TE is not arrived
//! @return AM_HAL_STATUS_SUCCESS means TE is arrived
//
//*****************************************************************************
am_hal_status_e
nemadc_get_te(void)
{
    if (nemadc_reg_read(NEMADC_REG_INTERRUPT) & 1<<3)
    {
        return AM_HAL_STATUS_IN_USE;
    }
    else
    {
        return AM_HAL_STATUS_SUCCESS;
    }
}
#endif
