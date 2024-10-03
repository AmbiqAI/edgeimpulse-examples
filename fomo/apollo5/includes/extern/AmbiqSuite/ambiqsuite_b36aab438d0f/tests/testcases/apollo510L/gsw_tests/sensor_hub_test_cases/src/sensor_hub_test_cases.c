//*****************************************************************************
//
//! @file sensor_hub_test_cases.c
//!
//! @brief This test is based off the Sensor Hub Test Case description in
//! https://ambiqmicro.atlassian.net/browse/CAYNSWS-165. The test runs on the
//! Apollo5 Presilicon Turbo Board fly-wired to an Apollo4p EVB running the 
//! ios_fifo project at boards/common4/examples/interfaces/ios_fifo, built for 
//! the apollo4p_evb (prebuilt binary ios_fifo.bin.apollo4p_evb is located 
//! in the root directory of this project). 
//!
//! NOTE: By default, PROCESS_SENSORS_TASK_ENABLE is set to 0 in
//!     sensor_hub_test_cases.h.  This setting is necessary for the test to
//!     run when no IOS slave is connected.  If the IOS slave is connected
//!     and configured as in the above description, PROCESS_SENSORS_TASK_ENABLE
//!     should be set to 1 before compiling.
//!
//! TASKS:
//!     GUI Task 
//!             - task for watch face display rendering
//!     Process Sensors Task 
//!             - task for processing of sensor hub data.  Sends
//!               data to emmc task in message buffer every 16K bytes
//!     eMMC Task 
//!             - If process sensors task is enabled, this task waits for data
//!               in the message buffer (received from process sensors task), and writes
//!               this data to the eMMC card
//!             - If process sensors task is disabled, this task writes data from a
//!             pre-filled buffer to the eMMC card every 500ms 
//!     Bring to See (BTS) Task  
//!             - task to turn the display on/off every ~5 seconds
//! 
//! HARDWARE CONFIG:
//! 1. Display Card Config:
//!     This test requires use of the Apollo5 Expansion Card 1, which is a display card
//!     of which there are 2 versions.
//! 
//!     The jumper configuration is slightly different for these cards, based on the 
//!     connection of GPIO92 with respect to the DISPLAY_RESET signal.  The DISPLAY_RESET
//!     signal cannot be assigned to GPIO92 as it is the CSn signal for IOM1.
//! 
//!     - Display Card Rev0: J17 must be IN (jumpered)
//!     - Display Card Rev2: J17 must be OUT (not jumpered)
//!
//! 2. Required Hardware Fly-wire connections when PROCESS_SENSORS_TASK_ENABLE is 1:
//!
//! Silicon - Apollo5 EB Connections
//! IOM1  Apollo5 EB  AP4P EVB 
//! ----  ----------  --------
//! SLINT    GP4 <-----> GP4 
//! SCK      GP8 <-----> GP0 
//! MOSI     GP9 <-----> GP1 
//! MISO     GP10 <-----> GP2 
//! CSn      GP92 <-----> GP3 
//!
//! FPGA - Turbo Board Connections
//! IOM1  Turbo FMC  AP4P EVB  AP5 GPIO  
//! ----  ---------  --------  --------
//! SLINT    GP4 <-----> GP4       4
//! SCK      GP5 <-----> GP0       8
//! MOSI     GP6 <-----> GP1       9
//! MISO     GP7 <-----> GP2      10
//! CSn      GP8 <-----> GP3      11
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "sensor_hub_test_cases.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************

void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0,
#ifdef APOLLO5_FPGA
    .eClockFreq               = AM_HAL_MSPI_CLK_250MHZ,
#else
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
#endif
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

   am_hal_mspi_interrupt_status_get(g_pMSPIPsramHandle, &ui32Status, false);

   am_hal_mspi_interrupt_clear(g_pMSPIPsramHandle, ui32Status);

   am_hal_mspi_interrupt_service(g_pMSPIPsramHandle, ui32Status);
}


//*****************************************************************************
//
// Optional setup/tear-down functions.
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
}

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
#ifndef APOLLO5_FPGA
    am_util_delay_ms(2000);
#endif
	
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);

}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// Sensor Hub Test 
//
//
//*****************************************************************************

void sensor_hub_test(void)
{
    uint32_t ui32Status = 0;

    // Configure Debug Pins 
    am_hal_gpio_pinconfig(GPIO_GUI_TASK, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_DISPLAY_ON_OFF, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_SLEEP_WAKE, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_GUI_TIMER, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_TICK_ERR, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_STIMER_ISR, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_TICK_HANDLER, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_DELTA_TOO_SMALL, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_EMMC_TASK, am_hal_gpio_pincfg_output);

    am_hal_gpio_state_write(GPIO_GUI_TASK, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_DISPLAY_ON_OFF, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_SLEEP_WAKE, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_GUI_TIMER, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_TICK_ERR, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_STIMER_ISR, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_TICK_HANDLER, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_DELTA_TOO_SMALL, AM_HAL_GPIO_OUTPUT_CLEAR);
    am_hal_gpio_state_write(GPIO_EMMC_TASK, AM_HAL_GPIO_OUTPUT_CLEAR);

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Sensor Hub Test Cases\n");
#if defined(RUNTIME_MIN) && (GUI_TASK_ENABLE == 1)
    am_util_stdio_printf("Test will complete after %d minutes\n", RUNTIME_MIN);
#else
    am_util_stdio_printf("Test will run forever\n");
#endif
    
    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();

#if (GUI_TASK_ENABLE == 1)
#ifndef APOLLO5_FPGA
    //
    // Run MSPI DDR timing scan
    //
    am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
    am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
    {
        am_util_stdio_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_stdio_printf("==== Scan Result: Failed, no valid setting.  \n");
    }
#endif

    //
    // Configure the MSPI and PSRAM Device.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);

#ifndef APOLLO5_FPGA
    //
    // Apply DDR timing setting
    //
    ui32Status = am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pPsramHandle, &MSPIDdrTimingConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to apply the timming scan parameter!\n");
    }
#endif

    //
    // Enable XIP mode.
    //
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
    }

#ifdef CPU_RUN_IN_HP_MODE
    if ( am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE) != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("CPU enter HP mode failed!\n");
    }
#endif

#ifdef GPU_RUN_IN_HP_MODE
    am_hal_pwrctrl_periph_disable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //
    //Switch to HP mode.
    //
    am_hal_pwrctrl_gpu_mode_e current_mode;
    am_hal_pwrctrl_gpu_mode_select(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE);
    am_hal_pwrctrl_gpu_mode_status(&current_mode);
    if(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE != current_mode)
    {
        am_util_stdio_printf("gpu switch to HP mode failed!\n");
    }
#endif

    // Init LVGL.
    lv_init();

    //If NEMA_GFX_POWERSAVE is defined, we keep GPU power off until an GPU CL is ready to submit.
#ifndef NEMA_GFX_POWERSAVE
    //Power on GPU
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);

    //Initialize NemaGFX.
    int ret = nema_init();
    if (ret != 0)
    {
        am_util_stdio_printf("Ambiq GPU init failed!\n");
    }
  
#endif
#endif
    //
    // Initialize timer
    //
    am_hal_timer_config_t       TimerConfig;
    am_hal_timer_default_config_set(&TimerConfig);
    TimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;

    am_hal_timer_config(0, &TimerConfig);
    am_hal_timer_start(0);

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("\nRunning sensor_hub_test\n");

    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}
