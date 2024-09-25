//*****************************************************************************
//
//! @file nemagfx_performance_test.c
//!
//! @brief NemaGFX performace test example.
//!
//! This example collects GPU performance data for a various of common operations,
//! including fill, copy, blend, scale, rotate, scale+rotate, perspective transformation,
//! SVG image rendering , TTF font rendering.
//!
//! We have created different test configuration files in the \src\config folder,
//! see src\config\config_*.h. These tests aim to collect GPU performance data
//! in different sceneries:
//!    - config_base.h: all tests are included.
//!    - config_morton.h: evaluate the rotation performance at different angles
//!                       and Morton, tiling settings.
//!    - config_rotate.h: evaluate rotation performance with different morton,
//!                       tiling, and burst length settings.
//!    - config_scale.h: evaluate scale performance with different morton,
//!                      tiling, and burst length settings.
//!    - config_scale_rotate.h: evaluate affine transformation performance
//!                             with different morton, tiling, and burst length settings.
//!    - config_write_param.h: evaluate GPU write PSRAM performance.
//!
//! How to use:
//! 1. Select the test config file in test_common.c line 54-59.
//! 2. Compile->Download->Collect SWO output->Copy results to a file
//! ->rename this file as *.csv -> open by Excel -> save it as *.xlsx
//!
//!
//! Note: Make sure the PSRAM is connected before running this test.
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

#include "nemagfx_performance_test.h"
#include "am_devices_mspi_psram_aps25616n.h"
#include "display.h"

void            *g_pPSRAMHandle;
void            *g_pMSPIHandle;

am_devices_mspi_psram_config_t MSPI_PSRAM_Config =
{

    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
    .eClockFreq               = AM_HAL_MSPI_CLK_250MHZ,
    .ui32NBTxnBufLength       = 0,
    .pNBTxnBuf                = NULL,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type mspi_interrupts[] =
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
void
psram_mspi_isr(void)
{
    uint32_t ui32Status;

    am_hal_mspi_interrupt_status_get(g_pMSPIHandle, &ui32Status, false);

    am_hal_mspi_interrupt_clear(g_pMSPIHandle, ui32Status);

    am_hal_mspi_interrupt_service(g_pMSPIHandle, ui32Status);
}

extern int32_t nemagfx_performance_test(void);

//*****************************************************************************
//
// Main function
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
    // Enable cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // External power on
    //
    am_bsp_external_pwr_on();
    am_util_delay_ms(100);

#ifdef CPU_RUN_IN_HP_MODE
    //
    //CPU switch to HP mode.
    //
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("CPU Enter HP mode failed!\n");
    }
#endif

    //
    //CPU status display.
    //
    am_hal_pwrctrl_mcu_mode_e cpu_current_status;
    am_hal_pwrctrl_mcu_mode_status(&cpu_current_status);
    if ( AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE != cpu_current_status )
    {
        am_util_stdio_printf("CPU Operate in Normal Mode\n");
    }
    else
    {
        am_util_stdio_printf("CPU Operate in High Performance Mode\n");
    }

#ifndef APOLLO5_FPGA
    //
    // Run MSPI DDR timing scan
    //
#ifdef USE_FIXED_MSPI_TIMING_CONFIG
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    MSPIDdrTimingConfig.ui32Turnaround = 6;
    MSPIDdrTimingConfig.ui32Rxneg = 0;
    MSPIDdrTimingConfig.ui32Rxdqsdelay = 3;
    MSPIDdrTimingConfig.ui32Txdqsdelay = 0;
#else
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_debug_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &MSPI_PSRAM_Config, &MSPIDdrTimingConfig) )
    {
        am_util_debug_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_debug_printf("==== Scan Result: Failed, no valid setting.  \n");
    }
#endif

#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_Config, &g_pPSRAMHandle, &g_pMSPIHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

#ifndef APOLLO5_FPGA
    //
    // Apply DDR timing setting
    //
    ui32Status = am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pPSRAMHandle, &MSPIDdrTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
    }
#endif

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPSRAMHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status)
    {
        am_util_stdio_printf("Enable XIP mode in the MSPI!\n");
    }
    else
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Init GPIO for time measure
    //
#ifdef PIN_MEASURE
    am_hal_gpio_pinconfig(PIN_MEASURE, am_hal_gpio_pincfg_output);
#endif

#ifdef GPU_RUN_IN_HP_MODE
    //
    //GPU switch to HP mode.
    //
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);
    if ( am_hal_pwrctrl_gpu_mode_select(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE)  != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("GPU Enter HP mode failed!\n");
    }
#endif

    //
    //GPU status display.
    //
    am_hal_pwrctrl_gpu_mode_e gpu_current_mode;
    am_hal_pwrctrl_gpu_mode_status(&gpu_current_mode);
    if ( AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE != gpu_current_mode )
    {
        am_util_stdio_printf("GPU Operate in Normal Mode\n");
    }
    else
    {
        am_util_stdio_printf("GPU Operate in High Performance Mode\n");
    }

    //
    //Power on GPU
    //
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    // Initialize NemaGFX
    //
    nema_init();

    //
    // Set GPU WCACHE timeout setting
    // Bit 31: When set to 1, write cache is enabled.
    // Bit 30-16: Timeout for the lines of the WCache.
    //
#ifdef CHANGE_WCACHE_TIMEOUT
    nema_reg_write(0xe4, 0x80640007);
#endif

    //
    // Run the performace test
    //
    nemagfx_performance_test();

    while (1)
    {
    }

}
