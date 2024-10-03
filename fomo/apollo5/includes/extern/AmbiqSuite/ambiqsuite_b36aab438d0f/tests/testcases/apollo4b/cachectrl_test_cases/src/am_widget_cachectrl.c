//*****************************************************************************
//
//! @file am_widget_cachectrl.c
//!
//! @brief Test widget for testing CACHECTRL.
//
//! This widget performs a variety of operations to the CACHECTRL block.
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
#include "am_widget_cachectrl.h"

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
// CACHECTRL ISRs.
//
//*****************************************************************************
//
//! Take over default ISR for CACHECTRL.
//
void am_cachectrl_isr(void)
{
}

//*****************************************************************************
//
// Widget setup function.
//
//*****************************************************************************
uint32_t am_widget_cachectrl_test_setup(am_widget_cachectrl_config_t *pTestCfg,
                                      void **ppWidget, char *pErrStr)
{
    uint32_t ui32Status = 0;

    //
    // Return status.
    //
    return ui32Status;
}

uint32_t
am_widget_cachectrl_test_device_info(void *pWidget, void *pCfg, char *pErrStr)
{
    bool bTestPassed = true;
    if ( bTestPassed )
    {
        return AM_WIDGET_SUCCESS;
    }
    else
    {
        return AM_WIDGET_ERROR;
    }
}
