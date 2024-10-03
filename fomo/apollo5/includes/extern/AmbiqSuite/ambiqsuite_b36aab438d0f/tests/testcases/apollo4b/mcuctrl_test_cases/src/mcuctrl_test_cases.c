//*****************************************************************************
//
//! @file mcuctrl_test_cases.c
//!
//! @brief MCUCTRL test cases.
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
#include "am_widget_mcuctrl.h"
#include "am_util.h"

//*****************************************************************************
//
// Local defines
//
//*****************************************************************************
#if defined(AM_PART_APOLLO4)
#define APOLLO4_SKU_EXPECTED    0x3FF
#elif defined(AM_PART_APOLLO4B)
#define APOLLO4_SKU_EXPECTED    0x3CF
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
    // Set the clock frequency.
    //

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
    //am_bsp_low_power_init();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo4 MCUCTRL Test Cases\n\n");
}

void
globalTearDown(void)
{
}


//*****************************************************************************
//
//  Test Case
//  Device Info
//
//*****************************************************************************
void
mcuctrl_test_device_info(void)
{
    bool bTestPass = true;
    am_hal_mcuctrl_device_t sMcuCtrlDevice;

#if 0
    // Note - the below test should probably be moved to the widget.
    am_widget_mcuctrl_test_device_info(0,0,0);
#endif

    //
    // Get the HAL hardware information about the device.
    //
    am_hal_mcuctrl_info_get(AM_HAL_MCUCTRL_INFO_DEVICEID, &sMcuCtrlDevice);

    // Let's print the retrieved values.
    am_util_stdio_printf("ChipPN:  0x%08X\n"
                         "ChipRev: 0x%08X\n"
                         "SKU:     0x%08X\n"
                         "CHIPID0: 0x%08X\n"
                         "CHIPID1: 0x%08X\n",
        sMcuCtrlDevice.ui32ChipPN,
        sMcuCtrlDevice.ui32ChipRev,
        sMcuCtrlDevice.ui32SKU,
        sMcuCtrlDevice.ui32ChipID0,
        sMcuCtrlDevice.ui32ChipID1);


    if ( (sMcuCtrlDevice.ui32ChipPN & (uint32_t)MCUCTRL_CHIPPN_PARTNUM_PN_M) !=
            MCUCTRL_CHIPPN_PARTNUM_APOLLO4 )
    {
        am_util_stdio_printf("NOT Apollo4!  PN=0x%x, expected 0x%x\n",
            (sMcuCtrlDevice.ui32ChipPN & (uint32_t)MCUCTRL_CHIPPN_PARTNUM_PN_M) >>
                MCUCTRL_CHIPPN_PARTNUM_PN_S,
                MCUCTRL_CHIPPN_PARTNUM_APOLLO4 >>
                MCUCTRL_CHIPPN_PARTNUM_PN_S);
        bTestPass = false;
    }

    if ( _FLD2VAL(MCUCTRL_CHIPREV_SIPART, sMcuCtrlDevice.ui32ChipRev) != 0xEBF )
    {
        am_util_stdio_printf("NOT Apollo3!  Failed SIPART (Silicon Part ID) (0x%08X, expected 0x%08X).\n",
            _FLD2VAL(MCUCTRL_CHIPREV_SIPART, sMcuCtrlDevice.ui32ChipRev), 0xEBF);
//          AM_ENUMX(MCUCTRL, CHIPPN, PARTNUM, APOLLO3) );
        bTestPass = false;
    }

#if defined(AM_PART_APOLLO4)
    if ( _FLD2VAL(MCUCTRL_CHIPREV_REVMAJ, sMcuCtrlDevice.ui32ChipRev) != MCUCTRL_CHIPREV_REVMAJ_A )
#elif defined(AM_PART_APOLLO4B)
    if ( _FLD2VAL(MCUCTRL_CHIPREV_REVMAJ, sMcuCtrlDevice.ui32ChipRev) != MCUCTRL_CHIPREV_REVMAJ_B )
#endif
    {
        am_util_stdio_printf("Failed REVMAJ (0x%X, expected 1-3).\n",
            _FLD2VAL(MCUCTRL_CHIPREV_REVMAJ, sMcuCtrlDevice.ui32ChipRev));
        bTestPass = false;
    }

    if ( (_FLD2VAL(MCUCTRL_CHIPREV_REVMIN, sMcuCtrlDevice.ui32ChipRev) != MCUCTRL_CHIPREV_REVMIN_REV0) &&
         (_FLD2VAL(MCUCTRL_CHIPREV_REVMIN, sMcuCtrlDevice.ui32ChipRev) != MCUCTRL_CHIPREV_REVMIN_REV1) &&
         (_FLD2VAL(MCUCTRL_CHIPREV_REVMIN, sMcuCtrlDevice.ui32ChipRev) != MCUCTRL_CHIPREV_REVMIN_REV2) )
    {
        am_util_stdio_printf("Failed REVMIN (0x%X, expected 0x%X).\n",
            _FLD2VAL(MCUCTRL_CHIPREV_REVMIN, sMcuCtrlDevice.ui32ChipRev), 0x1);
        bTestPass = false;
    }

    if ( sMcuCtrlDevice.ui32VendorID !=
         (('A' << 24) | ('M' << 16) | ('B' << 8) | ('Q' << 0)) )
    {
        am_util_stdio_printf("Failed VENDORID, (0x%08X, expected 0x%08X).\n",
            sMcuCtrlDevice.ui32VendorID,
            (('A' << 24) | ('M' << 16) | ('B' << 8) | ('Q' << 0)) );
        bTestPass = false;
    }

    if ( sMcuCtrlDevice.ui32SKU != APOLLO4_SKU_EXPECTED )
    {
        am_util_stdio_printf("Failed SKU, (0x%08X, expected 0x%08X).\n",
            sMcuCtrlDevice.ui32SKU, APOLLO4_SKU_EXPECTED );
        bTestPass = false;
    }

    TEST_ASSERT_TRUE(bTestPass);

    return;

} // mcuctrl_test_device_info()

//*****************************************************************************
//
//  Test Case
//  MCUCTRL status
//
//*****************************************************************************
bool
mcuctrl_test_status(void)
{
    bool bTestPass = true;
    am_hal_mcuctrl_status_t sStatus;

    if ( am_hal_mcuctrl_status_get(&sStatus) )
    {
        am_util_stdio_printf("Failed call to am_hal_mcuctrl_status_get().\n");
        bTestPass = false;
    }

    if ( sStatus.bDebuggerLockout )
    {
        am_util_stdio_printf("Fail: bbDebuggerLockout unexpected.\n");
        bTestPass = false;
    }

    if ( sStatus.bADCcalibrated )
    {
        am_util_stdio_printf("Fail: bADCcalibrated unexpected.\n");
        bTestPass = false;
    }

    if ( sStatus.bBattLoadEnabled )
    {
        am_util_stdio_printf("Fail: bBattLoadEnabled unexpected.\n");
        bTestPass = false;
    }

    if ( sStatus.bSecBootOnWarmRst )
    {
        am_util_stdio_printf("Fail: bSecBootOnWarmRst unexpected.\n");
        bTestPass = false;
    }

    if ( sStatus.bSecBootOnColdRst )
    {
        am_util_stdio_printf("Fail: bSecBootOnColdRst unexpected.\n");
        bTestPass = false;
    }

    //am_util_stdio_printf("End of mcuctrl_test_status().\n");

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // mcuctrl_test_status()

//*****************************************************************************
//
//  Test Case
//  MCUCTRL control
//
//*****************************************************************************
void
mcuctrl_test_control(void)
{
    bool bTestPass = true;
    uint32_t ui32RegVal;
    uint32_t ui32RegMsk, ui32RegExp;

    //
    // Enable external XTAL
    //
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_ENABLE, 0);

    ui32RegVal = MCUCTRL->XTALCTRL;
    ui32RegMsk =
        MCUCTRL_XTALCTRL_XTALCOMPPDNB_Msk   |
        MCUCTRL_XTALCTRL_XTALPDNB_Msk   |
        MCUCTRL_XTALCTRL_XTALCOMPBYPASS_Msk    |
        MCUCTRL_XTALCTRL_XTALCOREDISFB_Msk   |
        MCUCTRL_XTALCTRL_XTALSWE_Msk;

    ui32RegExp =
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALPDNB,       MCUCTRL_XTALCTRL_XTALPDNB_PWRUPCORE)        |
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALCOMPPDNB,   MCUCTRL_XTALCTRL_XTALCOMPPDNB_PWRUPCOMP)    |
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALCOMPBYPASS, MCUCTRL_XTALCTRL_XTALCOMPBYPASS_USECOMP)    |
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALCOREDISFB,  MCUCTRL_XTALCTRL_XTALCOREDISFB_EN)         |
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALSWE,        MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_EN);

    if ( (ui32RegVal & ui32RegMsk) != ui32RegExp )
    {
        am_util_stdio_printf("EXTCLK32K_ENABLE failed: expected=0x%08X, regval=0x%08X.\n", ui32RegExp, (ui32RegVal & ui32RegMsk));
        bTestPass = false;
    }

    //
    // Disable external XTAL
    //
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_DISABLE, 0);

    ui32RegVal = MCUCTRL->XTALCTRL;

    ui32RegExp =
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALPDNB,       MCUCTRL_XTALCTRL_XTALPDNB_PWRUPCORE)        |
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALCOMPPDNB,   MCUCTRL_XTALCTRL_XTALCOMPPDNB_PWRUPCOMP)    |
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALCOMPBYPASS, MCUCTRL_XTALCTRL_XTALCOMPBYPASS_USECOMP)    |
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALCOREDISFB,  MCUCTRL_XTALCTRL_XTALCOREDISFB_EN)          |
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALSWE,        MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_DIS);

    if ( (ui32RegVal & ui32RegMsk) != ui32RegExp )
    {
        am_util_stdio_printf("EXTCLK32K_DISABLE failed: expected=0x%08X, regval=0x%08X.\n", ui32RegExp, (ui32RegVal & ui32RegMsk));
        bTestPass = false;
    }

    TEST_ASSERT_TRUE(bTestPass);

    return;

} // mcuctrl_test_control()
