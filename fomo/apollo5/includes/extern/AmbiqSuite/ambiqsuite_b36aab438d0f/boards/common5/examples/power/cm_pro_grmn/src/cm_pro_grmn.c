//*****************************************************************************
//
//! @file cm_pro_grmn.c
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "cm_pro_grmn.h"

#include "gar_stream.h"

//#include "th_lib.h"

//*****************************************************************************
//
// External prototypes
//
//*****************************************************************************
extern void linear_alg_mid_100x100_sp_main(int argc, char *argv[]);
extern void cjpeg_rose7_preset_main(int argc, char *argv[]);
extern void core_main(int argc, char *argv[]);
extern void loops_all_mid_10k_sp_main(int argc, char *argv[]);
extern void nnet_test_main(int argc, char *argv[]);
extern void parser_125k_main(int argc, char *argv[]);
extern void radix2_big_64k_main(int argc, char *argv[]);
extern void sha_test_main(int argc, char *argv[]);
extern void zip_test_main(int argc, char *argv[]);

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


int main(void)
{
//  uint32_t      ui32Status;
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
//  am_bsp_itm_printf_enable();
    am_bsp_uart_printf_enable();
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

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();

#ifdef AM_DEBUG_PRINTF
    //
    // Enable printing to the console.
    //
    enable_print_interface();
#endif

    //
    // Initialize plotting interface.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("cm_pro_grmn - ");
#if defined ( USE_FP32 )
    am_util_stdio_printf("USE_FP32 defined\n");
#elif defined ( USE_FP64 )
    am_util_stdio_printf("USE_FP64 defined\n");
#else
    am_util_stdio_printf("FP not defined!\n");
#endif

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

#if defined ( HEAP_IN_PSRAM )
    am_util_stdio_printf(" Heap in PSRAM\n");
#elif defined ( HEAP_IN_SSRAM )
    am_util_stdio_printf(" Heap in SSRAM\n");
#else
    am_util_stdio_printf(" Heap in TCM\n");
#endif
    am_util_stdio_printf("REPORT_THMALLOC_STATS = %d\n", REPORT_THMALLOC_STATS);

    am_util_stdio_printf("cm_pro_grmn starting\n");

#ifdef ENABLE_PMU
    am_util_pmu_config_t pmu_config;
    am_util_pmu_profiling_t pmu_profiling;
    //
    // Select event types here
    //
    pmu_config.ui32Counters = VALID_PMU_COUNTERS;               // Enable all valid event counters

    pmu_config.ui32EventType[0] = ARM_PMU_L1I_CACHE_REFILL;
    pmu_config.ui32EventType[1] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[2] = ARM_PMU_L1I_CACHE;
    pmu_config.ui32EventType[3] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[4] = ARM_PMU_L1D_CACHE_MISS_RD;
    pmu_config.ui32EventType[5] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    pmu_config.ui32EventType[6] = ARM_PMU_L1D_CACHE_RD;
    pmu_config.ui32EventType[7] = ARM_PMU_CHAIN;                // Chain an odd-numbered counter with a preceding even-numbered counter to form a 32-bit counter.

    am_util_pmu_init(&pmu_config);
#endif

#if 0
    // Quick test to confirm external PSRAM is working
    int i;
    char *p;
    char *tmp;
    p = th_malloc( 64 );
    if (p != NULL)
    {
        tmp = p;
        for (i = 0; i < 64; i++)
        {
            *tmp++ = i;
        }
        th_free( p );
    }
#endif

    int argc = 4;
    char *argv[ 4 ];
    argv[ 0 ] = "-v0";
    argv[ 1 ] = "-i1";
    argv[ 2 ]  = "-c1";
    argv[ 3 ] = "-w1";

#if RUN_CJPEG
    #if ENABLE_CPU_HP_250
    argv[1] = "-i65";
    #else
    argv[1] = "-i25";
    #endif

    // The performance monitoring unit will be enabled right before each
    // workload is run in mith_main_loop(). Several of the workloads require
    // a fair amount of setup, so we don't want to record the cache info until
    // the actual workload runs.

    am_util_stdio_printf("Running cjpeg_rose7_preset tasks...\n");
    al_start_time();
    cjpeg_rose7_preset_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("cjpeg_rose7_preset tasks done...\n\n");
#endif

#if RUN_CORE
    argv[ 1 ] = "-i1";

    am_util_stdio_printf("Running core tasks...\n");
    al_start_time();
    core_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("core tasks done...\n\n");
#endif

#if RUN_LIN_100
  #if defined ( HEAP_IN_PSRAM )
    #if ENABLE_CPU_HP_250
    argv[1] = "-i35";
    #else
    argv[1] = "-i15";
    #endif
  #else
    #if ENABLE_CPU_HP_250
    argv[1] = "-i35";
    #else
    argv[1] = "-i15";
    #endif
  #endif

    am_util_stdio_printf("Running linear_alg_mid_100x100_sp tasks...\n");
    al_start_time();
    linear_alg_mid_100x100_sp_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("linear_alg_mid_100x100_sp tasks done...\n\n");
#endif

#if RUN_MID_10K
    argv[ 1 ] = "-i1";

    am_util_stdio_printf("Running loops_all_mid_10k_sp tasks...\n");
    al_start_time();
    loops_all_mid_10k_sp_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("loops_all_mid_10k_sp tasks done...\n");
#endif

#if RUN_NNET
    argv[1] = "-i1";

    am_util_stdio_printf("Running nnet_test tasks...\n");
    al_start_time();
    nnet_test_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("nnet_test tasks done...\n");
#endif

#if RUN_PARSER
    #if ENABLE_CPU_HP_250
    argv[1] = "-i6";
    #else
    argv[1] = "-i2";
    #endif

    am_util_stdio_printf("Running parser_125k tasks...\n");
    al_start_time();
    parser_125k_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("parser_125k tasks done...\n");
#endif

#if RUN_RADIX2
    #if ENABLE_CPU_HP_250
    argv[1] = "-i35";
    #else
    argv[1] = "-i12";
    #endif

    am_util_stdio_printf("Running radix2_big_64k tasks...\n");
    al_start_time();
    radix2_big_64k_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("radix2_big_64k tasks done...\n");
#endif

#if RUN_SHA
  #if defined ( HEAP_IN_PSRAM )
    #if ENABLE_CPU_HP_250
    argv[1] = "-i65";
    #else
    argv[1] = "-i25";
    #endif
  #else
    #if ENABLE_CPU_HP_250
    argv[1] = "-i50";
    #else
    argv[1] = "-i25";
    #endif
  #endif

    am_util_stdio_printf("Running sha_test tasks...\n");
    al_start_time();
    sha_test_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("sha_test tasks done...\n");
#endif

#if RUN_ZIP
    #if ENABLE_CPU_HP_250
    argv[1] = "-i30";
    #else
    argv[1] = "-i10";
    #endif

    am_util_stdio_printf("Running zip_test tasks...\n");
    al_start_time();
    zip_test_main(argc, argv);
    al_stop_time();
    //
    // Stop Events Counters & Collect Profilings.
    //
    pmu_get_profiling(&pmu_config, &pmu_profiling);
    am_util_stdio_printf("zip_test tasks done...\n");
#endif

    //
    // Disable performance monitoring unit
    //
    pmu_disable();
#ifdef ENABLE_PMU
    am_util_pmu_deinit();
#endif

#if RUN_GAR_STREAM
    am_util_stdio_printf("Running gar_stream tasks...\nInfo: Starting Run...\n");

    al_start_time();
    gar_stream_test();
    al_stop_time();

    am_util_stdio_printf("gar_stream tasks done...\n");

#endif

#if RUN_ZIP || RUN_GAR_STREAM
    am_util_stdio_printf("\nAll tests done...\n");
#endif

    __asm("    bkpt     ");

    return ( 1 );
}

#if __GNUC__ > 10
    int _close (int __fildes)
    {
        return ( 0 );
    }

    struct stat;
    int _fstat (int __fd, struct stat *__sbuf )
    {
        return ( 0 );
    }

    pid_t _getpid (void)
    {
        return ( 0 );
    }

    int _isatty (int __fildes)
    {
        return ( 0 );
    }

    int _kill (pid_t, int)
    {
        return ( 0 );
    }

    off_t _lseek (int __fildes, off_t __offset, int __whence)
    {
        return ( 0 );
    }

    _READ_WRITE_RETURN_TYPE _read (int __fd, void *__buf, size_t __nbyte)
    {
        return ( 0 );
    }

    _READ_WRITE_RETURN_TYPE _write (int __fd, const void *__buf, size_t __nbyte)
    {
        return ( 0 );
    }
#endif

#ifdef keil6
void _platform_pre_stackheap_init(void)
{
    am_bsp_low_power_init();
    am_util_delay_ms(2000);
    mspi_psram_init();
}
#endif