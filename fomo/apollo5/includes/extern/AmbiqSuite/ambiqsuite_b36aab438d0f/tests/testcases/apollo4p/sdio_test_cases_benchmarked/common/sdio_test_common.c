//*****************************************************************************
//
//! @file sdio_test_common.c
//!
//! @brief Example test case program.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "sdio_test_common.h"

#define DEBUG_PRINT        am_util_stdio_printf

uint8_t ui8RdBuf[BUF_LEN] AM_BIT_ALIGNED(128);
uint8_t ui8WrBuf[BUF_LEN] AM_BIT_ALIGNED(128);

sdio_speed_t sdio_test_speeds[] =
{
  { 48000000,    "48MHz" },
  { 24000000,    "24MHz" },
  { 12000000,    "12MHz" },
  {  3000000,     "3MHz" },  
  {   750000,    "750KHz"},
  {   375000,    "375KHz"},
};

sdio_width_t sdio_test_widths[] = 
{
  { AM_HAL_HOST_BUS_WIDTH_1,    "1bit" },
  { AM_HAL_HOST_BUS_WIDTH_4,    "4bit" },
  { AM_HAL_HOST_BUS_WIDTH_8,    "8bit" },
};

sdio_mode_t sdio_test_modes[] = 
{
  { AM_HAL_HOST_UHS_SDR50,    "SDR50" },
  { AM_HAL_HOST_UHS_DDR50,    "DDR50" },
};

sdio_voltage_t sdio_test_voltages[] =
{
  { AM_HAL_HOST_BUS_VOLTAGE_3_3,    "3.3V" },
  { AM_HAL_HOST_BUS_VOLTAGE_1_8,    "1.8V" },
};

#ifdef COLLECT_BENCHMARKS
sdio_benchmark_data_t g_BenchmarkData[N_BENCHMARKS]; 
#endif

uint32_t g_ui32Count = 0; 

//*****************************************************************************
//
// Init function for Timer A0.  per DaveC
//
//*****************************************************************************
void
stimer_init(void)
{
    //
    // Enable compare A interrupt in STIMER
    //
    am_hal_stimer_int_enable(AM_HAL_STIMER_INT_COMPAREA);

    //
    // Enable the timer interrupt in the NVIC.
    //
    NVIC_SetPriority(STIMER_CMPR0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(STIMER_CMPR0_IRQn);

    //
    // Configure the STIMER and run
    //
    am_hal_stimer_config(AM_HAL_STIMER_CFG_CLEAR | AM_HAL_STIMER_CFG_FREEZE);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);
    am_hal_stimer_config(AM_HAL_STIMER_XTAL_32KHZ |
                         AM_HAL_STIMER_CFG_COMPARE_A_ENABLE);

}


//*****************************************************************************
//
// Timer Interrupt Service Routine (ISR)  also per DaveC
//
//*****************************************************************************
void
am_stimer_cmpr0_isr(void)
{
    //
    // Check the timer interrupt status.
    //
    am_hal_stimer_int_clear(AM_HAL_STIMER_INT_COMPAREA);
    am_hal_stimer_compare_delta_set(0, WAKE_INTERVAL);

    g_ui32Count++;
}

#ifdef COLLECT_BENCHMARKS
//*****************************************************************************
//
// Function - init_benchmark_data()
// Description - Initializes all g_BenchmarkData array members with the
//  value .bIsValid = false.  Will be updated to true after valid data is
//  stored at the location in store_benchmark_data(). 
//
//*****************************************************************************
void
init_benchmark_data(void)
{
    // Initialize all array members to be invalid
    for(uint32_t i = 0; i < N_BENCHMARKS; i++)
    {
        g_BenchmarkData[i].bIsValid = false;
    }
}

//*****************************************************************************
//
// Function - store_benchmark_data()
// Description - Stores benchmark data passed, and uses it to calculate and 
//  store the Read and Write speeds in MBPS. 
//
//*****************************************************************************
void
store_benchmark_data(sdio_rw_type_e eRWType, sdio_speed_t* psSpeed, sdio_width_t* psWidth, sdio_mode_t* psMode, uint32_t ui32WriteMs, uint32_t ui32WriteVolume, uint32_t ui32ReadMs, uint32_t ui32ReadVolume)
{
    static uint32_t iBM = 0; // benchmark index
    float32_t f32WriteMBPS, f32ReadMBPS, f32WriteMs, f32WriteVolume, f32ReadMs, f32ReadVolume = 0;

    if (iBM >= N_BENCHMARKS)
    {
        am_util_stdio_printf("ERROR - could not store benchmark data, iBM %d exceeded N_BENCHMARKS %d\n", iBM, N_BENCHMARKS);
    }
    else
    {
        // Store Passed Data
        g_BenchmarkData[iBM].bIsValid = true;
        g_BenchmarkData[iBM].eRWType = eRWType;
        g_BenchmarkData[iBM].psSpeed = psSpeed;
        g_BenchmarkData[iBM].psWidth = psWidth;
        g_BenchmarkData[iBM].psMode = psMode;
        g_BenchmarkData[iBM].ui32WriteMs = ui32WriteMs;
        g_BenchmarkData[iBM].ui32WriteVolume = ui32WriteVolume;
        g_BenchmarkData[iBM].ui32ReadMs = ui32ReadMs;
        g_BenchmarkData[iBM].ui32ReadVolume = ui32ReadVolume;

        // Calculate & Store Write MBPS
        f32WriteVolume = (float32_t)g_BenchmarkData[iBM].ui32WriteVolume;
        f32WriteMs = (float32_t)g_BenchmarkData[iBM].ui32WriteMs;
        f32WriteMBPS = (f32WriteVolume / f32WriteMs) / 1000;
        g_BenchmarkData[iBM].f32WriteMBPS = f32WriteMBPS;

        // Calculate & Store Read MBPS
        f32ReadVolume = (float32_t)g_BenchmarkData[iBM].ui32ReadVolume;
        f32ReadMs = (float32_t)g_BenchmarkData[iBM].ui32ReadMs;
        f32ReadMBPS = (f32ReadVolume / f32ReadMs) / 1000;
        g_BenchmarkData[iBM].f32ReadMBPS = f32ReadMBPS;

        iBM++;
    }
}

//*****************************************************************************
//
// Function - display_single_benchmark_data()
// Description - Displays benchmark data at the specified index, only if the data
//   is valid and the index does not exceed the buffer size
//
//*****************************************************************************
void
display_single_benchmark(uint32_t index)
{

    if ((g_BenchmarkData[index].bIsValid == true) && (index < N_BENCHMARKS))
    {
        am_util_stdio_printf("eRWType=%d, ", g_BenchmarkData[index].eRWType);
        am_util_stdio_printf("Speed=%s, ", g_BenchmarkData[index].psSpeed->string);
        am_util_stdio_printf("Width=%s, ", g_BenchmarkData[index].psWidth->string);
        am_util_stdio_printf("Mode=%s, ", g_BenchmarkData[index].psMode->string);
        am_util_stdio_printf("WriteSpeed=%1.2fMBPS, ", g_BenchmarkData[index].f32WriteMBPS); 
        am_util_stdio_printf("ReadSpeed=%1.2fMBPS", g_BenchmarkData[index].f32ReadMBPS); 
        am_util_stdio_printf("\n");
    }
    else
    {
        am_util_stdio_printf("Data at index %d is invalid and will not be displayed\n", index);
    }
}

//*****************************************************************************
//
// Function - display_benchmark_data()
// Description - Displays all valid benchmark data for a specific read/write
//  type 
//
//*****************************************************************************
void
display_benchmark_data(sdio_rw_type_e eRWType)
{
    switch(eRWType)
    {
        case eRW_PIO:
            am_util_stdio_printf("PIO ");
            break;
        case eRW_SYNC:
            am_util_stdio_printf("Synchronous ");
            break;
        case eRW_ASYNC:
            am_util_stdio_printf("Asynchronous ");
            break;
        default:
            am_util_stdio_printf("ALL ");
            break;
    }
    am_util_stdio_printf("Benchmark Data:\n");
    
    for(uint32_t i = 0; i < N_BENCHMARKS; i++)
    {
        if((g_BenchmarkData[i].eRWType == eRWType) && (g_BenchmarkData[i].bIsValid == true))
        {
            display_single_benchmark(i);
        }
    }

}
#endif

//*****************************************************************************
//
// Optional Global setup/tear-down.
//
// globalSetUp() will get called before the test group starts, and
// globalTearDown() will get called after it ends. These won't be necessary for
// every test.
//
//*****************************************************************************
void
globalSetUp(void)
{
    
#ifdef COLLECT_BENCHMARKS
  //
  // Enable floating point
  //
  am_hal_sysctrl_fpu_enable();
  am_hal_sysctrl_fpu_stacking_enable(true);
#endif
 
#if (!defined(AM_PART_APOLLO4) && !defined(AM_PART_APOLLO4B) && !defined(AM_PART_APOLLO4P))
  //
  // Set the clock frequency.
  //
  am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);
#endif
  
#if !defined(APOLLO4_FPGA)
  //
  // Set the default cache configuration
  //
  am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
  am_hal_cachectrl_enable();
#endif

  //
  // Configure the board for low power operation.
  //
  am_bsp_low_power_init();
  
  stimer_init();  // DaveC enabling Stimer
  
  //
  // Global interrupt enable
  //
  am_hal_interrupt_master_enable();

  //
  // Print the banner.
  //
  am_bsp_itm_printf_enable();
  am_util_stdio_terminal_clear();
  am_util_stdio_printf("Apollo4 SDIO Test Cases\n\n");

  //
  // Configure SDIO PINs.
  //
  am_bsp_sdio_pins_enable(AM_HAL_HOST_BUS_WIDTH_8);

  //
  // level shift control in emmc board
  //
#if defined(FMC_BOARD_EMMC_TEST) || defined(SD_CARD_BOARD_SUPPORT_1_8_V)
  am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL, am_hal_gpio_pincfg_output);
  am_hal_gpio_output_clear(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#else
  am_hal_gpio_pinconfig(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL, am_hal_gpio_pincfg_output);
  am_hal_gpio_output_set(AM_BSP_GPIO_SD_LEVEL_SHIFT_SEL);
#endif

    //
    // initialize the test read and write buffers
    //
    for (int i = 0; i < BUF_LEN; i++)
    {
        ui8WrBuf[i] = rand() % 256;
        ui8RdBuf[i] = 0x0;
    }

#ifdef COLLECT_BENCHMARKS
    // Initialize the benchmark data structure
    init_benchmark_data();
#endif
}

void
globalTearDown(void)
{
}

