//*****************************************************************************
//
//! @file am_devices_ambt53_mailbox.c
//!
//! @brief The implementation of Apollo interface to AMBT53's mailbox.
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

uint32_t am_devices_ambt53_mailbox_set_d2m_int_thr(uint32_t threshold)
{
    if ((threshold < MAILBOX_FIFO_MAX_DEEPTH) & (threshold > 0))
    {
        return AMBT53_REG32_WR(MAILBOX_D2M_IT_ADDR, threshold);
    }
    return AM_DEVICES_AMBT53_STATUS_FAIL;
}

uint32_t am_devices_ambt53_mailbox_enable_d2m_int(uint32_t interrupt)
{
    uint32_t d2m_ie = 0;
    uint32_t reg_rw_status = AM_DEVICES_AMBT53_STATUS_FAIL;

    // mailbox interrupt source only occupies lowest two bits
    reg_rw_status = AMBT53_REG32_RD(MAILBOX_D2M_IE_ADDR, &d2m_ie);
    if ((reg_rw_status == AM_DEVICES_AMBT53_STATUS_SUCCESS) && (d2m_ie != (interrupt & 0x03)))
    {
        d2m_ie |= interrupt & 0x03;
        return AMBT53_REG32_WR(MAILBOX_D2M_IE_ADDR, d2m_ie);
    }
    return reg_rw_status;
}

/*
 * GLOBAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint32_t am_devices_ambt53_mailbox_read_data(uint32_t *buffer, uint32_t *len)
{
    uint32_t i = 0;
    uint32_t ui32Status = 0;
    uint32_t reg_rw_status = AM_DEVICES_AMBT53_STATUS_FAIL;

    if (buffer != NULL)
    {
        for (i = 0; i < *len; i++)
        {
            reg_rw_status = AMBT53_REG32_RD(MAILBOX_STATUS_ADDR, &ui32Status);
            if ((reg_rw_status != AM_DEVICES_AMBT53_STATUS_SUCCESS) || ((ui32Status & MAILBOX_STATUS_D2M_EMPTY_Msk) == 1))
            {
                break;
            }
            reg_rw_status = AMBT53_REG32_RD(MAILBOX_D2M_DATA_ADDR, &(buffer[i]));
            if (reg_rw_status != AM_DEVICES_AMBT53_STATUS_SUCCESS)
            {
                break;
            }
        }
    }
    *len = i;
    return reg_rw_status;
}

uint32_t am_devices_ambt53_mailbox_flush_data(uint32_t *buffer, uint32_t *len)
{
    uint32_t i = 0;
    uint32_t ui32Status = 0;
    uint32_t d2m_data;
    uint32_t reg_rw_status = AM_DEVICES_AMBT53_STATUS_FAIL;

    for (i = 0; i < MAILBOX_FIFO_MAX_DEEPTH; i++)
    {
        reg_rw_status = AMBT53_REG32_RD(MAILBOX_STATUS_ADDR, &ui32Status);
        if ((reg_rw_status != AM_DEVICES_AMBT53_STATUS_SUCCESS) || ((ui32Status & MAILBOX_STATUS_D2M_EMPTY_Msk) == 1))
        {
            break;
        }
        // Always drain FIFO
        reg_rw_status = AMBT53_REG32_RD(MAILBOX_D2M_DATA_ADDR, &d2m_data);
        if (reg_rw_status != AM_DEVICES_AMBT53_STATUS_SUCCESS)
        {
            break;
        }
        // Save data if buffer is supplied
        if (buffer != NULL)
        {
            buffer[i] = d2m_data;
        }
    }
    // Drop the length if len is unavaliable
    if (len != NULL)
    {
        *len = i;
    }
    return reg_rw_status;
}

uint32_t am_devices_ambt53_mailbox_write_data(uint32_t *buffer, uint32_t *len)
{
    uint32_t i = 0;
    uint32_t ui32Status = 0;
    uint32_t reg_rw_status = AM_DEVICES_AMBT53_STATUS_FAIL;

    if (buffer != NULL)
    {
        for (i = 0; ((i < *len) & (i < MAILBOX_FIFO_MAX_DEEPTH)); i++)
        {
            reg_rw_status = AMBT53_REG32_RD(MAILBOX_STATUS_ADDR, &ui32Status);
            if ((reg_rw_status != AM_DEVICES_AMBT53_STATUS_SUCCESS) || ((ui32Status & MAILBOX_STATUS_M2D_FULL_Msk) != 0))
            {
                break;
            }
            reg_rw_status = AMBT53_REG32_WR(MAILBOX_M2D_DATA_ADDR, buffer[i]);
            if (reg_rw_status != AM_DEVICES_AMBT53_STATUS_SUCCESS)
            {
                break;
            }
        }
    }
    *len = i;
    return reg_rw_status;
}

uint32_t am_devices_ambt53_mailbox_get_status(uint32_t *status)
{
    *status = 0;
    return AMBT53_REG32_RD(MAILBOX_STATUS_ADDR, status);
}

uint32_t am_devices_ambt53_mailbox_get_d2m_err(uint32_t *error_bits)
{
    *error_bits = 0;
    return AMBT53_REG32_RD(MAILBOX_D2M_ERROR_ADDR, error_bits);
}

uint32_t am_devices_ambt53_mailbox_clear_d2m_err(uint32_t err_to_clear)
{
    uint32_t error_bits = 0;
    uint32_t reg_rw_status = AM_DEVICES_AMBT53_STATUS_FAIL;
    if ( am_devices_ambt53_mailbox_get_d2m_err(&error_bits) == AM_DEVICES_AMBT53_STATUS_SUCCESS )
    {
        error_bits &= err_to_clear;
        if (error_bits != 0)
        {
            reg_rw_status = AMBT53_REG32_WR(MAILBOX_D2M_ERROR_ADDR, error_bits);
            // TODO: we need to check this behavior with HW guy
            // According to test, Ambiq Mailbox need to wait several cycles to
            // make the clear error register operation effective, wait 25 cycles can
            // cover the worst case which 400M core clock : 16M AHB clock
            am_util_delay_cycles(25);
            return reg_rw_status;
        }
        else
        {
            return AM_DEVICES_AMBT53_STATUS_SUCCESS;
        }
    }
    return AM_DEVICES_AMBT53_STATUS_FAIL;
}

uint32_t am_devices_ambt53_mailbox_get_d2m_int_thr(uint32_t *threshold)
{
    *threshold = 0;
    return AMBT53_REG32_RD(MAILBOX_D2M_IT_ADDR, threshold);
}

uint32_t am_devices_ambt53_mailbox_get_d2m_int_status(uint32_t *int_status)
{
    *int_status = 0;
    return AMBT53_REG32_RD(MAILBOX_D2M_IS_ADDR, int_status);
}

uint32_t am_devices_ambt53_mailbox_clear_d2m_int(uint32_t int_to_clear)
{
    uint32_t int_bits = 0;
    uint32_t reg_rw_status = AM_DEVICES_AMBT53_STATUS_FAIL;

    if ( am_devices_ambt53_mailbox_get_d2m_int_status(&int_bits) == AM_DEVICES_AMBT53_STATUS_SUCCESS )
    {
        int_bits &= int_to_clear;
        if (int_bits != 0)
        {
            reg_rw_status = AMBT53_REG32_WR(MAILBOX_D2M_IS_ADDR, int_bits);
            // TODO: we need to check this behavior with HW guy
            // According to test, Ambiq Mailbox need to wait several cycles to
            // make the clear error register operation effective, wait 25 cycles can
            // cover the worst case which 400M core clock : 16M AHB clock
            am_util_delay_cycles(25);
            return reg_rw_status;
        }
        else
        {
            return AM_DEVICES_AMBT53_STATUS_SUCCESS;
        }
    }
    return AM_DEVICES_AMBT53_STATUS_FAIL;
}

uint32_t am_devices_ambt53_mailbox_disable_d2m_int(uint32_t interrupt)
{
    uint32_t d2m_ie = 0;
    uint32_t reg_rw_status = AM_DEVICES_AMBT53_STATUS_FAIL;

    // mailbox interrupt source only occupies lowest two bits
    reg_rw_status = AMBT53_REG32_RD(MAILBOX_D2M_IE_ADDR, &d2m_ie);
    if ((reg_rw_status == AM_DEVICES_AMBT53_STATUS_SUCCESS) && ((interrupt & 0x03) != 0))
    {
        d2m_ie &= ~(interrupt & 0x03);
        return AMBT53_REG32_WR(MAILBOX_D2M_IE_ADDR, d2m_ie);
    }
    return reg_rw_status;
}

uint32_t am_devices_ambt53_mailbox_get_m2d_int_thr(uint32_t *threshold)
{
    *threshold = 0;
    return AMBT53_REG32_RD(MAILBOX_M2D_IT_ADDR, threshold);
}

uint32_t am_devices_ambt53_mailbox_init(uint32_t threshold, uint32_t int_msk)
{
    // Clear the d2m FIFO
    if ( am_devices_ambt53_mailbox_flush_data(NULL, NULL) == AM_DEVICES_AMBT53_STATUS_SUCCESS )
    {
        // Clear the d2m error if exists
        if ( am_devices_ambt53_mailbox_clear_d2m_err(MAILBOX_ERROR_EMPTY_Msk | MAILBOX_ERROR_FULL_Msk )
            == AM_DEVICES_AMBT53_STATUS_SUCCESS)
        {
            // Clear the d2m interrupt status if exists
            if ( am_devices_ambt53_mailbox_clear_d2m_int(MAILBOX_INT_THRESHOLD_Msk | MAILBOX_INT_ERROR_Msk )
                == AM_DEVICES_AMBT53_STATUS_SUCCESS)
            {
                // Init FIFO threshold
                if (am_devices_ambt53_mailbox_set_d2m_int_thr(threshold) == AM_DEVICES_AMBT53_STATUS_SUCCESS)
                {
                    // Init mailbox INT
                    if ( am_devices_ambt53_mailbox_disable_d2m_int(MAILBOX_INT_THRESHOLD_Msk | MAILBOX_INT_ERROR_Msk )
                        == AM_DEVICES_AMBT53_STATUS_SUCCESS)
                    {
                        if ( am_devices_ambt53_mailbox_enable_d2m_int(int_msk) == AM_DEVICES_AMBT53_STATUS_SUCCESS )
                        {
                            return AM_DEVICES_AMBT53_STATUS_SUCCESS;
                        }
                    }
                }
            }
        }
    }

    return AM_DEVICES_AMBT53_STATUS_FAIL;
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

