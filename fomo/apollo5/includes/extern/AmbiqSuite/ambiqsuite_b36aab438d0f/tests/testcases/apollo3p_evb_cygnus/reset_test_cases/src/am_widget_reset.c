//*****************************************************************************
//
//! @file am_widget_reset.c
//!
//! @brief Test widget for testing reset.
//
//! This widget performs reset states testing.
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
#include "am_widget_reset.h"

//*****************************************************************************
//
// Widget Config parameters
//
//*****************************************************************************

//*****************************************************************************
//
// Global Variables.
//
//*****************************************************************************

//*****************************************************************************
//
// Reset ISR.
//
//*****************************************************************************
//
//! Take over default reset ISR.
//! Note - brownout is the rstgen interrupt.
//
void
am_brownout_isr(void)
{
    am_util_stdio_printf("BROWNOUT interrupt..\n");
    while (1);
}


//*****************************************************************************
//
// Interrupt enable test.
//
//*****************************************************************************
uint32_t
am_widget_reset_enable_int_test(void *pWidget, char *pErrStr)
{
    uint32_t ui32Ret;

    //
    // Enable, then disable the interrupt
    //
    ui32Ret = am_hal_reset_interrupt_enable(AM_HAL_RESET_INTERRUPT_BODH);

    if ( RSTGEN->INTEN != AM_HAL_RESET_INTERRUPT_BODH )
    {
        am_util_stdio_printf("am_widget_reset_enable_int_test() failed enable.");
        return AM_WIDGET_ERROR;
    }

    ui32Ret = am_hal_reset_interrupt_disable(AM_HAL_RESET_INTERRUPT_BODH);

    if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("am_widget_reset_enable_int_test() failed call to am_hal_reset_interrupt_disable().");
        return AM_WIDGET_ERROR;
    }

    if ( RSTGEN->INTEN != 0 )
    {
        am_util_stdio_printf("am_widget_reset_enable_int_test() failed, INT did not disable.");
        return AM_WIDGET_ERROR;
    }

    //
    // Exit with the interrupt enabled.
    //
    ui32Ret = am_hal_reset_interrupt_enable(AM_HAL_RESET_INTERRUPT_BODH);

    if ( ui32Ret != AM_HAL_STATUS_SUCCESS )
    {
        am_util_stdio_printf("am_widget_reset_enable_int_test() failed call to am_hal_reset_interrupt_enable().");
        return AM_WIDGET_ERROR;
    }

    if ( RSTGEN->INTEN != AM_HAL_RESET_INTERRUPT_BODH )
    {
        am_util_stdio_printf("am_widget_reset_enable_int_test() failed, INT did not enable.");
        return AM_WIDGET_ERROR;
    }

    //
    // Enable the Brownout interrupt
    //
    return AM_WIDGET_SUCCESS;
}

