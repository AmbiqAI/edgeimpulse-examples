//*****************************************************************************
//
//! @file am_widget_gpio.c
//!
//! @brief This widget allows test cases to exercise the Apollo4 GPIO HAL.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_widget_gpio.h"
#include "am_bsp.h"
#include "am_util.h"
#include "unity.h"

volatile bool bIntServiced = false;
volatile bool bIRQStatusGet = false;
volatile bool bIRQClear = false;

//*****************************************************************************
//
// Look for errors in the HAL.
//
//*****************************************************************************


//*****************************************************************************
//
// Tracking variable for error status.
//
//*****************************************************************************
uint32_t g_ui32ErrorStatus = 0;

uint32_t g_ui32ch0_001f = 0;
uint32_t g_ui32ch0_203f = 0;
uint32_t g_ui32ch0_405f = 0;
uint32_t g_ui32ch0_607f = 0;
uint32_t g_ui32ch0_809f = 0;
uint32_t g_ui32ch0_a0bf = 0;
uint32_t g_ui32ch0_c0df = 0;
uint32_t g_ui32ch1_001f = 0;
uint32_t g_ui32ch1_203f = 0;
uint32_t g_ui32ch1_405f = 0;
uint32_t g_ui32ch1_607f = 0;
uint32_t g_ui32ch1_809f = 0;
uint32_t g_ui32ch1_a0bf = 0;
uint32_t g_ui32ch1_c0df = 0;

//*****************************************************************************
//
// Pin configurations.
//
//*****************************************************************************
am_hal_gpio_pincfg_t g_GPIO_INPUT_INT =
{
    .GP.cfg_b.uFuncSel         = 3,
    .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_DISABLE,
    .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
    .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
    .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_ENABLE,
    .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
    .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_NONE,   // LO2HI, HI2LO, BOTH
    .GP.cfg_b.uNCE             = 0,
    .GP.cfg_b.eCEpol           = 0,
    .GP.cfg_b.ePowerSw         = 0,
    .GP.cfg_b.eForceInputEn    = 0,
    .GP.cfg_b.eForceOutputEn   = 0,
    .GP.cfg_b.uRsvd_0          = 0,
    .GP.cfg_b.uRsvd_1          = 0
};


am_hal_gpio_pincfg_t g_GPIO_OUTPUT_READ =
{
        .GP.cfg_b.uFuncSel         = 3,
        .GP.cfg_b.eGPOutCfg        = AM_HAL_GPIO_PIN_OUTCFG_PUSHPULL,
        .GP.cfg_b.eDriveStrength   = AM_HAL_GPIO_PIN_DRIVESTRENGTH_0P5X,
        .GP.cfg_b.ePullup          = AM_HAL_GPIO_PIN_PULLUP_NONE,
        .GP.cfg_b.eGPInput         = AM_HAL_GPIO_PIN_INPUT_NONE,
        .GP.cfg_b.eGPRdZero        = AM_HAL_GPIO_PIN_RDZERO_READPIN,
        .GP.cfg_b.eIntDir          = AM_HAL_GPIO_PIN_INTDIR_NONE,
        .GP.cfg_b.uNCE             = 0,
        .GP.cfg_b.eCEpol           = 0,
        .GP.cfg_b.ePowerSw         = 0,
        .GP.cfg_b.eForceInputEn    = 0,
        .GP.cfg_b.eForceOutputEn   = 0,
        .GP.cfg_b.uRsvd_0          = 0,
        .GP.cfg_b.uRsvd_1          = 0
};

//*****************************************************************************
//
// Macros to check return values.
//
//*****************************************************************************
//
// Macros to check for pinconfig errors.
//
#define CHKPINCFG(pin,ret)                                                          \
        if ( ret )                                                                  \
        {                                                                           \
            am_util_stdio_printf("GPIO %d: pinconfig error 0x%08x encountered.",    \
                                 pin, ret);                                         \
            /* TODO: Handle this error condition. */                                \
            while(1);                                                               \
        }                                                                           \
        else                                                                        \
        {                                                                           \
         /* am_util_stdio_printf("GPIO %d pinconfig success.", pin); */             \
        }                                                                           \
     /* am_util_stdio_printf("  CFG reg=0x%08X\n", *((uint32_t*)0x40010000 + pin)); */


#define CHKGPIOINTCTRL(chl,pin,ret)                                                 \
        {                                                                           \
        if ( ret )                                                                  \
        {                                                                           \
            am_util_stdio_printf("GPIO %d interrupt control failed.",               \
                                 pin, ret);                                         \
            /* TODO: Handle this error condition. */                                \
            while(1);                                                               \
        }                                                                           \
        else                                                                        \
        {                                                                           \
         /* am_util_stdio_printf("GPIO %d interrupt control success.", pin); */     \
        }                                                                           \
     /* uint32_t ugrp, *puaddr;                                                     \
        ugrp = pin / 32;                                                            \
        puaddr = (uint32_t*)0x400102C0 + ((chl & 1) * 16)+ (ugrp * 4);              \
        am_util_stdio_printf("  INTEN reg 0x%08X=0x%08X.\n",(uint32_t)puaddr,*puaddr); */ \
        }

//*****************************************************************************
//
// GPIO interrupt handlers
//
//*****************************************************************************
void
am_gpio0_001f_isr(void)
{
    uint32_t ui32Stat = GPIO->MCUN0INT0STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO0_001F_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO0_001F_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO0_001F_IRQn, ui32IntStatus);

    GPIO->MCUN0INT0CLR = ui32Stat;
    g_ui32ch0_001f = 0x00000001;    // Show that the ISR was entered
}

void
am_gpio0_203f_isr(void)
{
    uint32_t ui32Stat = GPIO->MCUN0INT1STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO0_203F_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO0_203F_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
        // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO0_203F_IRQn, ui32IntStatus);

    GPIO->MCUN0INT1CLR = ui32Stat;
    g_ui32ch0_203f = 0x00000001;    // Show that the ISR was entered
}

void
am_gpio0_405f_isr(void)
{
    uint32_t ui32Stat = GPIO->MCUN0INT2STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO0_405F_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO0_405F_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO0_405F_IRQn, ui32IntStatus);

    GPIO->MCUN0INT2CLR = ui32Stat;
    g_ui32ch0_405f = 0x00000001;    // Show that the ISR was entered
}

void
am_gpio0_607f_isr(void)
{
    uint32_t ui32Stat = GPIO->MCUN0INT3STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO0_607F_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO0_607F_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO0_607F_IRQn, ui32IntStatus);

    GPIO->MCUN0INT3CLR = ui32Stat;
    g_ui32ch0_607f = 0x00000001;    // Show that the ISR was entered
}

void
am_gpio0_809f_isr(void)
{
#ifndef AM_PART_APOLLO510L
    uint32_t ui32Stat = GPIO->MCUN0INT4STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO0_809F_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO0_809F_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO0_809F_IRQn, ui32IntStatus);

    GPIO->MCUN0INT4CLR = ui32Stat;
    g_ui32ch0_809f = 0x00000001;    // Show that the ISR was entered
#endif
}

void
am_gpio0_a0bf_isr(void)
{
#ifndef AM_PART_APOLLO510L
    uint32_t ui32Stat = GPIO->MCUN0INT5STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO0_A0BF_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO0_A0BF_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO0_A0BF_IRQn, ui32IntStatus);

    GPIO->MCUN0INT5CLR = ui32Stat;
    g_ui32ch0_a0bf = 0x00000001;    // Show that the ISR was entered
#endif
}

void
am_gpio0_c0df_isr(void)
{
#ifndef AM_PART_APOLLO510L
    uint32_t ui32Stat = GPIO->MCUN0INT6STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO0_C0DF_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO0_C0DF_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO0_C0DF_IRQn, ui32IntStatus);

    GPIO->MCUN0INT6CLR = ui32Stat;
    g_ui32ch0_c0df = 0x00000001;    // Show that the ISR was entered
#endif
}

void
am_gpio1_001f_isr(void)
{
    uint32_t ui32Stat = GPIO->MCUN1INT0STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO1_001F_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO1_001F_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO1_001F_IRQn, ui32IntStatus);

    GPIO->MCUN1INT0CLR = ui32Stat;
    g_ui32ch1_001f = 0x00000001;    // Show that the ISR was entered
}

void
am_gpio1_203f_isr(void)
{
    uint32_t ui32Stat = GPIO->MCUN1INT1STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO1_203F_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO1_203F_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO1_203F_IRQn, ui32IntStatus);

    GPIO->MCUN1INT1CLR = ui32Stat;
    g_ui32ch1_203f = 0x00000001;    // Show that the ISR was entered
}

void
am_gpio1_405f_isr(void)
{
    uint32_t ui32Stat = GPIO->MCUN1INT2STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO1_405F_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO1_405F_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO1_405F_IRQn, ui32IntStatus);

    GPIO->MCUN1INT2CLR = ui32Stat;
    g_ui32ch1_405f = 0x00000001;    // Show that the ISR was entered
}

void
am_gpio1_607f_isr(void)
{
    uint32_t ui32Stat = GPIO->MCUN1INT3STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO1_607F_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO1_607F_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO1_607F_IRQn, ui32IntStatus);

    GPIO->MCUN1INT3CLR = ui32Stat;
    g_ui32ch1_607f = 0x00000001;    // Show that the ISR was entered
}

void
am_gpio1_809f_isr(void)
{
#ifndef AM_PART_APOLLO510L
    uint32_t ui32Stat = GPIO->MCUN1INT4STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO1_809F_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO1_809F_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO1_809F_IRQn, ui32IntStatus);

    GPIO->MCUN1INT4CLR = ui32Stat;
    g_ui32ch1_809f = 0x00000001;    // Show that the ISR was entered
#endif
}

void
am_gpio1_a0bf_isr(void)
{
#ifndef AM_PART_APOLLO510L
    uint32_t ui32Stat = GPIO->MCUN1INT4STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO1_A0BF_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO1_A0BF_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO1_A0BF_IRQn, ui32IntStatus);

    GPIO->MCUN1INT5CLR = ui32Stat;
    g_ui32ch1_a0bf = 0x00000001;    // Show that the ISR was entered
#endif
}

void
am_gpio1_c0df_isr(void)
{
#ifndef AM_PART_APOLLO510L
    uint32_t ui32Stat = GPIO->MCUN1INT6STAT;
    uint32_t ui32RetVal,ui32IntStatus;

    // Critical section to prevent the intrrupt status from being overwritten.
    AM_CRITICAL_BEGIN
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_status_get((uint32_t)GPIO1_C0DF_IRQn, true, &ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQStatusGet = true;
    }
    // Read and clear the GPIO interrupt status.
    ui32RetVal = am_hal_gpio_interrupt_irq_clear((uint32_t)GPIO1_C0DF_IRQn, ui32IntStatus);
    if(ui32RetVal == AM_HAL_STATUS_SUCCESS)
    {
        bIRQClear = true;
    }
    AM_CRITICAL_END
    // Call the ISR.
    ui32RetVal = am_hal_gpio_interrupt_service((uint32_t)GPIO1_C0DF_IRQn, ui32IntStatus);

    GPIO->MCUN1INT6CLR = ui32Stat;
    g_ui32ch1_c0df = 0x00000001;    // Show that the ISR was entered
#endif
}

// ISR callback function
void isr_handler(void)
{
    bIntServiced = true;
}


static uint32_t
isrflag_get(am_hal_gpio_int_channel_e echl, uint32_t pin)
{
    uint32_t retval = 0;

    if ( pin >= 224 )
    {
        return retval;
    }

    if ( echl == AM_HAL_GPIO_INT_CHANNEL_0 )
    {
        if ( pin < 32 )
        {
            retval = g_ui32ch0_001f;
        }
        else if ( pin < 64 )
        {
            retval = g_ui32ch0_203f;
        }
        else if ( pin < 96 )
        {
            retval = g_ui32ch0_405f;
        }
        else if ( pin < 128 )
        {
            retval = g_ui32ch0_607f;
        }
        else if ( pin < 160)
        {
            retval = g_ui32ch0_809f;
        }
        else if ( pin < 192)
        {
            retval = g_ui32ch0_a0bf;
        }
        else if ( pin < 224)
        {
            retval = g_ui32ch0_c0df;
        }
    }
    else if ( echl == AM_HAL_GPIO_INT_CHANNEL_1 )
    {
        if ( pin < 32 )
        {
            retval = g_ui32ch1_001f;
        }
        else if ( pin < 64 )
        {
            retval = g_ui32ch1_203f;
        }
        else if ( pin < 96 )
        {
            retval = g_ui32ch1_405f;
        }
        else if ( pin < 128 )
        {
            retval = g_ui32ch1_607f;
        }
        else if ( pin < 160)
        {
            retval = g_ui32ch1_809f;
        }
        else if ( pin < 192)
        {
            retval = g_ui32ch1_a0bf;
        }
        else if ( pin < 224)
        {
            retval = g_ui32ch1_c0df;
        }
    }
    else
    {
        retval = 0;
    }

    return retval;

} // isrflag_get()


static void
isrflags_initall(uint32_t setval)
{
    g_ui32ch0_001f = g_ui32ch0_203f = g_ui32ch0_405f = g_ui32ch0_607f = g_ui32ch0_809f = g_ui32ch0_a0bf = g_ui32ch0_c0df = setval;
    g_ui32ch1_001f = g_ui32ch1_203f = g_ui32ch1_405f = g_ui32ch1_607f = g_ui32ch1_809f = g_ui32ch1_a0bf = g_ui32ch1_c0df = setval;
} // isrflags_initall()

static bool
isrflags_chkall(am_hal_gpio_int_channel_e echl_excl, uint32_t pin_excl)
{
    uint32_t uexclbit;
    uint32_t ui32Mask = 0;

    //
    // All except one of the flags are expected to be zero.
    // Return false if no other flags are set.
    // Return true if another flag is set.
    //

    if ( g_ui32ch0_001f != 0 )
    {
        ui32Mask |= 0x0001;
    }
    if ( g_ui32ch0_203f != 0 )
    {
        ui32Mask |= 0x0002;
    }
    if ( g_ui32ch0_405f != 0 )
    {
        ui32Mask |= 0x0004;
    }
    if ( g_ui32ch0_607f != 0 )
    {
        ui32Mask |= 0x0008;
    }
    if ( g_ui32ch0_809f != 0 )
    {
        ui32Mask |= 0x0010;
    }
    if ( g_ui32ch0_a0bf != 0 )
    {
        ui32Mask |= 0x0020;
    }
    if ( g_ui32ch0_c0df != 0 )
    {
        ui32Mask |= 0x0040;
    }

    if ( g_ui32ch1_001f != 0 )
    {
        ui32Mask |= 0x0100;
    }
    if ( g_ui32ch1_203f != 0 )
    {
        ui32Mask |= 0x0200;
    }
    if ( g_ui32ch1_405f != 0 )
    {
        ui32Mask |= 0x0400;
    }
    if ( g_ui32ch1_607f != 0 )
    {
        ui32Mask |= 0x0800;
    }
    if ( g_ui32ch1_809f != 0 )
    {
        ui32Mask |= 0x1000;
    }
    if ( g_ui32ch1_a0bf != 0 )
    {
        ui32Mask |= 0x2000;
    }
    if ( g_ui32ch1_c0df != 0 )
    {
        ui32Mask |= 0x4000;
    }

    uexclbit  = 1 << (pin_excl / 32);
    uexclbit <<= ((echl_excl == AM_HAL_GPIO_INT_CHANNEL_1) ? 8 : 0);

    ui32Mask &= ( ~uexclbit );

    return ui32Mask == 0 ? false : true;
} // isrflags_chkall()


static IRQn_Type
NVICIRQ_get(am_hal_gpio_int_channel_e eChl, uint32_t ui32pin)
{
    uint32_t ui32irq;

    ui32irq = ui32pin / 32;
    ui32irq += (eChl == AM_HAL_GPIO_INT_CHANNEL_0) ? 0 : 7;

    switch ( ui32irq )
    {
        case 0:
            return GPIO0_001F_IRQn;
        case 1:
            return GPIO0_203F_IRQn;
        case 2:
            return GPIO0_405F_IRQn;
        case 3:
            return GPIO0_607F_IRQn;
        case 4:
            return GPIO0_809F_IRQn;
        case 5:
            return GPIO0_A0BF_IRQn;
        case 6:
            return GPIO0_C0DF_IRQn;
        case 7:
            return GPIO1_001F_IRQn;
        case 8:
            return GPIO1_203F_IRQn;
        case 9:
            return GPIO1_405F_IRQn;
        case 10:
            return GPIO1_607F_IRQn;
        case 11:
            return GPIO1_809F_IRQn;
        case 12:
            return GPIO1_A0BF_IRQn;
        case 13:
            return GPIO1_C0DF_IRQn;
        default:
            return (IRQn_Type)0;
    }
} // NVICIRQ_get()

static void
gpio_clear_allints(void)
{
     GPIO->MCUN0INT0CLR = 0xFFFFFFFF;
     GPIO->MCUN0INT1CLR = 0xFFFFFFFF;
     GPIO->MCUN0INT2CLR = 0xFFFFFFFF;
     GPIO->MCUN0INT3CLR = 0xFFFFFFFF;
#ifndef AM_PART_APOLLO510L
     GPIO->MCUN0INT4CLR = 0xFFFFFFFF;
     GPIO->MCUN0INT5CLR = 0xFFFFFFFF;
     GPIO->MCUN0INT6CLR = 0xFFFFFFFF;
#endif
     GPIO->MCUN1INT0CLR = 0xFFFFFFFF;
     GPIO->MCUN1INT1CLR = 0xFFFFFFFF;
     GPIO->MCUN1INT2CLR = 0xFFFFFFFF;
     GPIO->MCUN1INT3CLR = 0xFFFFFFFF;
#ifndef AM_PART_APOLLO510L
     GPIO->MCUN1INT4CLR = 0xFFFFFFFF;
     GPIO->MCUN1INT5CLR = 0xFFFFFFFF;
     GPIO->MCUN1INT6CLR = 0xFFFFFFFF;
#endif
} // gpio_clear_allints()

static void
gpio_disable_allints(void)
{
     GPIO->MCUN0INT0EN = 0;
     GPIO->MCUN0INT1EN = 0;
     GPIO->MCUN0INT2EN = 0;
     GPIO->MCUN0INT3EN = 0;
#ifndef AM_PART_APOLLO510L
     GPIO->MCUN0INT4EN = 0;
     GPIO->MCUN0INT5EN = 0;
     GPIO->MCUN0INT6EN = 0;
#endif
     GPIO->MCUN1INT0EN = 0;
     GPIO->MCUN1INT1EN = 0;
     GPIO->MCUN1INT2EN = 0;
     GPIO->MCUN1INT3EN = 0;
#ifndef AM_PART_APOLLO510L
     GPIO->MCUN1INT4EN = 0;
     GPIO->MCUN1INT5EN = 0;
     GPIO->MCUN1INT6EN = 0;
#endif
} // gpio_disable_allints()

#if 0
static uint32_t
isrflag_set(am_hal_gpio_int_channel_e echl, uint32_t pin, uint32_t setval)
{
    uint32_t retval = 0;

    if ( pin >= 224 )
    {
        return retval;
    }

    if ( echl == AM_HAL_GPIO_INT_CHANNEL_0 )
    {
        if ( pin < 32 )
        {
            g_ui32ch0_001f = setval;
        }
        else if ( pin < 64 )
        {
            g_ui32ch0_203f = setval;
        }
        else if ( pin < 96 )
        {
            g_ui32ch0_405f = setval;
        }
        else if ( pin < 128 )
        {
            g_ui32ch0_607f = setval;
        }
        else if ( pin < 160 )
        {
            g_ui32ch0_809f = setval;
        }
        else if ( pin < 192 )
        {
            g_ui32ch0_a0bf = setval;
        }
        else if ( pin < 224 )
        {
            g_ui32ch0_c0df = setval;
        }
        retval = setval;
    }
    else if ( echl == AM_HAL_GPIO_INT_CHANNEL_1 )
    {
        if ( pin < 32 )
        {
            g_ui32ch1_001f = setval;
        }
        else if ( pin < 64 )
        {
            g_ui32ch1_203f = setval;
        }
        else if ( pin < 96 )
        {
            g_ui32ch1_405f = setval;
        }
        else if ( pin < 128 )
        {
            g_ui32ch1_607f = setval;
        }
        else if ( pin < 160 )
        {
            g_ui32ch1_809f = setval;
        }
        else if ( pin < 192 )
        {
            g_ui32ch1_a0bf = setval;
        }
        else if ( pin < 224 )
        {
            g_ui32ch1_c0df = setval;
        }
        retval = setval;
    }
    else
    {
        retval = 0;
    }

    return retval;
} // isrflag_set()
#endif

static void
gpio_int_check_fail(uint32_t RetCode, uint32_t DesiredCode)
{
    if(RetCode != DesiredCode)
    {
        am_util_stdio_printf("-- FAIL!!!\n");
        if(RetCode & TINT_LO2HI)
        {
            am_util_stdio_printf("   rising edge triggered\n");
        }
        else
        {
            am_util_stdio_printf("   rising edge not triggered\n");
        }

        if(RetCode & TINT_LO2HI_OTHER_INT)
        {
            am_util_stdio_printf("   other interrupt triggered at rising edge\n");
        }

        if(RetCode & TINT_HI2LO)
        {
            am_util_stdio_printf("   falling edge triggered\n");
        }
        else
        {
            am_util_stdio_printf("   falling edge not triggered\n");
        }

        if(RetCode & TINT_HI2LO_OTHER_INT)
        {
            am_util_stdio_printf("   other interrupt triggered at falling edge\n");
        }
    }
    else
    {
        am_util_stdio_printf("-- PASS!!!\n");
    }
}
//*****************************************************************************
//
// GPIO brief interrupt test.
//
// Arguments:
//  ui32PinTestIntrpt   = Pin (input) to be interrupted.
//  ui32PinDrive        = Pin used to drive the interrupt.
//  eIntType:           = One of AM_HAL_GPIO_PIN_INTDIR_NONE/HI2LO/LO2HI/BOTH
//  eChl:               = One of AM_HAL_GPIO_INT_CHANNEL_0/1/BOTH
//  ui8LoopCount        = Number of times interrupt is to be iterated.
//
// The 2 pins are expected to be physically connected together and
// the pin configured by the caller.  Interrupts should not be enabled
// on entry.
//
//*****************************************************************************
uint32_t
am_widget_gpio_int_check(uint32_t ui32PinTestIntrpt, uint32_t ui32PinDrive,
                         am_hal_gpio_intdir_e eIntType, am_hal_gpio_int_channel_e eChl,
						 uint8_t ui8LoopCount)
{
    am_hal_gpio_mask_t GpioIntMask = AM_HAL_GPIO_MASK_DECLARE_ZERO;
    uint32_t ui32FuncRet, ui32RetVal;
    uint32_t ui32Code;
    IRQn_Type eIRQnum;

    //
    // Disable all GPIO interrupts
    //
    gpio_disable_allints();

    //
    // Initialize the return value
    //
    ui32RetVal = 0;

    //
    // Make sure the IRQ is disabled.
    //
    eIRQnum = NVICIRQ_get(eChl, ui32PinTestIntrpt);
    NVIC_DisableIRQ(eIRQnum);

    //
    // Configure the pin being tested.
    //
    am_hal_gpio_output_clear(ui32PinDrive);
    g_GPIO_INPUT_INT.GP.cfg_b.eIntDir = eIntType;
    ui32FuncRet = am_hal_gpio_pinconfig(ui32PinTestIntrpt, g_GPIO_INPUT_INT);
    TEST_ASSERT_TRUE(ui32FuncRet == AM_HAL_STATUS_SUCCESS);
    if(ui32FuncRet != AM_HAL_STATUS_SUCCESS)
    {
        ui32RetVal |= 0xFFFF;
    }
    //CHKPINCFG(ui32PinTestIntrpt, ui32FuncRet);

    //
    // Init and configure the driving pin.
    //
    am_hal_gpio_output_clear(ui32PinDrive);
    ui32FuncRet = am_hal_gpio_pinconfig(ui32PinDrive, g_GPIO_OUTPUT_READ);
    TEST_ASSERT_TRUE(ui32FuncRet == AM_HAL_STATUS_SUCCESS);
    if(ui32FuncRet != AM_HAL_STATUS_SUCCESS)
    {
        ui32RetVal |= 0xFFFF;
    }
    //CHKPINCFG(ui32PinDrive, ui32FuncRet);

    //Get the status of all the interrupts
    ui32FuncRet = am_hal_gpio_interrupt_status_get(eChl, false, &GpioIntMask);
    TEST_ASSERT_TRUE(ui32FuncRet == AM_HAL_STATUS_SUCCESS);
    if(ui32FuncRet != AM_HAL_STATUS_SUCCESS)
    {
        ui32RetVal |= 0x1000;
    }

    //
    // After pin configuration, clear all interrupts before enabling.
    //
    //gpio_clear_allints();
    ui32FuncRet = am_hal_gpio_interrupt_clear(eChl, &GpioIntMask);
    TEST_ASSERT_TRUE(ui32FuncRet == AM_HAL_STATUS_SUCCESS);
    if(ui32FuncRet != AM_HAL_STATUS_SUCCESS)
    {
        ui32RetVal |= 0x2000;
    }

    // Register the interrupt
    ui32FuncRet = am_hal_gpio_interrupt_register(eChl, ui32PinTestIntrpt,
                                         (am_hal_gpio_handler_t)isr_handler, NULL);
    TEST_ASSERT_TRUE(ui32FuncRet == AM_HAL_STATUS_SUCCESS);
    if(ui32FuncRet != AM_HAL_STATUS_SUCCESS)
    {
        ui32RetVal |= 0x4000;
    }

    //
    // Enable the interrupt
    //
    ui32FuncRet = am_hal_gpio_interrupt_control(eChl,
                                                AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                                &ui32PinTestIntrpt);
    TEST_ASSERT_TRUE(ui32FuncRet == AM_HAL_STATUS_SUCCESS);
    //CHKGPIOINTCTRL(eChl, ui32PinTestIntrpt, ui32FuncRet);
    if(ui32FuncRet != AM_HAL_STATUS_SUCCESS)
    {
        ui32RetVal |= 0x8000;
    }

    am_hal_interrupt_master_enable();

    //
    // Perform the given interrupt test.
    //
    isrflags_initall(0);

    NVIC_EnableIRQ(eIRQnum);
    // Trigger interrupt for given count.
    for(uint32_t ui32Loop = 0; ui32Loop < ui8LoopCount; ++ui32Loop)
    {
        ui32Code = 0;
        //
        // Take the driving signal high.
        //
        am_hal_gpio_output_set(ui32PinDrive);
        am_hal_delay_us(100);

        //
        // Verify that if ISR was called.
        //
        if ( isrflag_get(eChl, ui32PinTestIntrpt) )
        {
            ui32Code |= TINT_LO2HI;
        }

        //
        // Verify that no other ISRs were called.
        //
        if ( isrflags_chkall(eChl, ui32PinTestIntrpt) )
        {
            ui32Code |= TINT_LO2HI_OTHER_INT;
        }

        //
        // Clear all interrupt flags.
        //
        isrflags_initall(0);

        //
        // Take the driving signal low
        //
        am_hal_gpio_output_clear(ui32PinDrive);
        am_hal_delay_us(100);

        //
        // Verify that if ISR was called.
        //
        if ( isrflag_get(eChl, ui32PinTestIntrpt) )
        {
            ui32Code |= TINT_HI2LO;
        }

        //
        // Verify that no other ISRs were called.
        //
        if ( isrflags_chkall(eChl, ui32PinTestIntrpt) )
        {
            ui32Code |= TINT_HI2LO_OTHER_INT;
        }

        //
        // Clear all interrupt flags.
        //
        isrflags_initall(0);

        switch(eIntType)
        {
            case AM_HAL_GPIO_PIN_INTDIR_LO2HI:
                TEST_ASSERT_TRUE(ui32Code == TINT_LO2HI);
                gpio_int_check_fail(ui32Code, TINT_LO2HI);
                break;
            case AM_HAL_GPIO_PIN_INTDIR_HI2LO:
                TEST_ASSERT_TRUE(ui32Code == TINT_HI2LO);
                gpio_int_check_fail(ui32Code, TINT_HI2LO);
                break;
            case AM_HAL_GPIO_PIN_INTDIR_BOTH:
                TEST_ASSERT_TRUE(ui32Code == (TINT_LO2HI | TINT_HI2LO));
                gpio_int_check_fail(ui32Code, TINT_LO2HI | TINT_HI2LO);
                break;
            case AM_HAL_GPIO_PIN_INTDIR_NONE:
                TEST_ASSERT_TRUE(ui32Code == 0);
                gpio_int_check_fail(ui32Code, 0);
                break;
        }

    }

    //
    // Disable IRQ and master interrupts
    // ensure a clean slate for the next text.
    //
    gpio_disable_allints();
    gpio_clear_allints();
    NVIC_DisableIRQ(eIRQnum);
    am_hal_interrupt_master_disable();
    NVIC_ClearPendingIRQ(eIRQnum);
    //
    // Set pins back to default config
    //
    am_hal_gpio_output_clear(ui32PinDrive);
    ui32FuncRet = am_hal_gpio_pinconfig(ui32PinDrive,      am_hal_gpio_pincfg_default);
    ui32FuncRet = am_hal_gpio_pinconfig(ui32PinTestIntrpt, am_hal_gpio_pincfg_default);


    return ui32RetVal;
}
