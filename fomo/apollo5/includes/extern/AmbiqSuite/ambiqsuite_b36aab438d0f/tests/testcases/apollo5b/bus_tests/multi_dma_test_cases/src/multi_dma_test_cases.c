//*****************************************************************************
//
//! @file multi_dma_test_cases.c
//!
//! @brief Trigger multipul DMA transfers using IOM, MSPI, ADC and AUDADC to
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
#include "golden_340_rgba565.h"
#include "golden_340_bgr24.h"
#include "nema_utils.h"
//*****************************************************************************
//
// Test defines
//
//*****************************************************************************
#define TEST_LOOP      (1000UL)
#define REG_READ_LOOP  (50)

#ifdef APOLLO5_FPGA
#define USE_ADC         0
#else
#define USE_ADC         1
#endif

#define ENABLE_IOM
#define ENABLE_PSRAM
#define ENABLE_FLASH
#define ENABLE_DISPLAY
#define ENABLE_SDIO

#define RESX            340
#define RESY            340
#define LOOP_CNT        100
//#define FORMAT    AM_DEVICES_MSPI_RM69330_COLOR_MODE_16BIT
#define FORMAT    AM_DEVICES_MSPI_RM69330_COLOR_MODE_24BIT

#if (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_24BIT)

AM_SHARED_RW uint8_t ui8ImageArray[RESX * RESX * 3];

#else //AM_DEVICES_MSPI_RM69330_COLOR_MODE_16BIT

AM_SHARED_RW uint8_t ui8ImageArray[RESX * RESX * 2];

#endif
#if 0
#define STIME_FREQUENCY										1500000.0f
//returns time in seconds
float nema_get_time(void)
{
#if defined(APOLLO5_FPGA) && (defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5A))
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
#endif

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
    // Global interrupt enable
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Debug pin config and enable.
    //
    am_hal_gpio_pinconfig(DEBUG_PIN_IOM4, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_IOM7, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_MSPI0, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_MSPI1, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_MSPI2, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_SDIO, am_hal_gpio_pincfg_output);

    am_hal_gpio_output_clear(DEBUG_PIN_IOM4);
    am_hal_gpio_output_clear(DEBUG_PIN_IOM7);
    am_hal_gpio_output_clear(DEBUG_PIN_MSPI0);
    am_hal_gpio_output_clear(DEBUG_PIN_MSPI1);
    am_hal_gpio_output_clear(DEBUG_PIN_MSPI2);
    am_hal_gpio_output_clear(DEBUG_PIN_SDIO);


    //
    // DAXI config
    // TODO: Check this setting.
    //
    //CPU->DAXICFG_b.BUFFERENABLE = CPU_DAXICFG_BUFFERENABLE_ONE; // only enable a single buffer

    //
    // Print the banner
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo5 mulit dma Test Cases\n\n");

}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
static uint32_t        ui32AlwaysOnRegBuffer[32];

static uint32_t        ui32AlwaysOnRegBase[] =
{
  0x40004000UL,
  0x40010000UL,
  0x40020000UL,
  0x40021000UL,
  0x40000000UL,
  0x40008800UL,
  0x40008000UL,
  0x40024000UL
};

//*****************************************************************************
//
// Read registers
//
//*****************************************************************************
void
always_on_reg_read(void)
{
#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_set(DEBUG_PIN_4);
#endif

    for (uint32_t j = 0; j < (sizeof(ui32AlwaysOnRegBase)/sizeof(uint32_t)); j++)
    {
        for (uint32_t k = 0; k < (sizeof(ui32AlwaysOnRegBuffer)/sizeof(uint32_t)); k++)
        {
            ui32AlwaysOnRegBuffer[k] = AM_REGVAL(ui32AlwaysOnRegBase[j]+k*sizeof(uint32_t));
        }
    }

#ifdef USE_DEBUG_PIN
    am_hal_gpio_output_clear(DEBUG_PIN_4);
#endif
}

//*****************************************************************************
//
// init display image
//
//*****************************************************************************
void
image_init()
{
#if (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_24BIT)
    memcpy(ui8ImageArray,(void*)golden_340_bgr24,sizeof(golden_340_bgr24));
#else //(FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_16BIT)
    memcpy(ui8ImageArray,(void*)golden_340_rgba565,sizeof(golden_340_rgba565));
#endif
}

//*****************************************************************************
//
// Do multiple concurrent DMA trasactions to stress the APB bus.
//
//*****************************************************************************
bool
multi_dma_test(void)
{
    bool bRet = true;
    uint32_t ui32TestCounter = 0;
    float start_time,stop_time,fps;
    uint16_t ui16Loop=0;

    am_hal_timer_config_t sTimerConfig;
    uint32_t ui32Status = am_hal_timer_default_config_set(&sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to initialize a timer configuration structure with default values!\n");
    }
    sTimerConfig.eInputClock = AM_HAL_TIMER_CLOCK_HFRC_DIV16;
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    ui32Status = am_hal_timer_config(0, &sTimerConfig);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to configure a timer!\n");
    }
    ui32Status = am_hal_timer_start(0);
    if (AM_HAL_STATUS_SUCCESS != ui32Status)
    {
        am_util_stdio_printf("Failed to start a timer!\n");
    }

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
    // color format
    //
#if (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_24BIT)
    am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB888;
#elif (FORMAT == AM_DEVICES_MSPI_RM69330_COLOR_MODE_16BIT)
    am_devices_disp_color_e eDispColor = COLOR_FORMAT_RGB565;
#endif
    //
    // Initialize display
    //
    am_devices_display_init(RESX,
                            RESY,
                            eDispColor,
                            false);
#endif //defined(ENABLE_DISPLAY)

#if defined(ENABLE_SDIO)
    bRet = sdio_init();
    TEST_ASSERT_TRUE(bRet);
#endif


#if ( USE_ADC == 1 )
    bRet = adc_init();
    TEST_ASSERT_TRUE(bRet);
#endif

#if defined(APOLLO5_FPGA) && (defined(AM_PART_BRONCO) || defined(AM_PART_APOLLO5A))
	am_hal_stimer_config(AM_HAL_STIMER_HFRC_6MHZ | AM_HAL_STIMER_CFG_RUN);
#endif
    start_time = nema_get_time();
    while(ui32TestCounter < TEST_LOOP)
    {
#if defined(ENABLE_DISPLAY)
        static bool is_first_frame = true;
        if(is_first_frame == false)
        {
            //
            // wait transfer done
            //
            am_devices_display_wait_transfer_done();
        }
        //
        // transfer frame to the display
        //
        am_devices_display_transfer_frame(RESX,
                                          RESY,
                                          ( uint32_t )ui8ImageArray,
                                          NULL,
                                          NULL);
        is_first_frame = false;
        if(ui16Loop++ == LOOP_CNT)
        {
            stop_time = nema_get_time();
            fps = LOOP_CNT/(stop_time-start_time);
            am_util_stdio_printf("\nFPS: %.2f\n",fps);

            start_time = stop_time;
            ui16Loop = 0;
        }
#endif

#if defined(ENABLE_IOM)
        bRet = iom_start();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif
#if defined(ENABLE_PSRAM)
        bRet = psram_start();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif
#if defined(ENABLE_FLASH)
        bRet = flash_start();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif

#if defined(ENABLE_SDIO)
        bRet = sdio_write();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif
        for(uint32_t i=0; i < REG_READ_LOOP; i++)
        {
            always_on_reg_read();
        }

        //am_util_stdio_printf("loops :%d\n", ui32TestCounter);
        am_util_stdio_printf(".");

#if defined(ENABLE_FLASH)
        bRet = flash_wait();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif
#if defined(ENABLE_PSRAM)
        bRet = psram_wait();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif
#if defined(ENABLE_IOM)
        bRet = iom_wait();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif

#if defined(ENABLE_SDIO)
        bRet = sdio_read();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif

#if ( USE_ADC == 1 )
        bRet = adc_wait();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif

#if ( USE_ADC == 1 )
        bRet = adc_start();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif

        ui32TestCounter ++;

    } // while()

    return bRet;
}
