//*****************************************************************************
//
//! @file am_widget_mcuctrl.c
//!
//! @brief Test widget for testing MCUCTRL.
//
//! This widget performs a variety of operations to the MCUCTRL block.
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
#include "am_widget_mcuctrl.h"

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
// MCUCTRL ISRs.
//
//*****************************************************************************
//
//! Take over default ISR for MCUCTRL.
//
void am_mcuctrl_isr(void)
{
}




//*****************************************************************************
//
// Widget setup function.
//
//*****************************************************************************
uint32_t am_widget_mcuctrl_test_setup(am_widget_mcuctrl_config_t *pTestCfg,
                                      void **ppWidget, char *pErrStr)
{
    uint32_t ui32Status = 0;

    //
    // Return status.
    //
    return ui32Status;
}


uint32_t
am_widget_mcuctrl_test_device_info(void *pWidget, void *pCfg, char *pErrStr)
{
    bool bTestPass = true;

    return bTestPass ? AM_WIDGET_SUCCESS : AM_WIDGET_ERROR;
}

