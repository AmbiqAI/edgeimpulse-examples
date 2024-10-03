//*****************************************************************************
//
//! @file crc_task.c
//!
//! @brief Constantly run a CRC using a timer
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
#include <stdbool.h>

#include "am_mcu_apollo.h"
#include "am_util.h"

#include "crypto_stress_test_config.h"
#include "crc_task.h"
#include "string.h"
//******************************************************************************
//
// CRC Configuration
//
//******************************************************************************
#define CRC_START_ADDRESS    0x18000
#define CRC_LENGTH           (512 * 1024)

//******************************************************************************
//
// Globals for CRC
//
//******************************************************************************
uint32_t g_ui32LastCRCResult = 0xFFFFFFFF;
uint32_t g_ui32CRCCount = 0;
extern volatile bool bCryptoInUse;

uint32_t info0_ref[4];
uint32_t info1_ref[4];

//******************************************************************************
//
// Initialize the timer for the CRC task.
//
//******************************************************************************
void
crc_task_init(void)
{
#if ENABLE_INFO_ACCESS
    // Initialize the reference data
    am_hal_info1_read(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1, AM_REG_OTP_INFO1_SBR_SDCERT_ADDR_O / 4, 4, info1_ref);
    am_hal_info0_read(AM_HAL_INFO_INFOSPACE_CURRENT_INFO0, AM_REG_OTP_INFO0_SIGNATURE0_O, 4, info0_ref);
#endif
#if ENABLE_TIMER
    am_hal_timer_config_t sTimerConfig;

    am_hal_timer_default_config_set(&sTimerConfig);
    sTimerConfig.eFunction = AM_HAL_TIMER_FN_UPCOUNT;
    sTimerConfig.ui32PatternLimit = 0;
    sTimerConfig.ui32Compare1 = (6 * TIMER_TASK_DELAY);  // Default clock is 6MHz

    if (AM_HAL_STATUS_SUCCESS != am_hal_timer_config(0, &sTimerConfig))
    {
        PRINT("Failed to configure TIMER0\n");
    }

    am_hal_timer_clear(0);
    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
    NVIC_SetPriority(TIMER0_IRQn, 1);
#endif
}

//******************************************************************************
//
// Enable the CRC task.
//
//******************************************************************************
void
crc_task_enable(void)
{
#if ENABLE_TIMER
    am_hal_timer_interrupt_enable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
    NVIC_SetPriority(TIMER0_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(TIMER0_IRQn);
    am_hal_timer_start(0);
#endif
}

//******************************************************************************
//
// Disable the CRC task.
//
//******************************************************************************
void
crc_task_disable(void)
{
#if ENABLE_TIMER
    am_hal_timer_stop(0);
    NVIC_DisableIRQ(TIMER0_IRQn);
    am_hal_timer_interrupt_disable(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
#endif
}

//******************************************************************************
//
// Perform a CRC
//
// The CRC is done in the background by hardware. This function simply checks
// to see if there is an operation in progress, and determines whether we need
// to run another one.
//
//******************************************************************************
void
crc_operation(void)
{
#if ENABLE_CRC
    if ((SECURITY->CTRL & SECURITY_CTRL_ENABLE_Msk) == 0)
    {
        if (g_ui32CRCCount > 0)
        {
            g_ui32LastCRCResult = SECURITY->RESULT;
        }
        g_ui32CRCCount++;

        //
        // Program the CRC engine to compute the crc
        //
        SECURITY->RESULT          = 0xFFFFFFFF;
        SECURITY->SRCADDR         = CRC_START_ADDRESS;
        SECURITY->LEN             = CRC_LENGTH;
        SECURITY->CTRL_b.FUNCTION = SECURITY_CTRL_FUNCTION_CRC32;

        //
        // Start the CRC
        //
        SECURITY->CTRL_b.ENABLE = 1;
    }
#endif
#if ENABLE_INFO_ACCESS
    uint32_t dummy[4];
    if (!bCryptoInUse)
    {
        am_hal_info0_read(AM_HAL_INFO_INFOSPACE_CURRENT_INFO0, AM_REG_OTP_INFO0_SIGNATURE0_O, 4, dummy);
        if (memcmp(dummy, info0_ref, (size_t)16))
        {
            while(1);
        }
        am_hal_info1_read(AM_HAL_INFO_INFOSPACE_CURRENT_INFO1, AM_REG_OTP_INFO1_SBR_SDCERT_ADDR_O / 4, 4, dummy);
        if (memcmp(dummy, info1_ref, (size_t)16))
        {
            while(1);
        }
    }
#endif
}

//******************************************************************************
//
// TIMER ISR
//
//******************************************************************************
void
am_timer00_isr(void)
{
    am_hal_gpio_state_write(TIMER_GPIO, AM_HAL_GPIO_OUTPUT_SET);

    am_hal_timer_interrupt_clear(AM_HAL_TIMER_MASK(0, AM_HAL_TIMER_COMPARE1));
    am_hal_timer_clear(0);

    crc_operation();

    am_hal_gpio_state_write(TIMER_GPIO, AM_HAL_GPIO_OUTPUT_CLEAR);
}
