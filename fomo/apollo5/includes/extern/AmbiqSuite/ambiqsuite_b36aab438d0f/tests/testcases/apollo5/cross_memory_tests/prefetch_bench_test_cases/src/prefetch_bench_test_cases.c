//*****************************************************************************
//
//! @file prefetch_bench_test_cases.c
//!
//! @brief Test cases to measure prefetch performance for SSRAM and MSPI XIPMM
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "prefetch_bench_test_cases.h"

//*****************************************************************************
//
// Buffers
//
//*****************************************************************************

AM_SHARED_RW uint32_t g_pui32SSRAMBuf[BUF_SIZE / sizeof(uint32_t)] __attribute__ ((aligned(BUF_ALIGN)));
uint32_t *g_pui32XIPMMBuf = (uint32_t *)MSPI_XIP_BASE_ADDRESS;

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
AM_SHARED_RW uint32_t        ui32DMATCBBuffer[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;
bool            g_bMSPIEnabled;

#ifdef APOLLO5_FPGA
#define DEFAULT_MSPI_EDEVICECONFIG AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0
#else
#define DEFAULT_MSPI_EDEVICECONFIG AM_HAL_MSPI_FLASH_HEX_DDR_CE0
#endif
am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = DEFAULT_MSPI_EDEVICECONFIG,
    // 96MHz MSPI SCLK w/ DDR == 192MHz Mtransfers/s
    .eClockFreq               = AM_HAL_MSPI_CLK_192MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer,
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
//  MSPI ISRs.
//
//*****************************************************************************
void psram_mspi_isr(void)
{
   uint32_t      ui32Status;

   am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);
}


//*****************************************************************************
//
//  Initialize a timer for measuring the test duration
//
// Insure that TIMER_NUM is defined as the timer to use, and
// TIMER_TICKS_PER_SEC accurately reflects HFRC / 4 for the platform
//
//*****************************************************************************
void
ctimer_init(void)
{
    am_hal_timer_config_t       TimerConfig;

    //
    // Set up the default configuration.
    //
    am_hal_timer_default_config_set(&TimerConfig);

    //
    // Update the clock in the config.
    //

    TimerConfig.eInputClock             = AM_HAL_TIMER_CLOCK_HFRC_DIV4;
    TimerConfig.eFunction               = AM_HAL_TIMER_FN_UPCOUNT;

    //
    // Configure the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_timer_config(TIMER_NUM, &TimerConfig))
    {
        am_util_stdio_printf("Fail to config timer%d\n", TIMER_NUM);
    }

    //
    // Clear the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_timer_clear_stop(TIMER_NUM))
    {
        am_util_stdio_printf("Fail to clear&stop timer%d\n", TIMER_NUM);
    }

    //
    // Enable the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS != am_hal_timer_enable(TIMER_NUM))
    {
        am_util_stdio_printf("Fail to enable timer%d\n", TIMER_NUM);
    }

    //
    // Start the TIMER.
    //
    if(AM_HAL_STATUS_SUCCESS !=  am_hal_timer_start(TIMER_NUM))
    {
        am_util_stdio_printf("Fail to start timer%d\n", TIMER_NUM);
    }
}

void
mspiSetup(void)
{
    uint32_t ui32Status;

    //
    // Run MSPI DDR timing scan
    //
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_debug_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
    {
        am_util_debug_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_debug_printf("==== Scan Result: Failed, no valid setting.  \n");
    }

    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }

    //
    // Apply DDR timing setting
    //
    ui32Status = am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pPsramHandle, &MSPIDdrTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
    }

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);

    TEST_ASSERT_TRUE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        while(1){};
    }
    g_bMSPIEnabled = true;
}

void
mspiTeardown(void)
{
    if(!g_bMSPIEnabled)
    {
        return;
    }

    uint32_t ui32Status;

    //
    // Need to make sure all pending XIPMM transactions are flushed
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

    //
    // Disable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_disable_xip(g_pPsramHandle);

    TEST_ASSERT_TRUE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to disable XIP mode in the MSPI!\n");
        while(1){};
    }

    NVIC_DisableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);

    //
    // Shut down the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_deinit(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to shut down the MSPI and PSRAM Device correctly!\n");
    }

    g_sMspiPsramConfig.eDeviceConfig = DEFAULT_MSPI_EDEVICECONFIG;
    g_bMSPIEnabled = false;
}


//*****************************************************************************
//
//  Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{

    am_hal_cachectrl_dcache_enable(true);
    am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE);

    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();
    ctimer_init();
    g_bMSPIEnabled = false;

    am_util_stdio_printf("\n\n");
}

void
globalTearDown(void)
{

}


//*****************************************************************************
//
//  Optional setup/tear-down functions.
//
// These will be called before and after each test function listed below.
//
//*****************************************************************************
void
setUp(void)
{

}

void
tearDown(void)
{
    mspiTeardown();
}

//*****************************************************************************
//
//  Turn on or off the CPU prefetcher
//
// bool bEnable: true to enable prefetching, false to disable
//
//*****************************************************************************
void
set_prefetch(bool bEnable)
{
    if(bEnable)
    {
        MEMSYSCTL->PFCR |= MEMSYSCTL_PFCR_ENABLE_Msk; // enable prefetch
    }
    else
    {
        MEMSYSCTL->PFCR &= ~MEMSYSCTL_PFCR_ENABLE_Msk; // disable prefetch
    }
}

//*****************************************************************************
//
//  Subtract two uint32_t timer values accounting for wraparound
//
//
// ui32Prev: Starting timer value
// ui32Curr: Ending timer value
//
// Returns the difference in timer ticks
//
//*****************************************************************************
static uint32_t cal_time(uint32_t ui32Prev, uint32_t ui32Curr)
{
  if(ui32Prev > ui32Curr)
  {
      return 0xFFFFFFFF - ui32Prev + ui32Curr;
  }
  else
  {
      return ui32Curr - ui32Prev;
  }
}

//*****************************************************************************
//
//  Optimized CPU read generator for ARM CPUs
//
// Generates contiguous reads across an address range. Read data is thrown away
// in the process. Reads are optimized with 8 word multiple load operations for
// single cycle 32 byte cache line reads.
// These reads are manually unrolled 32 times based on the experimental
// observation that performance is reduced below 32 inlined 8-word reads.
// This may have to do with M55 branch prediction, though the pipeline is only
// 4 stage...
//
// Currently only Keil6 (MDK >=5.36) and GCC are supported. Other compilers will
// need their own custom inline asm.
//
// ui32SrcAddr: address for the source buffer
// ui32NumBytes: number of bytes to read. Rounded down to the
//               nearest 1KB
//
//*****************************************************************************
#if (defined (__ARMCC_VERSION)) && (__ARMCC_VERSION >= 6000000)
__attribute__((naked))
static void
kb_read( uint32_t ui32SrcAddr, uint32_t ui32NumBytes)
{
__asm
    (
     "    push    {r3-r10}\n\t"              // Save r3-r10 - used by this function
     "    lsr     r1, r1, #10\n\t"           // convert bytes to # iterations
     "__kb_read_loop:\n\t"
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    subs    r1, r1, #1\n\t"
     "    bne     __kb_read_loop\n\t"
     "    pop     {r3-r10}\n\t"              // Restore registers
     "    bx      lr\n\t"
    );
}
#elif defined(__GNUC_STDC_INLINE__)
__attribute__((naked))
static void
kb_read( uint32_t ui32SrcAddr, uint32_t ui32NumBytes)
{
__asm
    (
     "    push       {r3-r10}\n\t"           // Save r3-r10 - used by this function
     "    lsr     r1, r1, #10\n\t"           // convert bytes to # iterations
     "__kb_read_loop:\n\t"
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    ldmia r0!, {r3-r10}\n\t"           // load 8 words to registers
     "    subs    r1, r1, #1\n\t"
     "    bne     __kb_read_loop\n\t"
     "    pop     {r3-r10}\n\t"              // Restore registers
     "    bx      lr\n\t"
    );
}
#else
#error "Compiler Not supported"
#endif

//*****************************************************************************
//
//  Benchmark optimized CPU reads from another memory
//
// pMemBase: Pointer to the source memory
// ui32Length: Length of the memory in bytes. Should be a multiple of 1KB for
//   accurate results
// ui32Loops: Times to repeat the memory read
// bInvalidate: If true, invalidates cache prior to each read iteration
//
// Returns the average duration of the buffer reads over the benchmark iterations
// in seconds.
//
//*****************************************************************************
float
cpu_read_bench(void* pMemBase, uint32_t ui32Length, uint32_t ui32Loops, uint32_t bInvalidate)
{
    uint32_t ui32TimerTickBefore;
    uint32_t ui32ReadTimeTotal = 0;

    memset(pMemBase, 0, ui32Length);

    for (uint32_t ui32Loop = 0; ui32Loop < ui32Loops; ui32Loop++)
    {
        if(bInvalidate)
        {
            SCB_CleanInvalidateDCache();
            am_hal_sysctrl_bus_write_flush();
        }
        ui32TimerTickBefore = am_hal_timer_read(TIMER_NUM);
        kb_read((uint32_t)pMemBase, ui32Length);
        ui32ReadTimeTotal += cal_time(ui32TimerTickBefore, am_hal_timer_read(TIMER_NUM));
    }
    am_hal_sysctrl_bus_write_flush();
    return (float)((double)ui32ReadTimeTotal / TIMER_TICKS_PER_SEC / ui32Loops);
}

//*****************************************************************************
//
//  Benchmark MSPI XIPMM reads with prefetch enabled in hex mode
//
// Returns true always
//
//*****************************************************************************
bool
xipmm_bench_with_prefetch_hex_test(void)
{
    bool bTestPass = true;
    float fSec;
    g_sMspiPsramConfig.eDeviceConfig = AM_HAL_MSPI_FLASH_HEX_DDR_CE0;
    mspiSetup();
    set_prefetch(true);
    fSec = cpu_read_bench(g_pui32XIPMMBuf, BUF_SIZE, 1000, true);
    am_util_stdio_printf("XIPMM hex Read not precached prefetch:     read time  %f s  %4.3f MB/s\n", fSec, BUF_SIZE/fSec/(1024*1024));

    return bTestPass;
}

//*****************************************************************************
//
//  Benchmark MSPI XIPMM reads with prefetch disabled in hex mode
//
// Returns true always
//
//*****************************************************************************
bool
xipmm_bench_without_prefetch_hex_test(void)
{
    bool bTestPass = true;
    float fSec;
    g_sMspiPsramConfig.eDeviceConfig = AM_HAL_MSPI_FLASH_HEX_DDR_CE0;
    mspiSetup();

    set_prefetch(false);
    fSec = cpu_read_bench(g_pui32XIPMMBuf, BUF_SIZE, 1000, true);
    am_util_stdio_printf("XIPMM hex Read not precached no prefetch:     read time  %f s  %4.3f MB/s\n", fSec, BUF_SIZE/fSec/(1024*1024));

    return bTestPass;
}

//*****************************************************************************
//
//  Benchmark MSPI XIPMM reads with prefetch enabled in octal mode
//
// Returns true always
//
//*****************************************************************************
bool
xipmm_bench_with_prefetch_octal_test(void)
{
    bool bTestPass = true;
    float fSec;
    g_sMspiPsramConfig.eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0;
    mspiSetup();
    set_prefetch(true);
    fSec = cpu_read_bench(g_pui32XIPMMBuf, BUF_SIZE, 1000, true);
    am_util_stdio_printf("XIPMM octal Read not precached prefetch:     read time  %f s  %4.3f MB/s\n", fSec, BUF_SIZE/fSec/(1024*1024));

    return bTestPass;
}

//*****************************************************************************
//
//  Benchmark MSPI XIPMM reads with prefetch disabled in octal mode
//
// Returns true always
//
//*****************************************************************************
bool
xipmm_bench_without_prefetch_octal_test(void)
{
    bool bTestPass = true;
    float fSec;
    g_sMspiPsramConfig.eDeviceConfig = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0;
    mspiSetup();

    set_prefetch(false);
    fSec = cpu_read_bench(g_pui32XIPMMBuf, BUF_SIZE, 1000, true);
    am_util_stdio_printf("XIPMM octal Read not precached no prefetch:     read time  %f s  %4.3f MB/s\n", fSec, BUF_SIZE/fSec/(1024*1024));

    return bTestPass;
}

//*****************************************************************************
//
//  Benchmark SSRAM reads with prefetch enabled
//
// Returns true always
//
//*****************************************************************************
bool
ssram_bench_with_prefetch_test(void)
{
    bool bTestPass = true;
    float fSec;

    set_prefetch(true);
    fSec = cpu_read_bench(g_pui32SSRAMBuf, BUF_SIZE, 1000, true);
    am_util_stdio_printf("Read not precached prefetch:     read time  %f s  %4.3f MB/s\n", fSec, BUF_SIZE/fSec/(1024*1024));

    return bTestPass;
}

//*****************************************************************************
//
//  Benchmark SSRAM reads with prefetch disabled
//
// Returns true always
//
//*****************************************************************************
bool
ssram_bench_without_prefetch_test(void)
{
    bool bTestPass = true;
    float fSec;

    set_prefetch(false);
    fSec = cpu_read_bench(g_pui32SSRAMBuf, BUF_SIZE, 1000, true);
    am_util_stdio_printf("Read not precached no prefetch:  read time  %f s  %4.3f MB/s\n", fSec, BUF_SIZE/fSec/(1024*1024));

    return bTestPass;
}

//*****************************************************************************
//
//  Benchmark cached SSRAM reads
//
// For Apollo5 with 64K dcache, BUF_SIZE must be significantly less than the
// total cache size. 32K is a good upper bound to be safe.
//
// Returns true always
//
//*****************************************************************************
bool
ssram_bench_precached_test(void)
{
    bool bTestPass = true;
    float fSec;

    set_prefetch(true);
    fSec = cpu_read_bench(g_pui32SSRAMBuf, BUF_SIZE, 1000, false);
    am_util_stdio_printf("Read precached:                  read time  %f s  %4.3f MB/s\n", fSec, BUF_SIZE/fSec/(1024*1024));

    return bTestPass;
}
