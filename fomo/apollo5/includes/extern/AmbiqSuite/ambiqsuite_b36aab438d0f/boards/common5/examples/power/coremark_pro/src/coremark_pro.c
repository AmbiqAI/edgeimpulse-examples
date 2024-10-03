//*****************************************************************************
//
//! @file coremark_pro.c
//!
//! @brief EEMBC COREMARK PRO test.
//!
//! Purpose: This example runs the official EEMBC COREMARK PRO test.
//!
//! CoreMark(tm) PRO is a comprehensive, advanced processor benchmark that works
//! with and enhances the market-proven industry-standard EEMBC CoreMark(tm) benchmark.
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

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
//#define AM_DEBUG_PRINTF

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

extern void vStartCoremark(void);

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{

#ifdef gcc
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();
#endif

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

#if ENABLE_CPU_HP_250
    am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE);
#else
    am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_LOW_POWER);
#endif

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    am_hal_itm_enable();        // need to call to enable the PMU when not using debugger or ITM

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
    // Initialize the psram and enable XIP mode.
    //
#ifdef gcc
    mspi_psram_init();
#endif
    mspi_psram_config_print();

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize plotting interface.
    //
    am_util_stdio_terminal_clear();
    am_util_debug_printf("Coremark_pro Example\n");

#if defined ( NDEBUG )
    am_util_stdio_printf("Release");
#else
    am_util_stdio_printf("Debug");
#endif
#if defined ( __ICCARM__ )
    am_util_stdio_printf(" built with IAR on %s %s\n", __DATE__, __TIME__);
#elif defined ( __GNUC__ )
    am_util_stdio_printf(" built with GCC on %s %s\n", __DATE__, __TIME__);
#else
    am_util_stdio_printf(" built with unknown on %s %s\n", __DATE__, __TIME__);
#endif /* __ICCARM__ */
    am_util_stdio_printf(" Version: %s\n", __VERSION__);

    am_util_stdio_printf("CPU clock = %d Hz\n", MY_CORECLK_HZ );

#if defined ( HEAP_IN_SSRAM_AND_PSRAM )
    am_util_stdio_printf(" Heap in SSRAM and PSRAM\n");
#elif defined ( HEAP_IN_PSRAM )
    am_util_stdio_printf(" Heap in PSRAM\n");
#elif defined ( HEAP_IN_SSRAM0 )
    am_util_stdio_printf(" Heap in SSRAM0\n");
#else
    am_util_stdio_printf(" Heap location undefined!\n");
#endif
    am_util_stdio_printf("REPORT_THMALLOC_STATS = %d\n", REPORT_THMALLOC_STATS);

    am_util_stdio_printf("Coremark_pro starting\n");

    //
    // Run the application.
    //
#if defined(AM_BARE_METAL)
    vStartCoremark();
#elif defined(AM_FREERTOS)
    run_tasks();
#endif

    am_util_stdio_printf("\nCoreMarkPro finished...\n");

    while (1)
    {
    }
}

#ifdef keil6
void _platform_pre_stackheap_init(void)
{
    am_bsp_low_power_init();
    am_util_delay_ms(2000);
    mspi_psram_init();
}
#endif
