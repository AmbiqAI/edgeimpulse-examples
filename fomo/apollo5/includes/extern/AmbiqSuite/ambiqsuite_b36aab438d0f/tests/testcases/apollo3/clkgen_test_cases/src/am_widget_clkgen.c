//*****************************************************************************
//
//! @file am_widget_clkgen.c
//!
//! @brief Test widget for testing CLKGEN.
//
//! This widget performs a variety of operations to the CLKGEN block.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <string.h>
#include <ctype.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_widget_clkgen.h"

//*****************************************************************************
//
// Widget Config parameters
//
//*****************************************************************************

//*****************************************************************************
//
// Internal Data Structures
//
//*****************************************************************************

//*****************************************************************************
//
// Global Variables.
//
//*****************************************************************************

//*****************************************************************************
//
// CLKGEN ISRs.
//
//*****************************************************************************
//
//! Take over default ISR for CLKGEN.
//
void am_clkgen_isr(void)
{
}




//*****************************************************************************
//
// Widget setup function.
//
//*****************************************************************************
uint32_t am_widget_clkgen_test_setup(am_widget_clkgen_config_t *pTestCfg,
                                      void **ppWidget, char *pErrStr)
{
    uint32_t ui32Status = 0;

    //
    // Return status.
    //
    return ui32Status;
}




uint32_t
am_widget_clkgen_test_device_info(void *pWidget, void *pCfg, char *pErrStr)
{
    bool bTestPassed = true;
#if 0
    am_hal_clkgen_device_t sClkGenDevice;

    //
    //! Contains the HAL hardware information about the device.
    //

    am_hal_clkgen_info_get(AM_HAL_CLKGEN_INFO_DEVICEID, &sClkGenDevice);

    // Let's print the retrieved values.
    am_util_stdio_printf("ChipPN:  0x%08X\n"
                         "ChipRev: 0x%08X\n"
                         "SKU:     0x%08X\n"
                         "CHIPID0: 0x%08X\n"
                         "CHIPID1: 0x%08X\n",
        sClkGenDevice.ui32ChipPN,
        sClkGenDevice.ui32ChipRev,
        sClkGenDevice.ui32SKU,
        sClkGenDevice.ui32ChipID0,
        sClkGenDevice.ui32ChipID1,

    if ( (sClkGenDevice.ui32ChipPN & AM_REG_CLKGEN_CHIPPN_PARTNUM_PN_M) !=
            AM_REG_CLKGEN_CHIPPN_PARTNUM_APOLLO3 )
    {
        am_util_stdio_printf("NOT Apollo3!  PN=0x%x, expected 0x%x\n",
            (sClkGenDevice.ui32ChipPN & AM_REG_CLKGEN_CHIPPN_PARTNUM_PN_M) >>
                AM_REG_CLKGEN_CHIPPN_PARTNUM_PN_S,
                AM_REG_CLKGEN_CHIPPN_PARTNUM_APOLLO3 >>
                AM_REG_CLKGEN_CHIPPN_PARTNUM_PN_S);
        bTestPassed = false;
    }


    if ( AM_BFX(CLKGEN, CHIPREV, SIPART, sClkGenDevice.ui32ChipRev) !=
         AM_BFX(CLKGEN, CHIPPN, PARTNUM, AM_REG_CLKGEN_CHIPPN_PARTNUM_APOLLO3) )
    {
        am_util_stdio_printf("NOT Apollo3!  Failed SIPART (Silicon Part ID) (0x%X, expected 0x%X).\n",
            AM_BFX(CLKGEN, CHIPREV, SIPART, sClkGenDevice.ui32ChipRev),
            AM_ENUMX(CLKGEN, CHIPPN, PARNUM, APOLLO3) );
        bTestPassed = false;
    }

    if ( AM_BFX(CLKGEN, CHIPREV, REVMAJ, sClkGenDevice.ui32ChipRev) != 0x1 )
    {
        am_util_stdio_printf("Failed REVMAJ (0x%X, expected 0x%X).\n",
            AM_BFX(CLKGEN, CHIPREV, REVMAJ, sClkGenDevice.ui32ChipRev), 0x1);
        bTestPassed = false;
    }

    if ( AM_BFX(CLKGEN, CHIPREV, REVMIN, sClkGenDevice.ui32ChipRev) != 0x1 )
    {
        am_util_stdio_printf("Failed REVMIN (0x%X, expected 0x%X).\n",
            AM_BFX(CLKGEN, CHIPREV, REVMIN, sClkGenDevice.ui32ChipRev), 0x1);
        bTestPassed = false;
    }

    if ( AM_BFX(CLKGEN, CHIPREV, VENDORID, sClkGenDevice.ui32ChipRev) !=
        ('A' << 24) | ('B' << 16) | ('B' << 8) | ('Q' << 0) )
    {
        am_util_stdio_printf("Failed VENDORID, (0x%08X, expected 0x%08X).\n",
            AM_BFX(CLKGEN, CHIPREV, VENDORID, sClkGenDevice.ui32ChipRev),
            ('A' << 24) | ('B' << 16) | ('B' << 8) | ('Q' << 0) );
        bTestPassed = false;
    }

    if ( sClkGenDevice.ui32SKU  != 0x7 )
    {
        am_util_stdio_printf("Failed SKU, (0x%08X, expected 0x%08X).\n",
            sClkGenDevice.ui32ChipRev, 0x7 );
        bTestPassed = false;
    }

#endif

    if ( bTestPassed )
    {
        return AM_WIDGET_SUCCESS;
    }
    else
    {
        return AM_WIDGET_ERROR;
    }

}

