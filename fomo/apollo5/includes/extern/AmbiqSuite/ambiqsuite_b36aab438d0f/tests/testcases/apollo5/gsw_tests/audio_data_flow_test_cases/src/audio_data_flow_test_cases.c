//*****************************************************************************
//
//! @file audio_data_flow_test.c
//!
//! @brief Run the audio streaming data flow test.
//!
//! This test includes eMMC, MSPI and UART tasks. We send data from the eMMC
//! Task to the MSPI Task and from the MSPI Task to the UART Task. (On Turbo
//! board please connect pins GP9 and GP10, for UART0 loopback.)
//!
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************


#include "audio_test_config.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_hal_pwrctrl.h"
#include "am_devices_mspi_psram_aps25616n.h"
#include "unity.h"

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
AM_SHARED_RW uint32_t        ui32DMATCBBuffer2[2560];
void            *g_pPsramHandle;
void            *g_pMSPIPsramHandle;

am_devices_mspi_psram_config_t g_sMspiPsramConfig =
{
#ifdef AM_PART_APOLLO4P
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE1,
#else
    .eDeviceConfig            = AM_HAL_MSPI_FLASH_HEX_DDR_CE0,
#endif
    .eClockFreq               = AM_HAL_MSPI_CLK_48MHZ,
    .ui32NBTxnBufLength       = sizeof(ui32DMATCBBuffer2) / sizeof(uint32_t),
    .pNBTxnBuf                = ui32DMATCBBuffer2,
    .ui32ScramblingStartAddr  = 0,
    .ui32ScramblingEndAddr    = 0,
};

//! MSPI interrupts.
static const IRQn_Type MspiInterrupts[] =
{
    MSPI0_IRQn,
#if defined(AM_PART_APOLLO3P) || defined(AM_PART_APOLLO4) || defined(AM_PART_APOLLO4B)
    MSPI1_IRQn,
    MSPI2_IRQn,
#endif
};

am_hal_mpu_region_config_t sMPUCfg =
{
    .ui32RegionNumber = 6,
    .ui32BaseAddress = (uint32_t)ui32DMATCBBuffer2,
    .eShareable = NON_SHARE,
    .eAccessPermission = RW_NONPRIV,
    .bExecuteNever = true,
    .ui32LimitAddress = (uint32_t)ui32DMATCBBuffer2 + sizeof(ui32DMATCBBuffer2) - 1,
    .ui32AttrIndex = 0,
    .bEnable = true,
};
am_hal_mpu_attr_t sMPUAttr =
{
    .ui8AttrIndex = 0,
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


//We need to set up a small heap to be used by Clib, delete this may cause unexpected error.
__attribute__ ((section(".heap")))
uint32_t clib_heap[100];

//*****************************************************************************
//
// Optional Global setup.
//
// globalSetUp() will get called before the test group starts, and
//
//*****************************************************************************
void
globalSetUp(void)
{
    uint32_t ui32Status;
    
    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Audio Data Flow Test Cases\n\n");

    //
    // Enable global IRQ.
    //
    am_hal_interrupt_master_enable();
	   
    //
    // Configure the MSPI and PSRAM Device.
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
    am_hal_mpu_region_configure(&sMPUCfg, 1);
    //
    // Invalidate and clear DCACHE, this is required by CM55 TRF.
    //
    am_hal_cachectrl_dcache_invalidate(NULL, true);
    //
    // MPU enable
    //
    am_hal_mpu_enable(true, true);	
	
#ifdef AM_PART_APOLLO5_API
#ifdef WINBOND_PSRAM
    ui32Status = am_devices_mspi_psram_w958d6nw_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
#endif
#else
    ui32Status = am_devices_mspi_psram_aps25616n_ddr_init(MSPI_PSRAM_MODULE, &g_sMspiPsramConfig, &g_pPsramHandle, &g_pMSPIPsramHandle);
#endif
    TEST_ASSERT_TRUE(AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS == ui32Status);
    if (AM_DEVICES_MSPI_PSRAM_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure the MSPI and PSRAM Device correctly!\n");
    }
    NVIC_SetPriority(MspiInterrupts[MSPI_PSRAM_MODULE], PSRAM_ISR_PRIORITY);
    NVIC_EnableIRQ(MspiInterrupts[MSPI_PSRAM_MODULE]);
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
globalTearDown(void)
{
  // Global Teardown
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
// Main Function
//
//*****************************************************************************
int
audio_data_flow_test(void)
{
	
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

    PWRCTRL->SSRAMPWREN = PWRCTRL_SSRAMPWREN_PWRENSSRAM_ALL;  // enable all
    while(PWRCTRL->SSRAMPWRST == 0);

    //
    // Initialize timer
    //
    am_hal_timer_config_t       TimerConfig;
    am_hal_timer_default_config_set(&TimerConfig);
    TimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    TimerConfig.eFunction = AM_HAL_TIMER_FN_EDGE;

    am_hal_timer_config(0, &TimerConfig);
    am_hal_timer_start(0);

    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

