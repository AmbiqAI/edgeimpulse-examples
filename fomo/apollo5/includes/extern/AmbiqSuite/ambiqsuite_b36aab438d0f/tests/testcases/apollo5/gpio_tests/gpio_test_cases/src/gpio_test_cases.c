//*****************************************************************************
//
//! @file gpio_test_cases.c
//!
//! @brief The gpio tests conducted are
//!                    gpio_test_config - Test if GPIO pins can be configured.
//!                    gpio_test_connectivity - Test GPIO loopback connectivity
//!                              in pushpull and open drain configurations.
//!                    gpio_test_interrupt - Test GPIO pin interrupt.
//!@Setup - Use IOX mode 2 and the display card shoul not be connect to the
//!         Apollo5 eb board.
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
#include "am_widget_gpio.h"

#ifdef APOLLO5_FPGA

#define USE_GSW_IMGAE
//#define USE_DISP_MSPI_SDIO_IMAGE

#ifdef USE_GSW_IMGAE
#include "gsw_pinout.h"
#elif  USE_DISP_MSPI_SDIO_IMAGE
#include "disp_mspi_sdio_pinout.h"
#endif

#endif // APOLLO5_FPGA

// TODO - Consider enhancing the test by installing HardFault_Handler and adding
//        writes/reads from peripheral registers and memories as the devices
//        are enabled/disabled.

//*****************************************************************************
//
// Prototypes
//
//*****************************************************************************
uint32_t check_push_pull(uint32_t ui32Output, uint32_t ui32Input);
uint32_t check_open_drain(uint32_t ui32Output, uint32_t ui32Input);
uint32_t check_tristate(uint32_t ui32Output, uint32_t ui32Input);
uint32_t default_config_set();
static void banner(void);

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
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM output
    //
    if (am_bsp_debug_printf_enable())
    {
        // Cannot print - so no point proceeding
        while(1);
    }

    banner();

    // Set the default pin configuration for the pin pairs.
    default_config_set();
}

void
globalTearDown(void)
{
    // Set the default pin configuration for the pin pairs.
    default_config_set();
}

//*****************************************************************************
//
// Test cases for the main program to run.
//
//*****************************************************************************

//*****************************************************************************
//
// Pin configurations for the GPIO test.
//
//*****************************************************************************
const am_hal_gpio_pincfg_t sPushPull =
{
    .GP.cfg_b.uFuncSel          = 3,
    .GP.cfg_b.eGPOutCfg         = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
    .GP.cfg_b.eDriveStrength    = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.ePullup           = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.eGPInput          = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero         = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir           = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.uNCE              = 0,
    .GP.cfg_b.eCEpol            = 0,
    .GP.cfg_b.ePowerSw          = 0,
    .GP.cfg_b.eForceInputEn     = 0,
    .GP.cfg_b.eForceOutputEn    = 0,
    .GP.cfg_b.uRsvd_0           = 0,
    .GP.cfg_b.uRsvd_1           = 0,
};

const am_hal_gpio_pincfg_t sOpenDrain =
{
    .GP.cfg_b.uFuncSel          = 3,
    .GP.cfg_b.eGPOutCfg         = AM_HAL_GPIO_PIN_OUTCFG_OPENDRAIN,
    .GP.cfg_b.eDriveStrength    = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.ePullup           = AM_HAL_GPIO_PIN_PULLUP_1_5K,
    .GP.cfg_b.eGPInput          = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero         = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir           = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.uNCE              = 0,
    .GP.cfg_b.eCEpol            = 0,
    .GP.cfg_b.ePowerSw          = 0,
    .GP.cfg_b.eForceInputEn     = 0,
    .GP.cfg_b.eForceOutputEn    = 0,
    .GP.cfg_b.uRsvd_0           = 0,
    .GP.cfg_b.uRsvd_1           = 0,
};

const am_hal_gpio_pincfg_t sTristate =
{
    .GP.cfg_b.uFuncSel          = 3,
    .GP.cfg_b.eGPOutCfg         = AM_HAL_GPIO_PIN_OUTCFG_TRISTATE,
    .GP.cfg_b.eDriveStrength    = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.ePullup           = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.eGPInput          = AM_HAL_GPIO_PIN_INPUT_NONE,
    .GP.cfg_b.eGPRdZero         = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir           = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.uNCE              = 0,
    .GP.cfg_b.eCEpol            = 0,
    .GP.cfg_b.ePowerSw          = 0,
    .GP.cfg_b.eForceInputEn     = 0,
    .GP.cfg_b.eForceOutputEn    = 0,
    .GP.cfg_b.uRsvd_0           = 0,
    .GP.cfg_b.uRsvd_1           = 0,
};

const am_hal_gpio_pincfg_t sInput =
{
    .GP.cfg_b.uFuncSel          = 3,
    .GP.cfg_b.eGPOutCfg         = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength    = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.ePullup           = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.eGPInput          = AM_HAL_GPIO_PIN_INPUT_ENABLE,
    .GP.cfg_b.eGPRdZero         = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir           = AM_HAL_GPIO_PIN_INTDIR_LO2HI,
    .GP.cfg_b.uNCE              = 0,
    .GP.cfg_b.eCEpol            = 0,
    .GP.cfg_b.ePowerSw          = 0,
    .GP.cfg_b.eForceInputEn     = 0,
    .GP.cfg_b.eForceOutputEn    = 0,
    .GP.cfg_b.uRsvd_0           = 0,
    .GP.cfg_b.uRsvd_1           = 0,
};

typedef struct
{
    uint32_t pin1;
    uint32_t pin2;
}
gpio_loopback_t;


#ifdef APOLLO5_FPGA
const gpio_loopback_t LoopbackPairs[] =
{
    // The Turbo board pins connections, please refer https://ambiqmicro.atlassian.net/browse/CAYNSWS-482?focusedCommentId=82697
    // for how to connet. Besides, make sure these pins aren't connected to peripherals.

    // GP_68 ~ GP_71 are used by JLink
    // {GP_71, GP_69},
    // {GP_70, GP_68},
    {GP_67, GP_65},
    {GP_66, GP_64},
    {GP_63, GP_61},
    {GP_62, GP_60},
    {GP_59, GP_57},
    {GP_58, GP_56},
    {GP_51, GP_49},
    {GP_50, GP_48},
    {GP_43, GP_41},
    {GP_42, GP_40},
    {GP_37, GP_35},
    {GP_36, GP_34},
    #ifndef USE_GSW_IMGAE
    // GP_29 isn't used in GSW image
    {GP_29, GP_27},
    #endif // !USE_GSW_IMGAE
    {GP_28, GP_26},
    {GP_21, GP_19},
    {GP_20, GP_18},
    {GP_15, GP_11},
    {GP_14, GP_10},
    {GP_07, GP_05},
    {GP_06, GP_04},
    {GP_03, GP_01},
    {GP_02, GP_00},
    {GP_55, GP_72},
    {GP_54, GP_53},
    {GP_47, GP_52},
    {GP_46, GP_45},
    {GP_39, GP_44},
    {GP_38, GP_33},
    {GP_31, GP_32},
    {GP_30, GP_25},
    {GP_23, GP_24},
    {GP_22, GP_13},
    {GP_17, GP_12},
    {GP_16, GP_09},
    {GP_08, GP_73},
};
#else // APOLLO5_FPGA

const gpio_loopback_t LoopbackPairs[] =
{
    // J33
    { 95, 100},
    { 96, 101},
    { 97, 102},
    { 98, 103},
    { 99, 104},
    // J46
    {105, 110},
    {106, 111},
    {107, 112},
    {108, 113},
    // {109, 114}, // ECO for 114
    // J49
    {115, 120},
    {116, 121},
    {117, 122},
    {118, 123},
    {119, 124},
    // J52
    { 64,  69},
    { 65,  70},
    { 66,  71},
    { 67,  72},
    { 68,  73},
    // J54
    { 37,  42},
    { 38,  43},
    { 39,  44},
    { 40,  45},
    // { 41, 199}, // ECO for 199
    // J56
    { 74,  79},
    { 75,  80},
    { 76,  81},
    { 77,  82},
    { 78,  83},
    // J44
    {125, 130},
    {126, 131},
    {127, 132},
    {128, 133},
    {129, 134},
    // J47
    { 84, 156},
    { 85, 157},
    { 86, 158},
    { 87, 159},
    { 88, 160},
    // J48
    {  0,   1},
    {  2,  46},
    {  3,   4},
    {  8,   9},
    {  5,  52},
    // J50
    {  6,  10},
    {  7,  60},
    { 92,  29},
    { 58,  31},
    { 25,  32},
    // J53
    { 26,  33},
    { 27,  47},
    { 34,  48},
    { 35,  49},
    { 36, 139},
    // J57
    { 24,  89},
    { 62,  23},
    { 63,  22},
    { 93,  61},
    // { 94, GND}, // GND
    // J40
    {220, 148},
    {209, 149},
    {221, 150},
    {219, 151},
    {210, 152},
    // J42
    {153, 136},
    {154, 140},
    {155, 141},
    {222, 216},
    {223, 215},
    // J34
    { 16,  12},
    { 17,  13},
    { 18,  14},
    { 19,  15},
    { 50,  51},
    // J45
    {214, 186},
    {213, 187},
    {212, 188},
    {211, 218},
    {189, 217},
    // J51
    {143, 142},
    // {144, GND}, // GND
    {145,  53},
    {146,  54},
    {147,  55},
    // J39
    { 90, 208},
    { 91, 195},
    { 30, 200},
    // {GND, 138}, // GND
    // {GND, GND}, // GND
    // J41
    // {165, GND},   // GND
    // {SWDCK, SWO}, // SWD
    // {SWDIO,  59}, // SWD
    { 56,  11},
    // { 57, GND},   // GND

    // Jump wire
    { 41, 109},
    // {165,  59}, // GP165 is used by bootloader
    { 57, 144},
    { 94, 138},
};

#endif // !APOLLO5_FPGA

#define NUM_LOOPBACK_TESTS                                                  \
    (sizeof(LoopbackPairs) / sizeof(LoopbackPairs[0]))


typedef enum
{
    TC_GPIO_OK,
    TC_GPIO_READ_FAIL,
    TC_GPIO_RESET_FAIL,
    TC_GPIO_SET_FAIL
}tc_gpio_status_e;

//
// Certain pins can configure an extended drive strength. For these pins, the
// Slew Rate bit is overloaded to act as a third DS bit.
// This matrix identifies those pins with extended drive strength configuration.
// GPIOs: 37-45, 64-73, 105-124, 142-146.
//
static const uint32_t
g_ui32CfgDSExt[((AM_HAL_PIN_VIRTUAL_FIRST - 1) + 32) / 32] =
{
    0x00000000,     //  31:0
    0x00003FE0,     //  63:32,  set  45:37
    0x000003FF,     //  95:64,  set  73:64
    0x1FFBFE00,     // 127:96,  set 124:105
    0x0007C000,     // 159:128, set 146:142
    0x00000000,     // 191:160
    0x00000000      // 223:192
};

//
// GPIO drive strengths, the following table must be identical to the table
// in am_hal_gpio.c with the same identifier.
//
static const uint32_t
g_ui32DSpintbl[((AM_HAL_PIN_VIRTUAL_FIRST - 1) + 32) / 32] =
{
    0x8FC007E6,
    0xE3F3FFFF,
    0x81FFFFFF,
    0xFFFFFFFF,
    0xF00FC07F,
    0x00000001,
    0x00000189
};

//
// Apollo5's GPIO number may not consecutive, like Apollo4 Lite.
//
static const uint32_t
g_ui32removepintbl[((AM_HAL_PIN_VIRTUAL_FIRST - 1) + 32) / 32] =
{
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000,
    0x00000000
};

//*****************************************************************************
//
// Print out a banner and the pin pairs.
//
//*****************************************************************************
static void banner(void)
{
    uint32_t ux;

    am_util_stdio_printf("\nGPIO test cases\n");
    am_util_stdio_printf("Pin pairs:\n");

    for (ux = 0; ux < NUM_LOOPBACK_TESTS; ux++ )
    {
        am_util_stdio_printf(" [%2d,%2d]\n",
                             LoopbackPairs[ux].pin1, LoopbackPairs[ux].pin2);
    }
    am_util_stdio_printf("\n");
} // banner()


//*****************************************************************************
//
// Set the dafault config for the pin pairs.
//
//*****************************************************************************
uint32_t default_config_set()
{
    uint32_t ux,ui32TestRet;
    am_hal_gpio_pincfg_t pincfg_orig;

    pincfg_orig.GP.cfg = 0x00000003;

    for (ux = 0; ux < NUM_LOOPBACK_TESTS; ux++ )
    {
        ui32TestRet = am_hal_gpio_pinconfig(LoopbackPairs[ux].pin1, pincfg_orig);
        TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
        ui32TestRet = am_hal_gpio_pinconfig(LoopbackPairs[ux].pin2, pincfg_orig);
        TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
    }

    return ui32TestRet;
}



//*****************************************************************************
//
// Test cases for the main program to run.
//
//*****************************************************************************

void
gpio_test_config(void)
{
    uint32_t ux, ui32TestRet;
    am_hal_gpio_pincfg_t pincfg_orig;
    am_hal_gpio_pincfg_t test_pincfg_output = AM_HAL_GPIO_PINCFG_OUTPUT;

    am_util_stdio_printf("\ngpio_test_config():\n");

    //
    // Test the Apollo5 additional drive strengths.
    // First, positive test - make sure we can set the appropriate pins.
    //
    am_util_stdio_printf(" Positive tests:\n");
    for (ux = 0; ux < AM_HAL_PIN_VIRTUAL_FIRST; ux++ )
    {
        //
        // Capture the current pin config so we can restore it later.
        //
        ui32TestRet = am_hal_gpio_pinconfig_get(ux, &pincfg_orig);
        if ((g_ui32removepintbl[ux / 32] & (1 << (ux % 32))) != 0)
        {
            TEST_ASSERT_FALSE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
            am_util_stdio_printf("  informational msg only: pin %3d is not available \n", ux);
            continue;
        }
        else
        {
            TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
        }

        if ( pincfg_orig.GP.cfg != 0x00000003 )
        {
            // Most of the pins will be configured to 3. Print out the ones that aren't.
            am_util_stdio_printf("  informational msg only: pin %3d cfg = 0x%08X\n", ux, pincfg_orig.GP.cfg);
        }

        //
        // Skip SWD/SWO pins, at least for now.
        // However, we could test and restore them since we have the original config.
        //
        if ( (ux == 20) || (ux == 21) || (ux == AM_BSP_GPIO_ITM_SWO) )
        {
            continue;
        }
        //
        // Skip pin used in bootloader
        //
        if (ux == 165)
        {
            continue;
        }
        //
        // Skip pins that can't be used as GPIO
        //
        if ((ux == 114) || (ux == 199))
        {
            continue;
        }

        test_pincfg_output.GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P1X;
        ui32TestRet = am_hal_gpio_pinconfig(ux, test_pincfg_output);
        TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);

        test_pincfg_output.GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X;
        ui32TestRet = am_hal_gpio_pinconfig(ux, test_pincfg_output);
        TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);

        if ((g_ui32DSpintbl[ux / 32] & (1 << (ux % 32))) != 0)
        {
            test_pincfg_output.GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P75X;
            ui32TestRet = am_hal_gpio_pinconfig(ux, test_pincfg_output);
            TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
            if ( ui32TestRet != AM_HAL_STATUS_SUCCESS )
            {
                am_util_stdio_printf(" Pos test1 fail pin %d\n", ux);
            }
        }

        if ((g_ui32DSpintbl[ux / 32] & (1 << (ux % 32))) != 0)
        {
            test_pincfg_output.GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_1P0X;
            ui32TestRet = am_hal_gpio_pinconfig(ux, test_pincfg_output);
            TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
            if ( ui32TestRet != AM_HAL_STATUS_SUCCESS )
            {
                am_util_stdio_printf(" Pos test2 fail pin %d\n", ux);
            }
        }

        //
        // Restore the original configuration of this pin.
        //
        ui32TestRet = am_hal_gpio_pinconfig(ux, pincfg_orig);
        TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
    }

    //
    // Negative testing.
    //
    am_util_stdio_printf(" Negative tests:\n");
    for (ux = 0; ux < AM_HAL_PIN_VIRTUAL_FIRST; ux++ )
    {
        //
        // Capture the current pin config so we can restore it later.
        //
        ui32TestRet = am_hal_gpio_pinconfig_get(ux, &pincfg_orig);
        if ((g_ui32removepintbl[ux / 32] & (1 << (ux % 32))) != 0)
        {
            TEST_ASSERT_FALSE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
            am_util_stdio_printf("  informational msg only: pin %3d is not available \n", ux);
            continue;
        }
        else
        {
            TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
        }

        if ( pincfg_orig.GP.cfg != 0x00000003 )
        {
            // Most of the pins will be configured to 3. Print out the ones that aren't.
            am_util_stdio_printf("  informational msg only: pin %3d cfg = 0x%08X\n", ux, pincfg_orig.GP.cfg);
        }

        //
        // Skip SWD/SWO pins, at least for now.
        // However, we could test and restore them since we have the original config.
        //
        if ( (ux == 20) || (ux == 21) || (ux == AM_BSP_GPIO_ITM_SWO) )
        {
            continue;
        }
        //
        // Skip pin used in bootloader
        //
        if (ux == 165)
        {
            continue;
        }
        //
        // Skip pins that can't be used as GPIO
        //
        if ((ux == 114) || (ux == 199))
        {
            continue;
        }

        if ((g_ui32DSpintbl[ux / 32] & (1 << (ux % 32))) == 0)
        {
            test_pincfg_output.GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P75X;
            ui32TestRet = am_hal_gpio_pinconfig(ux, test_pincfg_output);
            TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_INVALID_OPERATION);
            if ( ui32TestRet != AM_HAL_STATUS_INVALID_OPERATION )
            {
                am_util_stdio_printf(" Neg test1 fail pin %d\n", ux);
            }
        }

        if ((g_ui32DSpintbl[ux / 32] & (1 << (ux % 32))) == 0)
        {
            test_pincfg_output.GP.cfg_b.eDriveStrength = AM_HAL_GPIO_PIN_DRIVESTRENGTH_1P0X;
            ui32TestRet = am_hal_gpio_pinconfig(ux, test_pincfg_output);
            TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_INVALID_OPERATION);
            if ( ui32TestRet != AM_HAL_STATUS_INVALID_OPERATION )
            {
                am_util_stdio_printf(" Neg test2 fail pin %d\n", ux);
            }
        }

        //
        // Restore the original configuration of this pin.
        //
        ui32TestRet = am_hal_gpio_pinconfig(ux, pincfg_orig);
        TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
    }
}

//*****************************************************************************
//
// Test the functionality of two GPIO pins to show that the output operations
// work correctly.
//
//*****************************************************************************
void
gpio_test_connectivity(void)
{
    am_util_stdio_printf("\nTest GPIO connectivity\n");

    uint32_t i;
    for ( i = 0; i < NUM_LOOPBACK_TESTS; i++ )
    {
        gpio_loopback_t sPinPair = LoopbackPairs[i];

        am_util_stdio_printf("\nTesting Pin Pair pin1=%d, pin2=%d:\n",
                                            sPinPair.pin1, sPinPair.pin2);

        check_push_pull(sPinPair.pin1, sPinPair.pin2);
        check_push_pull(sPinPair.pin2, sPinPair.pin1);
        check_open_drain(sPinPair.pin1, sPinPair.pin2);
        check_open_drain(sPinPair.pin2, sPinPair.pin1);

#ifndef APOLLO5_FPGA
        check_tristate(sPinPair.pin1, sPinPair.pin2);
        check_tristate(sPinPair.pin2, sPinPair.pin1);
#endif

    }
}

//*****************************************************************************
//
// Toggle an output pin, and make sure a connected input pin toggles with it.
//
//*****************************************************************************
uint32_t
check_pin_state(uint32_t ui32DesiredState, uint32_t ui32Output, uint32_t ui32Input)
{
    uint32_t ui32PinStateIn;
    uint32_t ui32PinStateOut;
    uint32_t ui32CounterTimeout;
    ui32CounterTimeout = 0;

    while (ui32CounterTimeout < 100)
    {
        ui32CounterTimeout++;
        ui32PinStateIn = am_hal_gpio_input_read(ui32Input);
        ui32PinStateOut = am_hal_gpio_output_read(ui32Output);
        if ((ui32PinStateIn == ui32PinStateOut) && (ui32PinStateIn == ui32DesiredState))
        {
            am_util_stdio_printf("Waited %d cycles  ", ui32CounterTimeout);
            break;
        }
    }
    TEST_ASSERT_TRUE((ui32PinStateIn == ui32PinStateOut) && (ui32PinStateIn == ui32DesiredState));
    if (ui32PinStateOut != ui32DesiredState)
    {
        if (ui32DesiredState == 1)
        {
            am_util_stdio_printf("Set   -- SET FAIL!!  \n");
            return TC_GPIO_SET_FAIL;
        }
        else if (ui32DesiredState == 0)
        {
            am_util_stdio_printf("Reset -- RESET FAIL!!  ");
            return TC_GPIO_RESET_FAIL;
        }
    }
    if (ui32PinStateIn != ui32DesiredState)
    {
        if (ui32DesiredState == 1)
        {
            am_util_stdio_printf("Set   -- Read FAIL!!  \n");
        }
        else if (ui32DesiredState == 0)
        {
            am_util_stdio_printf("Reset -- Read FAIL!!  ");
        }

        return TC_GPIO_READ_FAIL;
    }
    if (ui32DesiredState == 1)
    {
        am_util_stdio_printf("Set   -- PASS!!  \n");
    }
    else
    {
        am_util_stdio_printf("ReSet -- PASS!!  \n");
    }
    return TC_GPIO_OK;
}

//*****************************************************************************
//
// Function - check_push_pull()
// Description - Configure Output GPIO pin as pushpull.
//               Toggle output pin, and make sure a connected input pin toggles with it.
// Input Parameters - ui32Output : GPIO Output Pin #
//                  - ui32Input :  GPIO Intput Pin #
// Return - ui32NumErrors : Return number of errors.
//
//*****************************************************************************
uint32_t
check_push_pull(uint32_t ui32Output, uint32_t ui32Input)
{
    uint32_t ui32NumErrors = 0;

    am_util_stdio_printf("\nPushPull Test :\n");

    uint32_t ui32write_val,ui32read_val,ui32read_status;
    uint32_t ui32TestRet;
    am_hal_gpio_pincfg_t test_pincfg_output = sPushPull;
    am_hal_gpio_pincfg_t test_pincfg_input = sInput;

    // Configure output
    ui32TestRet = am_hal_gpio_pinconfig(ui32Output,test_pincfg_output);
    TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);

    // Configure input
    ui32TestRet = am_hal_gpio_pinconfig(ui32Input,test_pincfg_input);
    TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);

    // Set output high, and make sure input reads high.
    ui32write_val = 1;
    ui32read_val = 0;
    // Using functions instead of Macros to test the gpio_state functions.
    am_hal_gpio_state_write(ui32Output, AM_HAL_GPIO_OUTPUT_SET);
    ui32read_status = am_hal_gpio_state_read(ui32Input,AM_HAL_GPIO_INPUT_READ,&ui32read_val);
    TEST_ASSERT_TRUE(ui32read_status == AM_HAL_STATUS_SUCCESS);
    am_util_stdio_printf("Pin = %d Set High --",ui32Output);
    if (ui32read_val == ui32write_val)
    {
        am_util_stdio_printf(" Pass\n");
    }
    else
    {
        am_util_stdio_printf(" Fail\n");
    }
    TEST_ASSERT_TRUE(ui32read_val == ui32write_val);

    // Set output low, and make sure input reads low.
    ui32write_val = 0;
    ui32read_val = 1;
    // Using functions instead of Macros to test the gpio_state functions.
    am_hal_gpio_state_write(ui32Output, AM_HAL_GPIO_OUTPUT_CLEAR);
    ui32read_status = am_hal_gpio_state_read(ui32Input,AM_HAL_GPIO_INPUT_READ,&ui32read_val);
    //ui32read_val = am_hal_gpio_input_read(ui32Input);
    am_util_stdio_printf("Pin = %d Set Low -- ",ui32Output);
    if (ui32read_val == ui32write_val)
    {
        am_util_stdio_printf(" Pass\n");
    }
    else
    {
        am_util_stdio_printf(" Fail\n");
    }
    TEST_ASSERT_TRUE(ui32read_val == ui32write_val);

    return ui32NumErrors;
}

//*****************************************************************************
//
// Function - check_open_drain()
// Description - Configure Output GPIO pin as open drain.
//               Toggle output pin, and make sure a connected input pin
//               toggles with it, for all the pullup values.
// Input Parameters - ui32Output : GPIO Output Pin #
//                  - ui32Input :  GPIO Intput Pin #
// Return - ui32NumErrors : Return number of errors.
//
//*****************************************************************************
uint32_t
check_open_drain(uint32_t ui32Output, uint32_t ui32Input)
{
    am_hal_gpio_pullup_e pullcfg;
    uint32_t ui32NumErrors = 0;
    uint32_t ui32write_val,ui32read_val;
    uint32_t ui32TestRet;
    am_hal_gpio_pincfg_t test_pincfg_output = sOpenDrain;
    am_hal_gpio_pincfg_t test_pincfg_input = sInput;

    for (pullcfg = AM_HAL_GPIO_PIN_PULLUP_1_5K; pullcfg <= AM_HAL_GPIO_PIN_PULLUP_100K; pullcfg++)
    {
        //
        // For HS pads, only check 50 kΩ pullup
        //
        if ((g_ui32CfgDSExt[ui32Output / 32] & (1 << (ui32Output % 32))) != 0)
        {
            if (pullcfg != AM_HAL_GPIO_PIN_PULLUP_50K)
            {
                continue;
            }
        }

        test_pincfg_output.GP.cfg_b.ePullup = pullcfg;

        switch(pullcfg)
        {
            case AM_HAL_GPIO_PIN_PULLUP_1_5K:
                am_util_stdio_printf("\nOpen Drain Test for pullup 1.5K \n");
                break;
            case AM_HAL_GPIO_PIN_PULLUP_6K:
                am_util_stdio_printf("\nOpen Drain Test for pullup 6K \n");
                break;
            case AM_HAL_GPIO_PIN_PULLUP_12K:
                am_util_stdio_printf("\nOpen Drain Test for pullup 12K \n");
                break;
            case AM_HAL_GPIO_PIN_PULLUP_24K:
                am_util_stdio_printf("\nOpen Drain Test for pullup 24K \n");
                break;
            case AM_HAL_GPIO_PIN_PULLUP_50K:
                am_util_stdio_printf("\nOpen Drain Test for pullup 50K \n");
                break;
            case AM_HAL_GPIO_PIN_PULLUP_100K:
                am_util_stdio_printf("\nOpen Drain Test for pullup 100K \n");
                break;
            default:
                break;
        }
        // Configure output
        ui32TestRet = am_hal_gpio_pinconfig(ui32Output,test_pincfg_output);
        TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);

        // Configure input
        ui32TestRet = am_hal_gpio_pinconfig(ui32Input,test_pincfg_input);
        TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);

        // Set output high, and make sure input reads high.
        ui32write_val = 1;
        ui32read_val = 0;
        am_hal_gpio_output_set(ui32Output);
        // Add delay for weaker pullups.
        am_util_delay_ms(1);
        ui32read_val = am_hal_gpio_input_read(ui32Input);
        am_util_stdio_printf("Pin = %d Set High -- ",ui32Output);
        if (ui32read_val == ui32write_val)
        {
            am_util_stdio_printf(" Pass\n");
        }
        else
        {
            am_util_stdio_printf(" Fail\n");
        }
        TEST_ASSERT_TRUE(ui32read_val == ui32write_val);

        // Set output low, and make sure input reads low.
        ui32write_val = 0;
        ui32read_val = 1;
        am_hal_gpio_output_toggle(ui32Output);
        ui32read_val = am_hal_gpio_input_read(ui32Input);
        am_util_stdio_printf("Pin = %d Set Low -- ",ui32Output);
        if (ui32read_val == ui32write_val)
        {
            am_util_stdio_printf(" Pass\n");
        }
        else
        {
            am_util_stdio_printf(" Fail\n");
        }
        TEST_ASSERT_TRUE(ui32read_val == ui32write_val);
    }

    return ui32NumErrors;
}

//*****************************************************************************
//
// Function - check_triatate()
// Description - Configure Output GPIO pin as tristate.Check if input pin is driven by pullup value.
//               Disable tristate on output pin.Toggle output pin, and make sure
//                                                connected input pin toggles with it.
// Input Parameters - ui32Output : GPIO Output Pin #
//                  - ui32Input :  GPIO Intput Pin #
// Return - ui32NumErrors : Return number of errors.
//
//*****************************************************************************
uint32_t
check_tristate(uint32_t ui32Output, uint32_t ui32Input)
{
    uint32_t ui32NumErrors = 0;

    am_util_stdio_printf("\nTristate Test :\n");

    uint32_t ui32write_val,ui32read_val;
    uint32_t ui32TestRet;
    am_hal_gpio_pincfg_t test_pincfg_output = sTristate;
    am_hal_gpio_pincfg_t test_pincfg_input = sInput;

    // Configure output pin.
    ui32TestRet = am_hal_gpio_pinconfig(ui32Output,test_pincfg_output);
    TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);

    // Configure input as 50K pull-up resistor.
    test_pincfg_input.GP.cfg_b.ePullup = AM_HAL_GPIO_PIN_PULLUP_50K;
    // Configure input pin.
    ui32TestRet = am_hal_gpio_pinconfig(ui32Input,test_pincfg_input);
    TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
    am_util_delay_ms(1);

    // Set high-Z on output pin.
    am_hal_gpio_output_tristate_output_dis(ui32Output);
    am_util_delay_ms(1);
    // Read input pin.
    // Output is high-Z so we do not check output,make sure input reads high.
    ui32read_val = 0;
    ui32read_val = am_hal_gpio_input_read(ui32Input);

    am_util_stdio_printf("Input Pin = %d configured as 50k Pull-up, Output pin tristated --",ui32Input);
    if (ui32read_val == 1)
    {
        am_util_stdio_printf(" Pass\n");
    }
    else
    {
        am_util_stdio_printf(" Fail\n");
    }
    TEST_ASSERT_TRUE(ui32read_val == 1);

    // Enabling push-pull on output pin by setting ENn to 1.
    am_hal_gpio_output_tristate_output_tog(ui32Output);
    am_util_delay_ms(1);
    // Drive output pin low.
    am_hal_gpio_output_clear(ui32Output);
    am_util_delay_ms(1);
    // Read input and output pins.
    ui32write_val = am_hal_gpio_output_read(ui32Output);
    ui32read_val = am_hal_gpio_input_read(ui32Input);
    // Check if input and output pins are looped back.
    am_util_stdio_printf("Output Pin = %d not tristated and driven low --",ui32Output);
    if (ui32read_val == 0 && ui32write_val == 0)
    {
        am_util_stdio_printf(" Pass\n");
    }
    else
    {
        am_util_stdio_printf(" Fail\n");
    }
    TEST_ASSERT_TRUE(ui32read_val == ui32write_val);

    // Set high-Z on output pin.
    am_hal_gpio_output_tristate_output_dis(ui32Output);
    am_util_delay_ms(1);
    // Configure input pin as 50K pull-down resisitor
    test_pincfg_input.GP.cfg_b.ePullup = AM_HAL_GPIO_PIN_PULLDOWN_50K;
    // Configure input pin.
    ui32TestRet = am_hal_gpio_pinconfig(ui32Input,test_pincfg_input);
    TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
    am_util_delay_ms(1);
    // Read input pin.
    // Output is high-Z so we do not check output,make sure input reads low.
    ui32read_val = 1;
    ui32read_val = am_hal_gpio_input_read(ui32Input);
    am_util_stdio_printf("Input pin = %d configured as 50K Pull-down, Output pin tristated --",ui32Input);
    if (ui32read_val == 0)
    {
        am_util_stdio_printf(" Pass\n");
    }
    else
    {
        am_util_stdio_printf(" Fail\n");
    }
    TEST_ASSERT_TRUE(ui32read_val == 0);

    // Enabling push-pull on output pin by setting ENn to 1.
    am_hal_gpio_output_tristate_output_en(ui32Output);
    am_util_delay_ms(1);

    // Drive output pin high.
    am_hal_gpio_output_set(ui32Output);
    am_util_delay_ms(1);
    // Read input and output pins.
    ui32write_val = am_hal_gpio_output_read(ui32Output);
    ui32read_val = am_hal_gpio_input_read(ui32Input);
    // Check if input and output pins are looped back.
    am_util_stdio_printf("Output Pin = %d not tristated and driven high --",ui32Output);
    if (ui32read_val == 1 && ui32write_val == 1)
    {
        am_util_stdio_printf(" Pass\n");
    }
    else
    {
        am_util_stdio_printf(" Fail\n");
    }
    TEST_ASSERT_TRUE(ui32read_val == ui32write_val);


    // Configuring Inout and output pins as inputs for safety, with no pullup/pulldown.
    test_pincfg_input.GP.cfg_b.ePullup = AM_HAL_GPIO_PIN_PULLUP_NONE;
    ui32TestRet = am_hal_gpio_pinconfig(ui32Input,test_pincfg_input);
    TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);
    ui32TestRet = am_hal_gpio_pinconfig(ui32Output,test_pincfg_input);
    TEST_ASSERT_TRUE(ui32TestRet == AM_HAL_STATUS_SUCCESS);

    return ui32NumErrors;
}




void
gpio_test_interrupt(void)
{
    uint32_t ux, ui32PinDirLoop;
    uint32_t ui32PinTestIntrpt, ui32PinDrive, ui32TestRet;
	uint8_t ui8LoopCount = 1;  //Setting default count as 1.
    am_hal_gpio_int_channel_e eChl;

    am_util_stdio_printf("\n\nTest GPIO interrupts:\n");


    for ( ui32PinDirLoop = 0; ui32PinDirLoop < 2; ui32PinDirLoop++ )
    {
        //
        // Run the tests in both directions.  The first pass uses one pin for
        // the pin to be interrupted, the other pin as the driving pin.
        // The second pass swaps the roles of the 2 pins.
        //
        for ( ux = 0; ux < NUM_LOOPBACK_TESTS; ux++ )
        {
            //
            // Run the tests for each pin pair
            //
            gpio_loopback_t sPinPair = LoopbackPairs[ux];

            if ( ui32PinDirLoop == 0 )
            {
                ui32PinTestIntrpt = sPinPair.pin1;
                ui32PinDrive      = sPinPair.pin2;
            }
            else
            {
                ui32PinTestIntrpt = sPinPair.pin2;
                ui32PinDrive      = sPinPair.pin1;
            }

            //
            // Run the 3 tests (LO2HI, HI2LO, and BOTH) for each channel.
            //
            for ( eChl = AM_HAL_GPIO_INT_CHANNEL_0; eChl <= AM_HAL_GPIO_INT_CHANNEL_1; eChl++ )
            {
                am_util_stdio_printf("\nTesting Ch%d pin=%d, drvpin=%d:\n",
                                     eChl, ui32PinTestIntrpt, ui32PinDrive);
                //
                // Test Ch0 NONE EDGE
                //
                ui8LoopCount = 3;  // Looping the interrupt 3 times.

                am_util_stdio_printf("None  edge  ");
                ui32TestRet = am_widget_gpio_int_check(ui32PinTestIntrpt, ui32PinDrive,
                                                       AM_HAL_GPIO_PIN_INTDIR_NONE,
                                                       eChl, ui8LoopCount);
                TEST_ASSERT_TRUE(ui32TestRet == 0);

                //
                // Test Ch0 LO2HI
                //
                ui8LoopCount = 3;  // Looping the interrupt 3 times.

                am_util_stdio_printf("Rising  edge  ");
                ui32TestRet = am_widget_gpio_int_check(ui32PinTestIntrpt, ui32PinDrive,
                                                       AM_HAL_GPIO_PIN_INTDIR_LO2HI,
                                                       eChl, ui8LoopCount);
                TEST_ASSERT_TRUE(ui32TestRet == 0);

                //
                // Test Ch0 HI2LO
                //
                ui8LoopCount = 3;  // Looping the interrupt 3 times.

                am_util_stdio_printf("Falling edge  ");
                ui32TestRet = am_widget_gpio_int_check(ui32PinTestIntrpt, ui32PinDrive,
                                                       AM_HAL_GPIO_PIN_INTDIR_HI2LO,
                                                       eChl, ui8LoopCount);
                TEST_ASSERT_TRUE(ui32TestRet == 0);

                //
                // Test Ch0 BOTH
                //
                ui8LoopCount = 5;  // Looping the interrupt 5 times.

                am_util_stdio_printf("BOTH   edges  ");
                ui32TestRet = am_widget_gpio_int_check(ui32PinTestIntrpt, ui32PinDrive,
                                                       AM_HAL_GPIO_PIN_INTDIR_BOTH,
                                                       eChl, ui8LoopCount);
                TEST_ASSERT_TRUE(ui32TestRet == 0);
            }
        }
    }
}
