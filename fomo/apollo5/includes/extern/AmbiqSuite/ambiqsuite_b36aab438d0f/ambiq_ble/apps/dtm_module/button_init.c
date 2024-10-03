//*****************************************************************************
//
//! @file button_init.c
//!
//! @brief Initialize the button of board to switch to DTM mode from general BLE mode.
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

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_devices_cooper.h"
#include "wsf_types.h"
#include "FreeRTOSConfig.h"
#include "dtm_api.h"


//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define CTRL_BUTTON0          AM_BSP_GPIO_BUTTON0
#define CTRL_BUTTON1          AM_BSP_GPIO_BUTTON1


//*****************************************************************************
//
// Function Definitions
//
//*****************************************************************************

/*****************************************************************************/
/*!
 *  \fn     am_gpio0_001f_isr
 *
 *  \brief  ISR function of am_gpio0_001f(pins: 00-31, channel: 0)
 *
 *  \param  None.
 *
 *  \return None.
 */
/*****************************************************************************/
void am_gpio0_001f_isr(void)
{
    //
    // Read and clear the GPIO interrupt status.
    //
    uint32_t ui32IntStatus;
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, true, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_service(GPIO0_001F_IRQn, ui32IntStatus);
}

/*****************************************************************************/
/*!
 *  \fn     button0_handler
 *
 *  \brief  Handler for the button0 press action.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*****************************************************************************/
void button0_handler(void)
{
    ui_switch_to_dtm();
}

/*****************************************************************************/
/*!
 *  \fn     button1_handler
 *
 *  \brief  Handler for the button1 press action.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*****************************************************************************/
void button1_handler(void)
{
    ui_exit_from_dtm();
}

/*****************************************************************************/
/*!
 *  \fn     dtm_enter_button_init
 *
 *  \brief  Initialize the button gpio for switching to DTM mode.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*****************************************************************************/
void dtm_enter_button_init(void)
{
    am_hal_gpio_pincfg_t sPinCfg = {0};

    // Config the button 0
    sPinCfg.GP.cfg_b.uFuncSel  = 3;  // GPIO
    sPinCfg.GP.cfg_b.eGPOutCfg = AM_HAL_GPIO_PIN_OUTCFG_DISABLE;
    sPinCfg.GP.cfg_b.eGPInput  = AM_HAL_GPIO_PIN_INPUT_ENABLE;
    sPinCfg.GP.cfg_b.eGPRdZero = AM_HAL_GPIO_PIN_RDZERO_READPIN;
    sPinCfg.GP.cfg_b.eIntDir   = AM_HAL_GPIO_PIN_INTDIR_HI2LO;
    sPinCfg.GP.cfg_b.ePullup   = AM_HAL_GPIO_PIN_PULLUP_100K;

    am_hal_gpio_pinconfig(CTRL_BUTTON0, sPinCfg);
    am_hal_gpio_pinconfig(CTRL_BUTTON1, sPinCfg);

    // Register handler for button 0 & 1
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, CTRL_BUTTON0, (am_hal_gpio_handler_t)button0_handler, NULL);
    am_hal_gpio_interrupt_register(AM_HAL_GPIO_INT_CHANNEL_0, CTRL_BUTTON1, (am_hal_gpio_handler_t)button1_handler, NULL);

    // Clear the GPIO Interrupt (write to clear).
    uint32_t ui32IntStatus = 0;
    ui32IntStatus |= (1 << CTRL_BUTTON0) | (1 << CTRL_BUTTON1);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);

    // Enable the GPIO/button interrupt.
    uint32_t ui32GpioNum = CTRL_BUTTON0;
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0, AM_HAL_GPIO_INT_CTRL_INDV_ENABLE, &ui32GpioNum);
    ui32GpioNum = CTRL_BUTTON1;
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0, AM_HAL_GPIO_INT_CTRL_INDV_ENABLE, &ui32GpioNum);

    NVIC_SetPriority(GPIO0_001F_IRQn, NVIC_configKERNEL_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);
}

