//*****************************************************************************
//
//! @file daxi_coremark_pro.c
//!
//! @brief This example runs the official EEMBC COREMARK PRO test for multiple
//! loops with vary DAXI settings.
//!
//! GCC build scripts doesn't initialize the global variables which locate in SSRAM.
//! But in this example, we have a global variable array (image texture) in SSRAM,
//! it contains non-zero data. So we should initialize it during startup. I
//! uploaded modified linker_scripts.ld and startup_gcc.c under daxi_coremark_pro
//! path, please copy these 2 files to gcc path to replace the the original
//! generated ones.
//!
//! In this test, we only enabled jpeg compression workload by default. For jpeg
//! compression workload, the default memory allocation is as below.
//!
//! block/section          memory region
//! .text                  MRAM
//! .rodata                MRAM
//! System stack           TCM
//! Task stack             TCM
//! Task heap              TCM
//! System HEAP            TCM
//! HEAP for Test Harness  PSRAM (XIPMM)
//! Input BMP file         SSRAM
//! Output JPEG file       PSRAM (XIPMM)
//!
//! This test also supports cache test, please enable macro "CACHE_TEST" in
//! config-template.ini if you want to run it.
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
//! The Coremark run begins by first outputing a banner (to the SWO)
//! indicating that it has started.
//!
//! Text is output to the SWO at 1M bitrate.
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
#include "am_devices_mspi_psram_aps25616n.h"

extern void run_tasks(void);
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

//!< TODO: Do we really need to alloc this buffer at SSRAM?
AM_SHARED_RW uint32_t        g_ui32DMATCBBuffer[2560];
void            *g_pPSRAMHandle;
void            *g_pMSPIHandle;

am_devices_mspi_psram_config_t MSPI_PSRAM_CE1MSPIConfig =
{
#ifdef APS25616N_OCTAL_MODE_EN
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE1,
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
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the SEGGER SystemView Interface.
    //
#ifdef SYSTEM_VIEW
    SEGGER_SYSVIEW_Conf();
#endif

    //
    // Power up extended memory.
    //
    am_hal_pwrctrl_dsp_memory_config_t cfg = g_DefaultDSPMemCfg;
    am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP0, &cfg);
//     am_hal_pwrctrl_dsp_memory_config(AM_HAL_DSP1, &cfg);

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &MSPI_PSRAM_CE1MSPIConfig, &g_pPSRAMHandle, &g_pMSPIHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(mspi_interrupts[MSPI_PSRAM_MODULE], AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(mspi_interrupts[MSPI_PSRAM_MODULE]);

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPSRAMHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
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
    // DAXI init
    //
    am_util_stdio_printf("Default DAXI setting is 0x%08X.\n", CPU->DAXICFG);
    am_util_stdio_printf("Default cache setting is 0x%08X.\n", CPU->CACHECFG);
    daxi_test_cfg_init();
    cache_test_cfg_init();
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

