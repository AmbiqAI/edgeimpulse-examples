//*****************************************************************************
//
//! @file dma_stress_test_cases.c
//!
//! @brief This testcase is created only for internal manual test. Trigger
//!        multiple DMA transfers using IOM, MSPI, ADC and AUDADC to
//!        find possible data bus issues.
//!
//! Note: No data check is performed, so this testcase CAN'T find data errors.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "multi_dma.h"
#include "unity.h"
#include <setjmp.h>
#include "golden_340_bgr24.h"
//*****************************************************************************
//
// Test defines
//
//*****************************************************************************
#define TEST_LOOP      (1000UL)
#define REG_READ_LOOP  (50)

#define ENABLE_IOM
#define ENABLE_PSRAM
#define ENABLE_FLASH
#define ENABLE_DISPLAY
#define ENABLE_SDIO
#define ENABLE_ADC

#define RESX            340
#define RESY            340
#define LOOP_CNT        100
#define STIME_FREQUENCY	1500000.0f

AM_SHARED_RW uint8_t ui8ImageArray[RESX * RESX * 3];
float        g_DispStartTime, g_DispStopTime;
volatile bool         g_bDisplayDone = true;

//returns time in seconds
float get_time(void)
{
#if defined(APOLLO4_FPGA) && defined(AM_PART_APOLLO4L)
    //!<use stimer
    static uint32_t count_last =0;
    static float last_sec = 0;
    float full_scale_sec;
    float timer_offset;
    uint32_t ui32TimerCount = am_hal_stimer_counter_get();
    full_scale_sec = (float)0xFFFFFFFF / STIME_FREQUENCY;
    timer_offset = (float)ui32TimerCount / STIME_FREQUENCY;
    if (count_last > ui32TimerCount)
    {
    // overflow
        count_last = ui32TimerCount;
        last_sec += full_scale_sec + timer_offset;
        return last_sec;
    }
    else
    {
        count_last = ui32TimerCount;
        return last_sec + timer_offset;
    }
#else
    //!<use ctimer
	uint32_t ui32TimerCount = am_hal_timer_read(0); //!< This API only supports timer0.
    static uint32_t count_last = 0;
    static float last_sec = 0;
    float full_scale_sec;
    float timer_offset;
    float fTimerInputFreq;
    uint32_t ui32DivideRatio, ui32Timer0Clock;
    ui32Timer0Clock = TIMER->CTRL0_b.TMR0CLK;
    if ((ui32Timer0Clock >= 1) || (ui32Timer0Clock <= 5))
    {
        //
        // Calculate divide ratio
        // TIMER_CTRL0_TMR0CLK_HFRC_DIV16       = 1,
        // TIMER_CTRL0_TMR0CLK_HFRC_DIV64       = 2,
        // TIMER_CTRL0_TMR0CLK_HFRC_DIV256      = 3,
        // TIMER_CTRL0_TMR0CLK_HFRC_DIV1024     = 4,
        // TIMER_CTRL0_TMR0CLK_HFRC_DIV4K       = 5,
        //
        ui32DivideRatio = (1U << ((ui32Timer0Clock + 1) << 1));
    }
    else
    {
        return 0; //!< This API only supports HFRC as timer input clock.
    }
    fTimerInputFreq = (float)AM_HAL_CLKGEN_FREQ_MAX_HZ / (float)ui32DivideRatio;
    full_scale_sec = (float)0xFFFFFFFF / fTimerInputFreq;
    timer_offset = (float)ui32TimerCount / fTimerInputFreq;
    if (count_last > ui32TimerCount){
        // overflow
        count_last = ui32TimerCount;
        last_sec += full_scale_sec + timer_offset;
        return last_sec;
    } else {
        count_last = ui32TimerCount;
        return last_sec + timer_offset;
    }
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
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();
    //
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();
    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();
    //
    // Print the banner
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4l Bus Test Cases\n\n");
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// init display image
//
//*****************************************************************************
void
image_init()
{
    memcpy(ui8ImageArray,(void*)golden_340_bgr24,sizeof(golden_340_bgr24));
}

void
mspi_transfer_callback()
{
    g_bDisplayDone = true;
}
//*****************************************************************************
//
// Do multiple concurrent DMA trasactions to stress the APB DMA.
//
//*****************************************************************************
bool
dma_stress_test(void)
{
    bool bRet = true;
    float time = 0.0f;

#if defined(ENABLE_IOM)
    bRet = iom_init();
    TEST_ASSERT_TRUE(bRet);
#endif

#if defined(ENABLE_PSRAM)
    bRet = psram_init();
    TEST_ASSERT_TRUE(bRet);
#endif

#if defined(ENABLE_FLASH)
    bRet = flash_init();
    TEST_ASSERT_TRUE(bRet);
#endif

#if defined(ENABLE_DISPLAY)
    image_init();
    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            COLOR_FORMAT_RGB888,
                            false);
#endif //defined(ENABLE_DISPLAY)

#if defined(ENABLE_SDIO)
    bRet = sdio_init();
    TEST_ASSERT_TRUE(bRet);
#endif

#if defined(ENABLE_ADC) && !defined(APOLLO4_FPGA)
    bRet = adc_init();
    TEST_ASSERT_TRUE(bRet);
#endif

#if defined(APOLLO4_FPGA) && defined(AM_PART_APOLLO4L)
    am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);
#endif
#if defined(ENABLE_DISPLAY)
    g_DispStartTime = get_time();
#endif
    while (1)
    {
#if defined(ENABLE_DISPLAY)
        if (g_bDisplayDone == true)
        {
            g_DispStopTime = get_time();

            time = g_DispStopTime - g_DispStartTime;
            //
            // transfer frame to the display
            //
            am_devices_display_transfer_frame(RESX,
                                              RESY,
                                              ( uint32_t )ui8ImageArray,
                                              ( am_devices_disp_handler_t )mspi_transfer_callback,
                                              NULL);
            g_bDisplayDone = false;
            g_DispStartTime = get_time();
            am_util_stdio_printf("MSPI display: %f Seconds.\n", time);
        }
#endif

#if defined(ENABLE_IOM)
        if (g_bIomDmaFinish[4] && g_bIomDmaFinish[7])
        {
            time = g_IomStopTime[4] - g_IomStartTime[4];
            bRet = iom_start();
            TEST_ASSERT_TRUE(bRet);
            if(!bRet)
            {
                break;
            }
            am_util_stdio_printf("IOM: %f Seconds.\n", time);
        }
#endif
#if defined(ENABLE_PSRAM)
        if (g_bMspiPsramDMAFinish == true)
        {
            time = g_PsramStopTime - g_PsramStartTime;
            bRet = psram_start();
            TEST_ASSERT_TRUE(bRet);
            if(!bRet)
            {
                break;
            }
            am_util_stdio_printf("MSPI PSRAM: %f Seconds.\n", time);
        }
#endif
#if defined(ENABLE_FLASH)
        if (g_bMspiFlashDMAFinish == true)
        {
            time = g_FlashStopTime - g_FlashStartTime;
            bRet = flash_start();
            TEST_ASSERT_TRUE(bRet);
            if(!bRet)
            {
                break;
            }
            am_util_stdio_printf("MSPI flash: %f Seconds.\n", time);
        }
#endif

#if defined(ENABLE_SDIO)
        if (bAsyncWriteIsDone == true)
        {
            time = g_SdioStopTime - g_SdioStartTime;
            bRet = sdio_write();
            TEST_ASSERT_TRUE(bRet);
            if(!bRet)
            {
                break;
            }
            am_util_stdio_printf("SDIO: %f Seconds.\n", time);
        }
#endif

#if defined(ENABLE_ADC) && !defined(APOLLO4_FPGA)
        if (g_bADCDMAComplete == true)
        {
            time = g_AdcStopTime - g_AdcStartTime;
            bRet = adc_start();
            TEST_ASSERT_TRUE(bRet);
            if(!bRet)
            {
                break;
            }
            am_util_stdio_printf("ADC: %f Seconds.\n", time);
        }
#endif
    } // while()

    return bRet;
}
