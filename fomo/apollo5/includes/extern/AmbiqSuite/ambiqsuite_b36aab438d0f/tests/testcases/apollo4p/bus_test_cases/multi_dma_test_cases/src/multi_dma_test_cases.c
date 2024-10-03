//*****************************************************************************
//
//! @file multi_dma_test_cases.c
//!
//! @brief Trigger multiple DMA transfer using IOM, MSPI, ADC and AUDADC to
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

//*****************************************************************************
//
// Test defines
//
//*****************************************************************************
#define TEST_LOOP      (1000UL)
#define REG_READ_LOOP  (50)

#ifdef APOLLO4_FPGA
#define USE_ADC         0
#define USE_AUDADC      0
#else
#define USE_ADC         1
#define USE_AUDADC      1
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
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Debug pin config and enable.
    //
#ifdef USE_DEBUG_PIN
    am_hal_gpio_pinconfig(DEBUG_PIN_1, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_2, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_3, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_4, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_5, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_6, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_7, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_8, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_9, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_10, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_11, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(DEBUG_PIN_12, am_hal_gpio_pincfg_output);

    am_hal_gpio_output_clear(DEBUG_PIN_1);
    am_hal_gpio_output_clear(DEBUG_PIN_2);
    am_hal_gpio_output_clear(DEBUG_PIN_3);
    am_hal_gpio_output_clear(DEBUG_PIN_4);
    am_hal_gpio_output_clear(DEBUG_PIN_5);
    am_hal_gpio_output_clear(DEBUG_PIN_6);
    am_hal_gpio_output_clear(DEBUG_PIN_7);
    am_hal_gpio_output_clear(DEBUG_PIN_8);
    am_hal_gpio_output_clear(DEBUG_PIN_9);
    am_hal_gpio_output_clear(DEBUG_PIN_10);
    am_hal_gpio_output_clear(DEBUG_PIN_11);
    am_hal_gpio_output_clear(DEBUG_PIN_12);
#endif

    //
    // Print the banner
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4p Bus Test Cases\n\n");

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
  0x48000000UL,
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
// Do multiple concurrent DMA transactions to stress the APB bus.
//
//*****************************************************************************
bool
multi_dma_test(void)
{
    bool bRet = true;
    uint32_t ui32TestCounter = 0;

    bRet = iom_init();
    TEST_ASSERT_TRUE(bRet);

    bRet = psram_init();
    TEST_ASSERT_TRUE(bRet);

    bRet = flash_init();
    TEST_ASSERT_TRUE(bRet);

#if ( USE_ADC == 1 )
    bRet = adc_init();
    TEST_ASSERT_TRUE(bRet);
#endif

#if ( USE_AUDADC == 1 )
    bRet = audadc_init();
    TEST_ASSERT_TRUE(bRet);
#endif

    //
    // Loop TEST_LOOP times.
    //
    while(ui32TestCounter < TEST_LOOP)
    {

        bRet = iom_start();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }

        bRet = psram_start();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }

        bRet = flash_start();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }

        for(uint32_t i=0; i < REG_READ_LOOP; i++)
        {
            always_on_reg_read();
        }

        am_util_stdio_printf("Loop counter:%d\n", ui32TestCounter);

        bRet = flash_wait();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }

        bRet = psram_wait();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }

        bRet = iom_wait();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }

#if ( USE_ADC == 1 )
        bRet = adc_wait();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif

#if ( USE_AUDADC == 1 )
        bRet = audadc_wait();
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

#if ( USE_AUDADC == 1 )
        bRet = audadc_start();
        TEST_ASSERT_TRUE(bRet);
        if(!bRet)
        {
            break;
        }
#endif

        ui32TestCounter ++;

    }
    //
    // deinitialize peripherals before ending test.
    //

    //
    // deinitialize flash
    //
    bRet = flash_deinit();
    TEST_ASSERT_TRUE(bRet);
    //
    // deinitialize psram
    //
    bRet = psram_deinit();
    TEST_ASSERT_TRUE(bRet);

    //
    // deinitialize iom
    //
    bRet = iom_deinit();
    TEST_ASSERT_TRUE(bRet);

    return bRet;
}
