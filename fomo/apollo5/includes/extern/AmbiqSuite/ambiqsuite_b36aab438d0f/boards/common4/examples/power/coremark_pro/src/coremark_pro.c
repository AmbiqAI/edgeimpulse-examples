//*****************************************************************************
//
//! @file coremark_pro.c
//!
//! @brief EEMBC COREMARK PRO test.
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup ble_freertos_fit_lp BLE FreeRTOS Fit Lowpower Example
//! @ingroup power_examples
//! @{
//!
//! Purpose: This example runs the official EEMBC COREMARK PRO test.
//!
//! CoreMark®-PRO is a comprehensive, advanced processor benchmark that works 
//! with and enhances the market-proven industry-standard EEMBC CoreMark® benchmark. 
//! While CoreMark stresses the CPU pipeline, CoreMark-PRO tests the entire 
//! processor, adding comprehensive support for multicore technology, a combination 
//! of integer and floating-point workloads, and data sets for utilizing larger 
//! memory subsystems. Together, EEMBC CoreMark and CoreMark-PRO provide a standard 
//! benchmark covering the spectrum from low-end microcontrollers to high-performance 
//! computing processors.
//!
//! The EEMBC CoreMark-PRO benchmark contains five prevalent integer workloads and 
//! four popular floating-point workloads. 
//!
//! The integer workloads include:
//!
//! * JPEG compression
//! * ZIP compression
//! * XML parsing
//! * SHA-256 Secure Hash Algorithm
//! * A more memory-intensive version of the original CoreMark
//!
//! The floating-point workloads include:
//!
//! * Radix-2 Fast Fourier Transform (FFT)
//! * Gaussian elimination with partial pivoting derived from LINPACK
//! * A simple neural-net
//! * A greatly improved version of the Livermore loops benchmark using the following 24 FORTRAN 
//!   kernels converted to C (all of these reported as a single score of the `loops.c` workload). 
//!   The standard Livermore loops include:
//!   *   Kernel 1 -- hydro fragment
//!   *   Kernel 2 -- ICCG excerpt (Incomplete Cholesky Conjugate Gradient)
//!   *   Kernel 3 -- inner product
//!   *   Kernel 4 -- banded linear equations
//!   *   Kernel 5 -- tri-diagonal elimination, below diagonal
//!   *   Kernel 6 -- general linear recurrence equations
//!   *   Kernel 7 -- equation of state fragment
//!   *   Kernel 8 -- ADI integration
//!   *   Kernel 9 -- integrate predictors
//!   *   Kernel 10 -- difference predictors
//!   *   Kernel 11 -- first sum
//!   *   Kernel 12 -- first difference
//!   *   Kernel 13 -- 2-D PIC (Particle In Cell)
//!   *   Kernel 14 -- 1-D PIC (pticle In Cell)
//!   *   Kernel 15 -- Casual Fortran.
//!   *   Kernel 16 -- Monte Carlo search loop
//!   *   Kernel 17 -- implicit, conditional computation
//!   *   Kernel 18 -- 2-D explicit hydrodynamics fragment
//!   *   Kernel 19 -- general linear recurrence equations
//!   *   Kernel 20 -- Discrete ordinates transport, conditional recurrence on xx
//!   *   Kernel 21 -- matrix*matrix product
//!   *   Kernel 22 -- Planckian distribution
//!   *   Kernel 23 -- 2-D implicit hydrodynamics fragment
//!   *   Kernel 24 -- find location of first minimum in array
//!
//! More info may be found at the [EEMBC CoreMark-PRO website](https://www.eembc.org/coremark-pro/).
//!
//! Additional Information:
//! To enable debug printing, add the following project-level macro definitions.
//!
//! AM_DEBUG_PRINTF
//!
//! When defined, debug messages will be sent over ITM/SWO at 1M Baud.
//!
//! Note that when these macros are defined, the device will never achieve deep
//! sleep, only normal sleep, due to the ITM (and thus the HFRC) being enabled.
//
//*****************************************************************************
//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "coremark_pro.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define AM_DEBUG_PRINTF

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
uint32_t        g_ui32DMATCBBuffer[256];
void            *g_pPSRAMHandle;
void            *g_pMSPIHandle;

// Need to allocate 20 Words even though we only need 16, to ensure we have 16 Byte alignment
AM_SHARED_RW uint32_t axiScratchBuf[20];

am_devices_mspi_psram_config_t MSPI_PSRAM_Config =
{
#if defined(AM_PART_APOLLO4P)
    .eDeviceConfig            = AM_BSP_MSPI_PSRAM_MODULE_HEX_DDR_CE,
#elif defined(AM_PART_APOLLO4B)
    .eDeviceConfig            = AM_BSP_MSPI_PSRAM_MODULE_OCTAL_DDR_CE,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
    .ui32NBTxnBufLength       = sizeof(g_ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = g_ui32DMATCBBuffer,
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

__attribute__ ((section(".heap")))
uint32_t clib_heap[1000];

//*****************************************************************************
//
// Enable printing to the console.
//
//*****************************************************************************
void
enable_print_interface(void)
{
    //
    // Initialize a debug printing interface.
    //
    am_bsp_debug_printf_enable();
}

//*****************************************************************************
//
// Disable printing to the console.
//
//*****************************************************************************
void
disable_print_interface(void)
{
    //
    // Initialize a debug printing interface.
    //
    am_bsp_debug_printf_disable();
    am_hal_itm_disable();
}

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

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    uint32_t      ui32Status;


    //
    //  Set up the DAXICFG.
    //
    am_hal_daxi_config_t sDAXIConfig =
    {
#if defined(AM_PART_APOLLO4P)
        .bDaxiPassThrough         = false,
        .bAgingSEnabled           = true,
        .eAgingCounter            = AM_HAL_DAXI_CONFIG_AGING_1024,
        .eNumBuf                  = AM_HAL_DAXI_CONFIG_NUMBUF_32,
#else // defined(AM_PART_APOLLO4B)
        .agingCounter             = 2,
        .eNumBuf                  = AM_HAL_DAXI_CONFIG_NUMBUF_1,
#endif
        .eNumFreeBuf              = AM_HAL_DAXI_CONFIG_NUMFREEBUF_3,
    };
    am_hal_daxi_config(&sDAXIConfig);
    am_hal_delay_us(100);

    //
    //  Set up the cache.
    //
    am_hal_cachectrl_config_t sCacheConfig =
    {
        .bLRU                       = true,
        .eDescript                  = AM_HAL_CACHECTRL_DESCR_2WAY_128B_2048E,
        .eMode                      = AM_HAL_CACHECTRL_CONFIG_MODE_INSTR_DATA,
    };
    am_hal_cachectrl_config(&sCacheConfig);
    am_hal_cachectrl_enable();

    //
    // Set up scratch AXI buf (needs 64B - aligned to 16 Bytes)
    //
#if defined(AM_PART_APOLLO4B)
    am_hal_daxi_control(AM_HAL_DAXI_CONTROL_AXIMEM, (uint8_t *)((uint32_t)(axiScratchBuf + 3) & ~0xF));
#endif

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Set up the SSRAM reservation.
    //
    PWRCTRL->SSRAMPWREN = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL;  // enable all
    while(PWRCTRL->SSRAMPWRST == 0);

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
    // Power up extended memory.
    //
#ifdef HEAP_IN_SSRAM_AND_PSRAM
    am_hal_pwrctrl_dsp_memory_config_t cfg = g_DefaultDSPMemCfg;
#ifdef AM_PART_APOLLO4P
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &cfg);
#else
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &cfg);
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &cfg);
#endif
#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
#ifdef AM_PART_APOLLO4P
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_Config, &g_pPSRAMHandle, &g_pMSPIHandle);
#else
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_Config, &g_pPSRAMHandle, &g_pMSPIHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
        am_util_stdio_printf("XIP will make this code Hard Fault if external chip is not communicating.\n\n");
        am_util_stdio_printf("Stopping Coremark Pro Test!\n");
        while(1);
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
#ifdef AM_PART_APOLLO4P
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPSRAMHandle);
#else
    ui32Status = am_devices_mspi_psram_aps12808l_ddr_enable_xip(g_pPSRAMHandle);
#endif
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status)
    {
        am_util_stdio_printf("Enable XIP mode in the MSPI!\n");
    }
    else
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    enable_print_interface();
#endif

    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("Coremark_pro Example\n");

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

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

