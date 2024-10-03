//*****************************************************************************
//
//! @file complex_test_cases.c
//!
//! @brief Customer application-like test case simultaneously exercising eMMC,
//!   XIP MSPI PSRAM, SDIO, graphics, DSPI, crypto, and CPU.
//!
//! Test setup
//! ----------
//!
//!   Apollo5 FPGA Turbo
//!   ------------------
//!   * Uncomment #define APOLLO5_FPGA in mcu/apollo5a/am_mcu_apollo.h and
//!      change the value to the FPGA image clock in MHz, then rebuild HAL
//!   * Set BOARD?=apollo5_fpga_turbo in example-defs.mk
//!   * Set up the FPGA and Turbo boards (power brick to the FPGA, Turbo
//!     plugged into the FPGA FMC, USB between the FPGA and your PC)
//!   * Plug the RS9116 WiFi eval board 10p ribbon into the Turbo SDIO1 header
//!   * Make sure the RS9116 main board is jumpered for 1.8V and not 3.3V
//!   * Connect a USB cable between your PC and the RS9116 eval board POWER jack
//!   * If using JLink OB, connect the JLink USB to your PC and remove J18 and J19
//!   * If using external JLink, install J19 and connect the JLink to DBG_HDR
//!   * Turn on the FPGA board and check for LEDs on Turbo and the RS9116 board
//!   * Update Turbo IOX fw to latest
//!   * On Turbo, allow touch interrupts by jumpering FMC header GP67 to
//!     J26 pin 5 (INTn)
//!   * Set the FPGA board DIP switches to 1011 to enable MRAM (see 
//!     https://ambiqmicro.atlassian.net/wiki/spaces/C/pages/3336241168/FPGA+DIP+switch+settings)
//!   * Load a DISP_MSPI_SDIO bitstream to the FPGA. This 8MHz SOF has been
//!     tested: https://ambiqm.sharepoint.com/:u:/r/sites/sws9/Shared%20Documents/FGPA_Emulation/Cayenne/Apollo5A/Turbo%20pre-silicon%20board%20-%20Shanghai/Final%20RTL%20-%20230307%20FPGA/cayenne_fpga_rtl0307_dm5_disp_mspi_sdio_test3a.sof?csf=1&web=1&e=A540qE
//!   * Press the CFG_SW button twice to set the IOX to mode 3 (DISP_MSPI_SDIO).
//!     LEDs should show 1100 (LED2 and LED3 on, 4&5 off)
//!   * Load git/oz.jdebug in Ozone. If using JLink OB, you will be warned about
//!     using Ozone in trial mode. Click through this warning, or use an
//!     external JLink Plus.
//!   * In Ozone, enable SWO:
//!       Tools-> Trace Settings, set Trace Source to SWO
//!       Unclick AUTO by SWO Frequency
//!       Type "1000000" in SWO Frequency and click OK
//!   * Click OK to download to the target and play button to run
//!
//!   Apollo5 EB
//!   ----------
//!   * Make sure that #define APOLLO5_FPGA in mcu/apollo5a/am_mcu_apollo.h is
//!     commented out. This is now the default.
//!   * Set BOARD?=apollo5_eb or apollo5_eb_revb in example-defs.mk
//!   * If using the Peripheral card rev 2.0, make sure that in
//!     boards/<your_board>/bsp/am_bsp.c you replace ".eIC = DISP_IC_RM69330,"
//!     with ".eIC = DISP_IC_CO5300," to support the different display controller.
//!   * Connect USB from MAIN USB to your PC. This will provide power and JLink
//!     OB. Note that JLink OB will show up in JLink tools even if your are
//!     using external JLink.
//!   * Set up the Peripheral Card with a display (TBD)
//!   * If using JLink OB, make sure that J29 is not jumpered
//!   * If using external JLink, jumper J29 and connec the JLink to J27
//!   * Plug the RS9116 WiFi eval board 10p ribbon into the Turbo SDIO1 header
//!   * Make sure the RS9116 main board is jumpered for 1.8V and not 3.3V
//!   * Connect a USB cable between your PC and the RS9116 eval board POWER jack
//!   * Turn on the board power (SW_ON)
//!   * Insure IOX is up to date
//!   * IOX should be in mode 1 (GSW). If not, press IOX_RSTn
//!   * Load git/oz.jdebug in Ozone. If using JLink OB, you will be warned about
//!     using Ozone in trial mode. Click through this warning, or use an
//!     external JLink Plus.
//!   * In Ozone, enable SWO:
//!       Tools-> Trace Settings, set Trace Source to SWO
//!       Unclick AUTO by SWO Frequency
//!       Type "1000000" in SWO Frequency and click OK
//!   * Click OK to download to the target and click the play button to run
//!
//!  Using the test
//!  --------------
//!  The test should run until a preset number of audio files have been played
//!  back. While the test is running, the display should show a ticking watch
//!  face while the SWO viewer shows the state of the various tasks. Swiping
//!  left and right should drag the watchface, and swiping far left should
//!  snap to a screen with a button that changes between blue and red when
//!  clicked.
//! 
//!  In order for this test to be considered passing, the watchface second hand
//!  ticks should be consistent and the touch swipe motion should be responsive.
//!
//!  Known issues
//!  ------------
//!  * HS GPU and CPU mode are disabled, which drastically impacts graphics
//!    performance. Will be re-enabled when LP mode init is debugged on SI
//!  * SDIO WiFi fails to configure at 48MHz SDR50 but works at 24MHz on the EB
//!  * Display is untested on the EB due to not yet having the Peripheral
//!    Boards. This will be tested hackishly with the Turbo board soon.
//!  * Unsure if LVGL is using GPU acceleration for all features. This is
//!    particularly evident in tearing and lag when the button changes
//!    colors.
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
#include "complex_test_cases.h"
#include "devices/am_devices_mspi_psram_aps25616n.c"

#include "nema_vg.h"
#include "nema_error.h"
#include "lv_ambiq_touch.h"
#include "lvgl.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
bool            g_bTestEnd = false;

uint32_t        ui32DMATCBBuffer[4096];

void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;
#ifndef APOLLO5_FPGA
am_devices_mspi_psram_ddr_timing_config_t MSPIDdrTimingConfig;
#endif

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
#ifdef APOLLO5_FPGA
    .eClockFreq               = AM_HAL_MSPI_CLK_250MHZ,
#else
    .eClockFreq               = AM_HAL_MSPI_CLK_96MHZ,
#endif //APOLLO5_FPGA
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type MspiInterrupts[] =
{
    MSPI0_IRQn,
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
    uint32_t      ui32Status;

#if !defined(APOLLO5_FPGA)
    am_hal_cachectrl_icache_enable();
    // dcache is causing corrupt frames in PlaybackTask, keep off until we
    // can debug further
    am_hal_cachectrl_dcache_disable();
#endif

#ifdef APOLLO5_FPGA
    // Use ARM sleep on M55 FPGA instead of Ambiq sleep
    PWRCTRL->CPUPWRCTRL_b.SLEEPMODE=1;
#endif // APOLLO5_FPGA

#ifndef APOLLO5_FPGA
    // Set GPIO102 high to enable VDD18 load switch
    am_hal_gpio_pinconfig(AM_BSP_GPIO_VDD18_SWITCH, g_AM_BSP_GPIO_VDD18_SWITCH);
    am_hal_gpio_output_set(AM_BSP_GPIO_VDD18_SWITCH);
#endif //APOLLO5_FPGA
    am_bsp_touch_als_enable();

#ifndef APOLLO5_FPGA
    am_util_stdio_printf("Starting MSPI DDR Timing Scan: \n");
    if ( AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == am_devices_mspi_psram_aps25616n_ddr_init_timing_check(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &MSPIDdrTimingConfig) )
    {
        am_util_stdio_printf("==== Scan Result: RXDQSDELAY0 = %d \n", MSPIDdrTimingConfig.ui32Rxdqsdelay);
    }
    else
    {
        am_util_stdio_printf("==== Scan Result: Failed, no valid setting.  \n");
        while(1);
    }
#endif

    //
    // Enable global IRQ.
    //
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);
    am_hal_interrupt_master_enable();

    lv_ambiq_touch_init();

#ifdef WINBOND_PSRAM
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
#endif

    TEST_ASSERT_TRUE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
        while(1){};
    }
    //
    // Configure the MSPI and PSRAM Device.
    //
#ifndef APOLLO5_FPGA
    //
    //  Set the DDR timing from previous scan.
    //
    am_devices_mspi_psram_aps25616n_apply_ddr_timing(g_pPsramHandle, &MSPIDdrTimingConfig);
#endif

    //
    // Enable XIP mode.
    //
#ifdef WINBOND_PSRAM
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_enable_xip(g_pPsramHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_enable_xip(g_pPsramHandle);
#endif

    TEST_ASSERT_TRUE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to enable XIP mode in the MSPI!\n");
        while(1){};
    }

#ifdef CPU_RUN_IN_HP_MODE

    ui32Status = am_hal_pwrctrl_mcu_mode_select(AM_HAL_PWRCTRL_MCU_MODE_HIGH_PERFORMANCE);
    TEST_ASSERT_TRUE(ui32Status == AM_HAL_STATUS_SUCCESS );

    if ( AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("CPU enter HP mode failed!\n");
        while(1){};
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

    TEST_ASSERT_TRUE(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE == current_mode);
    if(AM_HAL_PWRCTRL_GPU_MODE_HIGH_PERFORMANCE != current_mode)
    {
        am_util_stdio_printf("gpu switch to HP mode failed!\n");
        while(1){};
    }
#endif
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_DISP);

    // Init LVGL.
    lv_init();

    //If NEMA_GFX_POWERSAVE is defined, we keep GPU power off until an GPU CL is ready to submit.
#ifndef NEMA_GFX_POWERSAVE
    //Power on GPU
    am_hal_pwrctrl_periph_enable(AM_HAL_PWRCTRL_PERIPH_GFX);
    //Initialize NemaGFX.
    ui32Status = nema_init();
    TEST_ASSERT_TRUE(ui32Status == 0);
    if (ui32Status != 0)
    {
        am_util_stdio_printf("Ambiq GPU init failed!\n");
        while(1){};
    }

    //Initialize NemaGFX.
    nema_vg_init(LV_AMBIQ_FB_RESX, LV_AMBIQ_FB_RESY);
    TEST_ASSERT_TRUE(NEMA_ERR_NO_ERROR == nema_get_error());
    if (NEMA_ERR_NO_ERROR != nema_get_error())
    {
        am_util_stdio_printf("NemaVG init failed!\n");
        while(1){};
    }
#endif
}

void
globalTearDown(void)
{
}


//*****************************************************************************
//
// Complex Test
//
//*****************************************************************************

void complex_test(void)
{
    uint32_t ui32Status = 0;

    am_util_stdio_printf("\nRunning complex_test\r\n");

    //
    // Output clock configuration info
    //
    am_util_stdio_printf("\nconfigCPU_CLOCK_HZ=%dHz", configCPU_CLOCK_HZ);

    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }
}
