//*****************************************************************************
//
//! @file sample_test_cases.c
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

#include "unity.h"
#include "am_mcu_apollo.h"
#include "am_widget_ble.h"

//*****************************************************************************
//
// Globals
//
//*****************************************************************************
am_widget_ble_config_t sWidgetConfig;
void *pWidget;
char errStr[128];

//*****************************************************************************
//
// Test configuration variables.
//
//*****************************************************************************
am_hal_ble_config_t sTestConfig =
{
    .ui32SpiClkCfg = AM_HAL_BLE_HCI_CLK_DIV2,
    .ui32ReadThreshold = 4,
    .ui32WriteThreshold = 4,
    .ui32BleClockConfig = AM_HAL_BLE_CORE_MCU_CLK,
#if 0
    .ui32ClockDrift = 500,
    .ui32SleepClockDrift = 500,
    .bAgcEnabled = true,
    .bSleepEnabled = true,
#endif
    .bUseDefaultPatches = true,
};

//am_hal_ble_config_t sTestConfig = { _VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC), 4, 4};

uint32_t pui32ClkCfgs[] =
{
    AM_HAL_BLE_HCI_CLK_DIV2,
    AM_HAL_BLE_HCI_CLK_DIV4,
    AM_HAL_BLE_HCI_CLK_DIV8,
    AM_HAL_BLE_HCI_CLK_DIV16,
    AM_HAL_BLE_HCI_CLK_DIV32,
    AM_HAL_BLE_HCI_CLK_DIV64,
};

#if 0
// CAJ: DIV4 cases removed because they don't work in FPGA. We should re-test
// these in silicon.
uint32_t pui32ClkCfgs[] =
{
    // 746.3K
    (_VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC_DIV64)  |
     _VAL2FLD(BLEIF_CLKCFG_IOCLKEN, 1)                          |
     _VAL2FLD(BLEIF_CLKCFG_CLK32KEN, 1)),

    // 1.5M
    (_VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC_DIV32)  |
     _VAL2FLD(BLEIF_CLKCFG_IOCLKEN, 1)                          |
     _VAL2FLD(BLEIF_CLKCFG_CLK32KEN, 1)),

    // 3M
    (_VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC_DIV16)  |
     _VAL2FLD(BLEIF_CLKCFG_IOCLKEN, 1)                          |
     _VAL2FLD(BLEIF_CLKCFG_CLK32KEN, 1)),

    // 4M
    (_VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC_DIV4)   |
     BLEIF_CLKCFG_DIV3_Msk                                      |
     _VAL2FLD(BLEIF_CLKCFG_IOCLKEN, 1)                          |
     _VAL2FLD(BLEIF_CLKCFG_CLK32KEN, 1)),

    // 4M
    (_VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC_DIV8)   |
     _VAL2FLD(BLEIF_CLKCFG_IOCLKEN, 1)                          |
     _VAL2FLD(BLEIF_CLKCFG_CLK32KEN, 1)),

    // 8.3M
    (_VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC_DIV2)   |
     BLEIF_CLKCFG_DIV3_Msk                                      |
     _VAL2FLD(BLEIF_CLKCFG_IOCLKEN, 1)                          |
     _VAL2FLD(BLEIF_CLKCFG_CLK32KEN, 1)),

    // 12M
    (_VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC_DIV4)   |
     _VAL2FLD(BLEIF_CLKCFG_IOCLKEN, 1)                          |
     _VAL2FLD(BLEIF_CLKCFG_CLK32KEN, 1)),

    // 16M
    (_VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC)        |
     BLEIF_CLKCFG_DIV3_Msk                                      |
     _VAL2FLD(BLEIF_CLKCFG_IOCLKEN, 1)                          |
     _VAL2FLD(BLEIF_CLKCFG_CLK32KEN, 1)),

    // 24M
    (_VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC_DIV2)   |
     _VAL2FLD(BLEIF_CLKCFG_IOCLKEN, 1)                          |
     _VAL2FLD(BLEIF_CLKCFG_CLK32KEN, 1)),

    //(_VAL2FLD(BLEIF_CLKCFG_FSEL, BLEIF_CLKCFG_FSEL_HFRC)      |
    // _VAL2FLD(BLEIF_CLKCFG_IOCLKEN, 1)                        |
    // _VAL2FLD(BLEIF_CLKCFG_CLK32KEN, 1)),
};
#endif

#define NUM_CLK_CFG (sizeof(pui32ClkCfgs) / sizeof(uint32_t))

typedef struct
{
    uint32_t ui32ReadThreshold;
    uint32_t ui32WriteThreshold;
}
threshold_t;

threshold_t psTestThresholds[] =
{
    {16, 16},
    {20, 2},
    {15, 8},
    {2, 9},
    {20, 20},
    {4, 16},
    {1, 10},
    {3, 31},
    {12, 6},
    {27, 5},
    {25, 25},
};
#define NUM_THRESHOLDS (sizeof(psTestThresholds) / sizeof(threshold_t))

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
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo3 BLE Test Cases\n\n");

    //
    // Enable debug pins.
    //
    // 30.6 - SCLK
    // 31.6 - MISO
    // 32.6 - MOSI
    // 33.4 - CSN
    // 35.7 - SPI_STATUS
    // 41.1 - IRQ (not configured, also SWO)
    //
    const am_hal_gpio_pincfg_t g_AM_PIN_30_BLE_SCK    = { .uFuncSel = 6 };
    const am_hal_gpio_pincfg_t g_AM_PIN_31_BLE_MISO   = { .uFuncSel = 6 };
    const am_hal_gpio_pincfg_t g_AM_PIN_32_BLE_MOSI   = { .uFuncSel = 6 };
    const am_hal_gpio_pincfg_t g_AM_PIN_33_BLE_CSN    = { .uFuncSel = 4 };
    const am_hal_gpio_pincfg_t g_AM_PIN_35_BLE_STATUS = { .uFuncSel = 7 };
    //const am_hal_gpio_pincfg_t g_AM_PIN_41_BLE_IRQ    = { .uFuncSel = 1 };
    am_hal_gpio_pinconfig(30, g_AM_PIN_30_BLE_SCK);
    am_hal_gpio_pinconfig(31, g_AM_PIN_31_BLE_MISO);
    am_hal_gpio_pinconfig(32, g_AM_PIN_32_BLE_MOSI);
    am_hal_gpio_pinconfig(33, g_AM_PIN_33_BLE_CSN);
    am_hal_gpio_pinconfig(35, g_AM_PIN_35_BLE_STATUS);
    //am_hal_gpio_pinconfig(41, g_AM_PIN_41_BLE_IRQ);

    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_08);

    am_widget_ble_setup(&pWidget, errStr);

    am_hal_interrupt_master_enable();
}

void
globalTearDown(void)
{
}

//*****************************************************************************
//
// Make sure that a blocking read/write pair can be executed.
//
//*****************************************************************************
void
ble_test_blocking_transfer(void)
{
    bool testPassed = false;

    sTestConfig.ui32SpiClkCfg = pui32ClkCfgs[0];
    am_widget_ble_test_config(0, &sTestConfig);

    for (uint32_t i = 0; i < NUM_CLK_CFG; i++)
    {
        am_util_stdio_printf("Blocking test: %d\n", i);
        sTestConfig.ui32SpiClkCfg = pui32ClkCfgs[i];
        am_widget_ble_test_config(0, &sTestConfig);
        testPassed = am_widget_ble_test_blocking_hci(0,0);

        TEST_ASSERT_TRUE(testPassed);
    }
}

//*****************************************************************************
//
// Make sure that that a nonblocking read/write pair can be executed.
//
//*****************************************************************************
void
ble_test_nonblocking_transfer(void)
{
    bool testPassed = false;

    sTestConfig.ui32SpiClkCfg = pui32ClkCfgs[0];
    sTestConfig.ui32ReadThreshold = psTestThresholds[0].ui32ReadThreshold;
    sTestConfig.ui32WriteThreshold = psTestThresholds[0].ui32WriteThreshold;
    am_widget_ble_test_config(0, &sTestConfig);

    for (uint32_t i = 0; i < NUM_THRESHOLDS; i++)
    {
        am_util_stdio_printf("Non-blocking threshold test: %d\n", i);
        sTestConfig.ui32ReadThreshold = psTestThresholds[i].ui32ReadThreshold;
        sTestConfig.ui32WriteThreshold = psTestThresholds[i].ui32WriteThreshold;
        testPassed = am_widget_ble_test_nonblocking_hci(0,0);
    }

    sTestConfig.ui32SpiClkCfg = pui32ClkCfgs[0];
    sTestConfig.ui32ReadThreshold = psTestThresholds[0].ui32ReadThreshold;
    sTestConfig.ui32WriteThreshold = psTestThresholds[0].ui32WriteThreshold;

    for (uint32_t i = 0; i < NUM_CLK_CFG; i++)
    {
        am_util_stdio_printf("Non-blocking speed test: %d\n", i);
        sTestConfig.ui32SpiClkCfg = pui32ClkCfgs[i];
        am_widget_ble_test_config(0, &sTestConfig);
        testPassed = am_widget_ble_test_nonblocking_hci(0,0);

        TEST_ASSERT_TRUE(testPassed);
    }
}

//*****************************************************************************
//
// Make sure status bits and the corresponding interrupts are functioning
// properly.
//
//*****************************************************************************
void
ble_test_status(void)
{
    bool testPassed = false;

    testPassed = am_widget_ble_test_status_check(pWidget, errStr);

    TEST_ASSERT_TRUE(testPassed);
}

//*****************************************************************************
//
// To be implemented later.
//
// The BLE hardware is going to require some special software sequencing to
// actually shut down correctly. We need a test that makes sure this works
// correctly.
//
//*****************************************************************************
void
ble_test_shutdown_restart(void)
{
    bool testPassed = true;
    TEST_ASSERT_TRUE(testPassed);
}

// Command queue test?
// Patching test?
