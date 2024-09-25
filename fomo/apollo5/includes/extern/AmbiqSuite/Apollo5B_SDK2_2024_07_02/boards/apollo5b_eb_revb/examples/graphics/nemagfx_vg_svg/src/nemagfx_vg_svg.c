//*****************************************************************************
//
//! @file nemagfx_vg_svg.c
//!
//! @brief NemaGFX example.
//
//*****************************************************************************
//*****************************************************************************
//
// Copyright (c) 2024, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_a5b_sdk2-748191cd0 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "nemagfx_vg_svg.h"
#include "nemagfx_buffer_customize.h"

//*****************************************************************************
//
// Macro define
//
//*****************************************************************************
#ifndef NON_CACHEABLE_CL_RB_HEAP
    //
    // Make the Command list and ring buffer heap non-cacheable to workaround 
    // an issue of Apollo5a which may result in SVG format image rendering abnormal.
    //
    #ifdef AM_PART_APOLLO5A
        #define NON_CACHEABLE_CL_RB_HEAP 1
    #else
        #define NON_CACHEABLE_CL_RB_HEAP 0
    #endif
#endif

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//static uint32_t        ui32DMATCBBuffer[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = 0,
    .pNBTxnBuf                = NULL,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type MspiInterrupts[] =
{
    MSPI0_IRQn,
    MSPI1_IRQn,
    MSPI2_IRQn,
};

//
// Take over the interrupt handler for whichever MSPI we're using.
//
#define psram_mspi_isr                                                          \
    am_mspi_isr1(MSPI_PSRAM_MODULE)
#define am_mspi_isr1(n)                                                        \
    am_mspi_isr(n)
#define am_mspi_isr(n)                                                         \
    am_mspi ## n ## _isr

//*****************************************************************************
//
// MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
   uint32_t      ui32Status;

#ifdef SYSTEM_VIEW
   traceISR_ENTER();
#endif

   am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);

#ifdef SYSTEM_VIEW
   traceISR_EXIT();
#endif
}

//*****************************************************************************
//
// Customize graphics heap.
//
//*****************************************************************************

#ifdef NEMA_USE_CUSTOM_MALLOC
    #define GRAPHIC_HEAP_SIZE 0x200000UL

    AM_SHARED_RW uint8_t graphic_heap_ssram[GRAPHIC_HEAP_SIZE];
#endif


//*****************************************************************************
//
// Setup MPU regionsm.
//
//*****************************************************************************

#if (NON_CACHEABLE_CL_RB_HEAP==1) && defined(NEMA_USE_CUSTOM_MALLOC)

#define MPU_REGION_NUMBER 6
#define MPU_ATTRIBUTE_ENTRY 0

/*
 * Set the graphics heap to noncacheable,
 * the GPU command list buffers and global ring buffer will be allocated from this heap.
 */
void set_graphic_heap_noncacheable(void)
{

    am_hal_mpu_region_config_t sMPUCfg = 
    { 
        .ui32RegionNumber = MPU_REGION_NUMBER,
        .ui32BaseAddress = 0x20080000U,
        .eShareable = NON_SHARE,
        .eAccessPermission = RW_NONPRIV,
        .bExecuteNever = true,
        .ui32LimitAddress = 0,
        .ui32AttrIndex = MPU_ATTRIBUTE_ENTRY,
        .bEnable = true,
    };

    //MPU non-cacheable attribute setting.
    //OuterAttr and InnerAttr is set to 0b0100 to follow the requirement in ArmV8-m Architecture Reference Manual
    am_hal_mpu_attr_t sMPUAttr = 
    {   
        .ui8AttrIndex = MPU_ATTRIBUTE_ENTRY,
        .bNormalMem = true,
        .sOuterAttr = {
                        .bNonTransient = false, 
                        .bWriteBack = true, 
                        .bReadAllocate = false, 
                        .bWriteAllocate = false
                      },
        .sInnerAttr = {
                        .bNonTransient = false, 
                        .bWriteBack = true, 
                        .bReadAllocate = false, 
                        .bWriteAllocate = false
                      },
        .eDeviceAttr = 0,
    };

    //
    // Set up the attributes.
    //
    am_hal_mpu_attr_configure(&sMPUAttr, 1);
    //
    // Clear the MPU regions.
    //
    am_hal_mpu_region_clear();

    //
    // Set up the regions.
    //
    sMPUCfg.ui32BaseAddress = (uintptr_t)graphic_heap_ssram;
    sMPUCfg.ui32LimitAddress = (uintptr_t)graphic_heap_ssram + GRAPHIC_HEAP_SIZE - 1;
    am_hal_mpu_region_configure(&sMPUCfg, 1);

    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF. 
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);
}

#endif

bool graphic_heap_region_need_flush(uint32_t region_start, uint32_t region_size)
{
#if NON_CACHEABLE_CL_RB_HEAP==1
    if((region_start >= (uint32_t)graphic_heap_ssram) && 
       (region_start < (uint32_t)graphic_heap_ssram + GRAPHIC_HEAP_SIZE))
    {
        return false;
    }
    else
#endif
    {
        return true;
    }
}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    uint32_t ui32Status;

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Configure the SEGGER SystemView Interface.
    //
#ifdef SYSTEM_VIEW
    SEGGER_SYSVIEW_Conf();
#endif

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("nemagfx_vg_svg Example\n");

#ifdef RUN_IN_HP_MODE
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("HP_LP:Enter HP mode failed!\n");
    }
#endif

    //
    // Set the memory region used for graphics buffer to non-cacheable.
    //
#if defined(NEMA_USE_CUSTOM_MALLOC) && (NON_CACHEABLE_CL_RB_HEAP==1)
    set_graphic_heap_noncacheable();
#endif  

    //
    // Customize graphics .
    //
#ifdef NEMA_USE_CUSTOM_MALLOC
    graphic_heap_init((void*)graphic_heap_ssram, GRAPHIC_HEAP_SIZE);
#endif


    //
    // Run the application.
    //
    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

