//*****************************************************************************
//
//! @file am_devices_ambt53_gpio.c
//!
//! @brief The implementation of Apollo interface to AMBT53 GPIO embeded in SCPM.
//!
//! @addtogroup ambt53 AMBT53 Device Driver
//! @ingroup devices
//! @{
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
#include "am_bsp.h"
#include "am_mcu_apollo.h"
#include "am_util_stdio.h"
#include "am_devices_ambt53.h"

//*****************************************************************************
//
//! @name Macro definitions.
//! @{
//
//*****************************************************************************

//*****************************************************************************
//! Array of function pointers for handling GPIO interrupts.
//*****************************************************************************
static am_hal_gpio_handler_t gpio_ppfnHandlers[AMBT53_GPIO_NUMWORDS][32];
static void *gpio_pppvIrqArgs[AMBT53_GPIO_NUMWORDS][32];
/////////////////////////////////////////////////////////////////////////////////
//
// SCPM Extended GPIO driver
//
/////////////////////////////////////////////////////////////////////////////////

//! Default settings for GPIOs.
const am_devices_ambt53_gpio_pincfg_t am_devices_ambt53_gpio_pincfg_default           = AMBT53_GPIO_PINCFG_DEFAULT;
//! Default settings for GPIOs.
const am_devices_ambt53_gpio_pincfg_t am_devices_ambt53_gpio_pincfg_output            = AMBT53_GPIO_PINCFG_OUTPUT;
//! Default settings for GPIOs.
const am_devices_ambt53_gpio_pincfg_t am_devices_ambt53_gpio_pincfg_input             = AMBT53_GPIO_PINCFG_INPUT;

//*****************************************************************************
//! @brief
//! @param gpio_index
//! @param reg_address
//! @param reg_val
//*****************************************************************************
static void
am_devices_ambt53_gpio_reg_config(uint32_t gpio_index, uint32_t reg_address, uint32_t reg_val)
{
    uint32_t raw_cfg = 0;
    SCPM_REG32_RD(reg_address, &raw_cfg);
    raw_cfg = SET_ATTRIBUTE(raw_cfg, gpio_index, 1, (reg_val & 0x01));
    SCPM_REG32_WR(reg_address, raw_cfg);
}

//*****************************************************************************
//
//  Return the current configuration of a pin.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_gpio_pinconfig_get(uint32_t ui32GpioNum, am_devices_ambt53_gpio_pincfg_t* psGpioCfg)
{
    uint8_t group_num = ui32GpioNum / 32;
    uint8_t gpio_index = ui32GpioNum % 32;
    uint32_t ui32ReadState = 0;

    if ( psGpioCfg == NULL )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }
    SCPM_REG32_RD(AMBT53_GPIO_DIR_REG(group_num), &ui32ReadState);
    psGpioCfg->eGPDir = GET_ATTRIBUTE(ui32ReadState, gpio_index, 1);
    SCPM_REG32_RD(AMBT53_GPIO_IS_REG(group_num), &ui32ReadState);
    psGpioCfg->eIntTri = GET_ATTRIBUTE(ui32ReadState, gpio_index, 1);
    SCPM_REG32_RD(AMBT53_GPIO_IBE_REG(group_num), &ui32ReadState);
    psGpioCfg->eIntDir = GET_ATTRIBUTE(ui32ReadState, gpio_index, 1);
    SCPM_REG32_RD(AMBT53_GPIO_IEV_REG(group_num), &ui32ReadState);
    psGpioCfg->eIntPol = GET_ATTRIBUTE(ui32ReadState, gpio_index, 1);
    SCPM_REG32_RD(AMBT53_GPIO_DB_REG(group_num), &ui32ReadState);
    psGpioCfg->bDBEn = GET_ATTRIBUTE(ui32ReadState, gpio_index, 1);
    SCPM_REG32_RD(AMBT53_GPIO_DFG_REG(group_num), &psGpioCfg->uDBVal);

    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Configure the function of a single pin.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_gpio_pinconfig(uint32_t ui32GpioNum, const am_devices_ambt53_gpio_pincfg_t sGpioCfg)
{
    uint8_t group_num = 0;
    uint8_t gpio_index = 0;
    // Parameter check
    if ( (ui32GpioNum >= AMBT53_GPIO_TOTAL_GPIOS) || (sGpioCfg.eIntTri > AMBT53_GPIO_INTR_LEVEL) || (sGpioCfg.eGPDir > AMBT53_GPIO_DIR_OUTPUT) ||
         (sGpioCfg.eIntDir > AMBT53_GPIO_INTR_BOTH_EDGE) || (sGpioCfg.eIntPol > AMBT53_GPIO_INTR_POS_EDGE) || (sGpioCfg.ePullup > AMBT53_GPIO_PULL_DOWN) ||
         ((sGpioCfg.bDBEn == 1) && (sGpioCfg.uDBVal == 0xFFFFFFFF)) )   // no x_clk connected to ambt53, not allowed to be 32'hFFFF_FFFF
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }
    group_num = ui32GpioNum / 32;
    gpio_index = ui32GpioNum % 32;

    am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_CTRL_REG(group_num), AMBT53_GPIO_SOFT_MODE); // Fixed to soft mode
    am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_DIR_REG(group_num), sGpioCfg.eGPDir);
    am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_IS_REG(group_num), sGpioCfg.eIntTri);
    am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_IBE_REG(group_num), sGpioCfg.eIntDir);
    am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_IEV_REG(group_num), sGpioCfg.eIntPol);
    am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_DB_REG(group_num), sGpioCfg.bDBEn);
    SCPM_REG32_WR(AMBT53_GPIO_DFG_REG(group_num), sGpioCfg.uDBVal);

    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Read GPIO state values
//
//*****************************************************************************
uint32_t
am_devices_ambt53_gpio_state_read(uint32_t ui32GpioNum,
                                  am_hal_gpio_read_type_e eReadType,
                                  uint32_t *pui32ReadState)
{
    uint8_t group_num = ui32GpioNum / 32;
    uint8_t gpio_index = ui32GpioNum % 32;

    //
    // Find the correct register to read based on the read type input. Each of
    // these registers map exactly one bit to each pin, so the calculation for
    // which register to use is simple.
    //
    switch (eReadType)
    {
        case AM_HAL_GPIO_INPUT_READ:
            SCPM_REG32_RD(AMBT53_GPIO_IDATA_REG(group_num), pui32ReadState);
            break;

        case AM_HAL_GPIO_OUTPUT_READ:
            SCPM_REG32_RD(AMBT53_GPIO_ODATA_REG(group_num), pui32ReadState);
            break;

        default:
            return AM_DEVICES_SCPM_STATUS_INVALID_OPERATION;
    }

    //
    // We need to do one more shift and mask to get to the specific bit we want
    // for the chosen pin. Return the value to the caller through the read
    // state variable.
    //
    *pui32ReadState = GET_ATTRIBUTE(*pui32ReadState, gpio_index, 1);

    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Write GPIO state values
//
//*****************************************************************************
uint32_t
am_devices_ambt53_gpio_state_write(uint32_t ui32GpioNum,
                                   am_hal_gpio_write_type_e eWriteType)
{
    uint8_t group_num = ui32GpioNum / 32;
    uint8_t gpio_index = ui32GpioNum % 32;

    switch (eWriteType)
    {
        case AM_HAL_GPIO_OUTPUT_CLEAR:
            am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_CLR_REG(group_num), 1);
            break;
        case AM_HAL_GPIO_OUTPUT_SET:
            am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_SET_REG(group_num), 1);
            break;
        default:
            return AM_DEVICES_SCPM_STATUS_INVALID_OPERATION;
    }

    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  GPIO Interrupt control.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_gpio_interrupt_control(am_hal_gpio_int_ctrl_e eControl,
                                         void *pGpioIntMaskOrNumber)
{
    uint32_t ui32Gpionum, group_num, gpio_index;
    am_devices_ambt53_gpio_mask_t *pGpioIntMask = (am_devices_ambt53_gpio_mask_t*)pGpioIntMaskOrNumber;

    //
    // In all cases, the pointer must be non-NULL.
    //
    if ( pGpioIntMaskOrNumber == NULL )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }

    if ( eControl > AM_HAL_GPIO_INT_CTRL_LAST )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }

    if ( eControl <= AM_HAL_GPIO_INT_CTRL_INDV_ENABLE )
    {
        ui32Gpionum = *(uint32_t *)pGpioIntMaskOrNumber;
        if ( ui32Gpionum >= AMBT53_GPIO_TOTAL_GPIOS )
        {
            return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
        }
        group_num = ui32Gpionum / 32;
        gpio_index = ui32Gpionum % 32;
    }

    switch ( eControl )
    {
        case AM_HAL_GPIO_INT_CTRL_INDV_DISABLE:
            am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_IEN_REG(group_num), 0);
            am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_IM_REG(group_num), 1);
            break;

        case AM_HAL_GPIO_INT_CTRL_INDV_ENABLE:
            am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_IEN_REG(group_num), 1);
            am_devices_ambt53_gpio_reg_config(gpio_index, AMBT53_GPIO_IM_REG(group_num), 0);
            break;

        case AM_HAL_GPIO_INT_CTRL_MASK_DISABLE:
            {
                uint32_t raw_cfg = 0;
                SCPM_REG32_RD(AMBT53_GPIO_IEN_REG(AMBT53_GPIO0_IRQn), &raw_cfg);
                raw_cfg &= ~pGpioIntMask->U.Msk[AMBT53_GPIO0_IRQn];
                SCPM_REG32_WR(AMBT53_GPIO_IEN_REG(AMBT53_GPIO0_IRQn), raw_cfg);
                SCPM_REG32_RD(AMBT53_GPIO_IEN_REG(AMBT53_GPIO1_IRQn), &raw_cfg);
                raw_cfg &= ~pGpioIntMask->U.Msk[AMBT53_GPIO1_IRQn];
                SCPM_REG32_WR(AMBT53_GPIO_IEN_REG(AMBT53_GPIO1_IRQn), raw_cfg);
            }
            break;

        case AM_HAL_GPIO_INT_CTRL_MASK_ENABLE:
            {
                uint32_t raw_cfg = 0;
                SCPM_REG32_RD(AMBT53_GPIO_IEN_REG(AMBT53_GPIO0_IRQn), &raw_cfg);
                raw_cfg |= pGpioIntMask->U.Msk[AMBT53_GPIO0_IRQn];
                SCPM_REG32_WR(AMBT53_GPIO_IEN_REG(AMBT53_GPIO0_IRQn), raw_cfg);
                SCPM_REG32_RD(AMBT53_GPIO_IEN_REG(AMBT53_GPIO1_IRQn), &raw_cfg);
                raw_cfg |= pGpioIntMask->U.Msk[AMBT53_GPIO1_IRQn];
                SCPM_REG32_WR(AMBT53_GPIO_IEN_REG(AMBT53_GPIO1_IRQn), raw_cfg);
            }
            break;

        default:
            break;
    }
    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Read the interrupt status of a given GPIO IRQ.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_gpio_interrupt_irq_status_get(uint32_t ui32GpioIrq,
                                                bool bEnabledOnly,
                                                uint32_t *pui32IntStatus)
{
    uint32_t ui32Mask = 0xFFFFFFFF;
    uint32_t ui32Status = 0;

    if ( pui32IntStatus == NULL )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }

    if ( bEnabledOnly )
    {
        SCPM_REG32_RD(AMBT53_GPIO_IEN_REG(ui32GpioIrq), &ui32Mask);
    }
    SCPM_REG32_RD(AMBT53_GPIO_RIS_REG(ui32GpioIrq), &ui32Status);
    *pui32IntStatus = ui32Status & ui32Mask;

    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Clear the interrupt(s) for the given GPIO IRQ.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_gpio_interrupt_irq_clear(uint32_t ui32GpioIrq,
                                           uint32_t ui32GpioIntMaskStatus)
{
    SCPM_REG32_WR(AMBT53_GPIO_IC_REG(ui32GpioIrq), ui32GpioIntMaskStatus);

    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}
//*****************************************************************************
//
//  Register an interrupt handler for a specific GPIO
//
//*****************************************************************************
uint32_t
am_devices_ambt53_gpio_interrupt_register(uint32_t ui32GpioNum,
                                          am_hal_gpio_handler_t pfnHandler,
                                          void *pArg)
{
    uint8_t group_num = ui32GpioNum / 32;
    uint8_t gpio_index = ui32GpioNum % 32;

    gpio_ppfnHandlers[group_num][gpio_index] = pfnHandler;
    gpio_pppvIrqArgs[group_num][gpio_index] = pArg;

    return AM_DEVICES_SCPM_STATUS_SUCCESS;
}

//*****************************************************************************
//
//  Relay interrupts from the main GPIO module to individual handlers.
//
//*****************************************************************************
uint32_t
am_devices_ambt53_gpio_interrupt_service(uint32_t ui32GpioIrq,
                                         uint32_t ui32GpioIntMaskStatus)
{
    uint32_t ui32RetStatus = AM_DEVICES_SCPM_STATUS_SUCCESS;
    uint32_t ui32FFS;
    am_hal_gpio_handler_t pfnHandler;
    void *pArg;

    if ( ui32GpioIrq > AMBT53_GPIO_NUMWORDS )
    {
        return AM_DEVICES_SCPM_STATUS_INVALID_ARG;
    }

    //
    // Handle interrupts.
    // Get status word from the caller.
    //
    while ( ui32GpioIntMaskStatus )
    {
        //
        // We need to FFS (Find First Set).  We can easily zero-base FFS
        // since we know that at least 1 bit is set in ui32GpioIntMaskStatus.
        // FFS(x) = 31 - clz(x & -x).       // Zero-based version of FFS.
        //
        ui32FFS = ui32GpioIntMaskStatus & (uint32_t)(-(int32_t)ui32GpioIntMaskStatus);
        ui32FFS = 31 - AM_ASM_CLZ(ui32FFS);

        //
        // Turn off the bit we picked in the working copy
        //
        ui32GpioIntMaskStatus &= ~(0x00000001 << ui32FFS);

        //
        // Check the bit handler table to see if there is an interrupt handler
        // registered for this particular bit.
        //
        pfnHandler = gpio_ppfnHandlers[ui32GpioIrq][ui32FFS];
        pArg = gpio_pppvIrqArgs[ui32GpioIrq][ui32FFS];
        if ( pfnHandler )
        {
            //
            // If we found an interrupt handler routine, call it now.
            //
            pfnHandler(pArg);
        }
        else
        {
            //
            // No handler was registered for the GPIO that interrupted.
            // Return an error.
            //
            ui32RetStatus = AM_DEVICES_SCPM_STATUS_INVALID_OPERATION;
        }
    }

    return ui32RetStatus;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

