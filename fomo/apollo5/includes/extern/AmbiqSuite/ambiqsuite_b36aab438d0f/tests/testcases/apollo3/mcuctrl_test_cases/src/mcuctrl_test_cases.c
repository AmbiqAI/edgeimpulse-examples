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
#define APOLLO3_SKU_EXPECTED    0x3


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
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    //
    // Set the default cache configuration
    //
    //am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo3 MCUCTRL Test Cases\n\n");
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


    if ( ((sMcuCtrlDevice.ui32ChipPN & (uint32_t)MCUCTRL_CHIPPN_PARTNUM_PN_M) !=
            MCUCTRL_CHIPPN_PARTNUM_APOLLO3 ) &&
        ((sMcuCtrlDevice.ui32ChipPN & (uint32_t)MCUCTRL_CHIPPN_PARTNUM_PN_M) !=
            MCUCTRL_CHIPPN_PARTNUM_APOLLO3P ) )
    {
        am_util_stdio_printf("NOT Apollo3!  PN=0x%x, expected 0x%x\n",
            (sMcuCtrlDevice.ui32ChipPN & (uint32_t)MCUCTRL_CHIPPN_PARTNUM_PN_M) >>
                MCUCTRL_CHIPPN_PARTNUM_PN_S,
                MCUCTRL_CHIPPN_PARTNUM_APOLLO3 >>
                MCUCTRL_CHIPPN_PARTNUM_PN_S);
        bTestPass = false;
    }

    if ( _FLD2VAL(MCUCTRL_CHIPREV_SIPART, sMcuCtrlDevice.ui32ChipRev) != 0xECF )
    {
        am_util_stdio_printf("NOT Apollo3!  Failed SIPART (Silicon Part ID) (0x%08X, expected 0x%08X).\n",
            _FLD2VAL(MCUCTRL_CHIPREV_SIPART, sMcuCtrlDevice.ui32ChipRev), 0xECF);
//          AM_ENUMX(MCUCTRL, CHIPPN, PARTNUM, APOLLO3) );
        bTestPass = false;
    }

    if (( _FLD2VAL(MCUCTRL_CHIPREV_REVMAJ, sMcuCtrlDevice.ui32ChipRev) != MCUCTRL_CHIPREV_REVMAJ_A ) &&
        ( _FLD2VAL(MCUCTRL_CHIPREV_REVMAJ, sMcuCtrlDevice.ui32ChipRev) != MCUCTRL_CHIPREV_REVMAJ_B ) &&
        ( _FLD2VAL(MCUCTRL_CHIPREV_REVMAJ, sMcuCtrlDevice.ui32ChipRev) != MCUCTRL_CHIPREV_REVMAJ_C ))
    {
        am_util_stdio_printf("Failed REVMAJ (0x%X, expected 1-3).\n",
            _FLD2VAL(MCUCTRL_CHIPREV_REVMAJ, sMcuCtrlDevice.ui32ChipRev));
        bTestPass = false;
    }

    if ( (_FLD2VAL(MCUCTRL_CHIPREV_REVMIN, sMcuCtrlDevice.ui32ChipRev) != MCUCTRL_CHIPREV_REVMIN_REV0) &&
         (_FLD2VAL(MCUCTRL_CHIPREV_REVMIN, sMcuCtrlDevice.ui32ChipRev) != MCUCTRL_CHIPREV_REVMIN_REV1) )
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

    if ( sMcuCtrlDevice.ui32SKU != APOLLO3_SKU_EXPECTED )
    {
        am_util_stdio_printf("Failed SKU, (0x%08X, expected 0x%08X).\n",
            sMcuCtrlDevice.ui32SKU, APOLLO3_SKU_EXPECTED );
        bTestPass = false;
    }

    TEST_ASSERT_TRUE(bTestPass);

    return;

} // mcuctrl_test_device_info()




//*****************************************************************************
//
//  Test Case
//  Fault Info
//
//*****************************************************************************
bool
mcuctrl_test_fault_info(void)
{
    bool bTestPass = true;
    am_hal_mcuctrl_fault_t  sMcuCtrlFault;

    //
    // Get the HAL hardware information about the device.
    //
    am_hal_mcuctrl_info_get(AM_HAL_MCUCTRL_INFO_FAULT_STATUS, &sMcuCtrlFault);

    if ( sMcuCtrlFault.bICODE )
    {
        am_util_stdio_printf("Failed Fault Info ICODE (bICODE is true), ui32ICODE addr = 0x%X.\n",
                             sMcuCtrlFault.ui32ICODE);
        bTestPass = false;
    }

    if ( sMcuCtrlFault.bDCODE )
    {
        am_util_stdio_printf("Failed Fault Info DCODE (bDCODE is true), ui32DCODE addr = 0x%X.\n",
                             sMcuCtrlFault.ui32DCODE);
        bTestPass = false;
    }

    if ( sMcuCtrlFault.bSYS )
    {
        am_util_stdio_printf("Failed Fault Info SYS (bSYS is true), ui32SYS addr = 0x%X.\n",
                             sMcuCtrlFault.ui32DCODE);
        bTestPass = false;
    }

    TEST_ASSERT_TRUE(bTestPass);

    return bTestPass;

} // mcuctrl_test_fault_info()


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

    if ( sStatus.bBurstAck )
    {
        am_util_stdio_printf("Fail: bBurstAck unexpected.\n");
        bTestPass = false;
    }

    if ( !sStatus.bBLEAck )
    {
        am_util_stdio_printf("Fail: bBLEAck expected.\n");
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
    uint32_t ui32Ret, ui32Control, ui32RegVal;
#ifndef APOLLO3P_FPGA
    uint32_t ui32RegMsk, ui32RegExp;
#endif // APOLLO3P_FPGA

    //
    // Enable fault capture
    //
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_ENABLE, 0);

    if ( MCUCTRL->FAULTCAPTUREEN_b.FAULTCAPTUREEN != 1 )
    {
        am_util_stdio_printf("Failed, enable of fault capture failed.\n");
        bTestPass = false;
    }

    if ( !mcuctrl_test_status() )
    {
        am_util_stdio_printf("Failed status check after enable of fault capture.\n");
        bTestPass = false;
    }

    if ( !mcuctrl_test_fault_info() )
    {
        am_util_stdio_printf("Failed test fault info after enable of fault capture.\n");
        bTestPass = false;
    }

    //
    // Disable fault capture
    //
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_FAULT_CAPTURE_DISABLE, 0);

    if ( MCUCTRL->FAULTCAPTUREEN_b.FAULTCAPTUREEN  != 0 )
    {
        am_util_stdio_printf("Failed, disable of fault capture failed.\n");
        bTestPass = false;
    }

    if ( !mcuctrl_test_status() )
    {
        am_util_stdio_printf("Failed status check after disable of fault capture.\n");
        bTestPass = false;
    }

    if ( !mcuctrl_test_fault_info() )
    {
        am_util_stdio_printf("Failed test fault info after disable of fault capture.\n");
        bTestPass = false;
    }

    //
    // Enable external XTAL
    //
#ifndef APOLLO3P_FPGA
    am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_EXTCLK32K_ENABLE, 0);

    ui32RegVal = MCUCTRL->XTALCTRL;
    ui32RegMsk =
        MCUCTRL_XTALCTRL_PDNBCMPRXTAL_Msk   |
        MCUCTRL_XTALCTRL_PDNBCOREXTAL_Msk   |
        MCUCTRL_XTALCTRL_BYPCMPRXTAL_Msk    |
        MCUCTRL_XTALCTRL_FDBKDSBLXTAL_Msk   |
        MCUCTRL_XTALCTRL_XTALSWE_Msk;

    ui32RegExp =
        _VAL2FLD(MCUCTRL_XTALCTRL_PDNBCMPRXTAL, MCUCTRL_XTALCTRL_PDNBCMPRXTAL_PWRDNCOMP)    |
        _VAL2FLD(MCUCTRL_XTALCTRL_PDNBCOREXTAL, MCUCTRL_XTALCTRL_PDNBCOREXTAL_PWRDNCORE)    |
        _VAL2FLD(MCUCTRL_XTALCTRL_BYPCMPRXTAL,  MCUCTRL_XTALCTRL_BYPCMPRXTAL_BYPCOMP)       |
        _VAL2FLD(MCUCTRL_XTALCTRL_FDBKDSBLXTAL, MCUCTRL_XTALCTRL_FDBKDSBLXTAL_DIS)          |
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALSWE,      MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_EN);

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
        _VAL2FLD(MCUCTRL_XTALCTRL_PDNBCMPRXTAL, MCUCTRL_XTALCTRL_PDNBCMPRXTAL_PWRUPCOMP)    |
        _VAL2FLD(MCUCTRL_XTALCTRL_PDNBCOREXTAL, MCUCTRL_XTALCTRL_PDNBCOREXTAL_PWRUPCORE)    |
        _VAL2FLD(MCUCTRL_XTALCTRL_BYPCMPRXTAL,  MCUCTRL_XTALCTRL_BYPCMPRXTAL_USECOMP)       |
        _VAL2FLD(MCUCTRL_XTALCTRL_FDBKDSBLXTAL, MCUCTRL_XTALCTRL_FDBKDSBLXTAL_EN)           |
        _VAL2FLD(MCUCTRL_XTALCTRL_XTALSWE,      MCUCTRL_XTALCTRL_XTALSWE_OVERRIDE_DIS);

    if ( (ui32RegVal & ui32RegMsk) != ui32RegExp )
    {
        am_util_stdio_printf("EXTCLK32K_DISABLE failed: expected=0x%08X, regval=0x%08X.\n", ui32RegExp, (ui32RegVal & ui32RegMsk));
        bTestPass = false;
    }
#endif // APOLLO3P_FPGA

    //
    // SRAM Prefetch
    //

    //
    // Save original settings
    //
    ui32RegVal = MCUCTRL->SRAMMODE;

    //
    // The I/D CACHE prefetch must have the corresponding I/D prefetch set.
    // Therefore an attemtp to set only the ICACHE should fail.
    //
    ui32Control = AM_HAL_MCUCTRL_SRAM_PREFETCH_INSTRCACHE;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret == AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("1: SRAM_PREFETCH_INSTRCACHE 1 call failed %d.\n", ui32Ret);
        bTestPass = false;
    }

    // Nothing enabled.

    ui32Control = AM_HAL_MCUCTRL_SRAM_PREFETCH_INSTR;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("2: SRAM_PREFETCH_INSTR call failed %d.\n", ui32Ret);
        bTestPass = false;
    }
    else
    {
        if ( _FLD2VAL(MCUCTRL_SRAMMODE_IPREFETCH, MCUCTRL->SRAMMODE) == 0 )
        {
            am_util_stdio_printf("3: SRAM_PREFETCH_INSTR (enable) failed (0x%08X).\n", MCUCTRL->SRAMMODE);
            bTestPass = false;
        }
    }

    // Current state: SRAM I enabled.

    //
    // The I/D CACHE prefetch must have the corresponding I/D prefetch set.
    // Since we just set the Instr prefetch, we should be able to set the
    // instr cache prefetch.
    //
    ui32Control = AM_HAL_MCUCTRL_SRAM_PREFETCH_INSTRCACHE;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("4: SRAM_PREFETCH_INSTRCACHE 1 call failed %d.\n", ui32Ret);
        bTestPass = false;
    }
    else
    {
        if ( _FLD2VAL(MCUCTRL_SRAMMODE_IPREFETCH_CACHE, MCUCTRL->SRAMMODE) == 0 )
        {
            am_util_stdio_printf("5: SRAM_PREFETCH_INSTRCACHE 1 (enable) failed (0x%08X).\n", MCUCTRL->SRAMMODE);
            bTestPass = false;
        }
    }

    // Current state: SRAM I and ICACHE enabled.

    //
    // Disable instr prefetch.
    // We must either disable both prefetches (reg and cache),
    // or cache must already be disabled.
    //
    ui32Control = AM_HAL_MCUCTRL_SRAM_NOPREFETCH_INSTR;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret == AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("6: SRAM_NOPREFETCH_INSTR 1 call failed %d.\n", ui32Ret);
        bTestPass = false;
    }

    // Current state: SRAM I and ICACHE enabled.

    //
    // Disable instr prefetch AND instr cache.
    //
    ui32Control = AM_HAL_MCUCTRL_SRAM_NOPREFETCH_INSTR | AM_HAL_MCUCTRL_SRAM_NOPREFETCH_INSTRCACHE;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("7: SRAM_NOPREFETCH_INSTR 2 call failed %d.\n", ui32Ret);
        bTestPass = false;
    }
    else
    {
        if ( _FLD2VAL(MCUCTRL_SRAMMODE_IPREFETCH, MCUCTRL->SRAMMODE) == 1 )
        {
            am_util_stdio_printf("8: SRAM_NOPREFETCH_INSTR 2 (disable) failed (0x%08X).\n", MCUCTRL->SRAMMODE);
            bTestPass = false;
        }
    }

    // Current state: Nothing enabled.

    //
    // The I/D CACHE prefetch must have the corresponding I/D prefetch set.
    //
    ui32Control = AM_HAL_MCUCTRL_SRAM_PREFETCH_INSTRCACHE;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);

    // Call should fail without the regular I/D prefetch.
    if ( ui32Ret == AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("9: SRAM_PREFETCH_INSTRCACHE 2 call failed %d.\n", ui32Ret);
        bTestPass = false;
    }

    // Current state: Nothing enabled.

    //
    // Now try again with the I/D prefetch.
    //
    ui32Control = AM_HAL_MCUCTRL_SRAM_PREFETCH_INSTRCACHE | AM_HAL_MCUCTRL_SRAM_PREFETCH_INSTR;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("10: SRAM_PREFETCH_INSTRCACHE 3 call failed %d.\n", ui32Ret);
        bTestPass = false;
    }
    else
    {
        if ( _FLD2VAL(MCUCTRL_SRAMMODE_IPREFETCH_CACHE, MCUCTRL->SRAMMODE) == 0 )
        {
            am_util_stdio_printf("11: SRAM_PREFETCH_INSTRCACHE 3 (enable) failed (0x%08X).\n", MCUCTRL->SRAMMODE);
            bTestPass = false;
        }
    }

    // Current state: SRAM I and ICACHE enabled.

    //
    //
    ui32Control = AM_HAL_MCUCTRL_SRAM_NOPREFETCH_INSTRCACHE | AM_HAL_MCUCTRL_SRAM_NOPREFETCH_INSTR;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("12: SRAM_NOPREFETCH_INSTRCACHE call failed %d.\n", ui32Ret);
        bTestPass = false;
    }
    else
    {
        if ( _FLD2VAL(MCUCTRL_SRAMMODE_IPREFETCH_CACHE, MCUCTRL->SRAMMODE) == 1 )
        {
            am_util_stdio_printf("13: SRAM_PREFETCH_NOINSTRCACHE (disable) failed (0x%08X).\n", MCUCTRL->SRAMMODE);
            bTestPass = false;
        }
    }

    // Current state: nothing enabled.

    ui32Control = AM_HAL_MCUCTRL_SRAM_PREFETCH_DATA;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("14: SRAM_PREFETCH_DATA call failed %d.\n", ui32Ret);
        bTestPass = false;
    }
    else
    {
        if ( _FLD2VAL(MCUCTRL_SRAMMODE_DPREFETCH, MCUCTRL->SRAMMODE) == 0 )
        {
            am_util_stdio_printf("15: SRAM_PREFETCH_DATA (enable) failed (0x%08X).\n", MCUCTRL->SRAMMODE);
            bTestPass = false;
        }
    }

    // Current state: D enabled, DC disabled.

    ui32Control = AM_HAL_MCUCTRL_SRAM_NOPREFETCH_DATA;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("16: SRAM_NOPREFETCH_DATA call failed %d.\n", ui32Ret);
        bTestPass = false;
    }
    else
    {
        if ( _FLD2VAL(MCUCTRL_SRAMMODE_DPREFETCH, MCUCTRL->SRAMMODE) == 1 )
        {
            am_util_stdio_printf("17: SRAM_NOPREFETCH_DATA (disable) failed (0x%08X).\n", MCUCTRL->SRAMMODE);
            bTestPass = false;
        }
    }

    // Current state: D disabled, DC disabled.

    ui32Control = AM_HAL_MCUCTRL_SRAM_PREFETCH_DATACACHE;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret == AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("18: SRAM_PREFETCH_DATACACHE call failed %d.\n", ui32Ret);
        bTestPass = false;
    }

    // Current state: nothing enabled.

    ui32Control = AM_HAL_MCUCTRL_SRAM_PREFETCH_DATACACHE | AM_HAL_MCUCTRL_SRAM_PREFETCH_DATA;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("19: SRAM_PREFETCH_DATACACHE call failed %d.\n", ui32Ret);
        bTestPass = false;
    }
    else
    {
        if ( _FLD2VAL(MCUCTRL_SRAMMODE_DPREFETCH_CACHE, MCUCTRL->SRAMMODE) == 0 )
        {
            am_util_stdio_printf("20: SRAM_PREFETCH_DATACACHE (enable) failed (0x%08X).\n", MCUCTRL->SRAMMODE);
            bTestPass = false;
        }
    }

    // Current state: D enabled, DC enabled.

    ui32Control = AM_HAL_MCUCTRL_SRAM_NOPREFETCH_DATACACHE;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("21: SRAM_NOPREFETCH_DATACACHE call failed %d.\n", ui32Ret);
        bTestPass = false;
    }
    else
    {
        if ( _FLD2VAL(MCUCTRL_SRAMMODE_DPREFETCH_CACHE, MCUCTRL->SRAMMODE) == 1 )
        {
            am_util_stdio_printf("22: SRAM_NOPREFETCH_DATACACHE (disable) failed (0x%08X).\n", MCUCTRL->SRAMMODE);
            bTestPass = false;
        }
    }

    // Current state: D enabled, DC disabled.

    ui32Control = AM_HAL_MCUCTRL_SRAM_NOPREFETCH_DATA;
    ui32Ret = am_hal_mcuctrl_control(AM_HAL_MCUCTRL_CONTROL_SRAM_PREFETCH, &ui32Control);
    if ( ui32Ret != AM_HAL_STATUS_SUCCESS)
    {
        am_util_stdio_printf("23: SRAM_NOPREFETCH_DATA call failed %d.\n", ui32Ret);
        bTestPass = false;
    }
    else
    {
        if ( _FLD2VAL(MCUCTRL_SRAMMODE_DPREFETCH_CACHE, MCUCTRL->SRAMMODE) == 1 )
        {
            am_util_stdio_printf("24: SRAM_NOPREFETCH_DATA (disable) failed (0x%08X).\n", MCUCTRL->SRAMMODE);
            bTestPass = false;
        }
    }

    //
    // Restore original settings
    //
    MCUCTRL->SRAMMODE = ui32RegVal;


    TEST_ASSERT_TRUE(bTestPass);

    return;

} // mcuctrl_test_control()
