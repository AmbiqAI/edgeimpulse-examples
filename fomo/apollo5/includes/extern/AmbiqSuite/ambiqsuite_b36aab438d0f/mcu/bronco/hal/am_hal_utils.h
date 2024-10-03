//*****************************************************************************
//
//! @file am_hal_utils.h
//!
//! @brief HAL Utility Functions
//!
//! @addtogroup utils4 Utils - HAL Utility Functions
//! @ingroup bronco_hal
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
#ifndef AM_HAL_UTILS_H
#define AM_HAL_UTILS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <stdbool.h>

//*****************************************************************************
//
//! @brief Use the bootrom to implement a spin loop.
//!
//! @param ui32us - Number of microseconds to delay.  Must be >=1; the
//! value of 0 will result in an extremely long delay.
//!
//! Use this function to implement a CPU busy waiting spin loop without cache
//! or delay uncertainties.
//!
//! @note Interrupts are not disabled during execution of this function.
//!       Therefore, any interrupt taken will affect the delay timing.
//
//*****************************************************************************
extern void am_hal_delay_us(uint32_t ui32us);

//*****************************************************************************
//
//! @brief Delays for a desired amount of cycles while also waiting for a
//! status to change a value.
//!
//! @param ui32usMaxDelay - Maximum number of ~1uS delay loops.
//! @param ui32Address    - Address of the register for the status change.
//! @param ui32Mask   - Mask for the status change.
//! @param ui32Value  - Target value for the status change.
//!
//! This function will delay for approximately the given number of microseconds
//! while checking for a status change, exiting when either the given time has
//! expired or the status change is detected.
//!
//! @returns AM_HAL_STATUS_SUCCESS = status change detected.
//!          AM_HAL_STATUS_TIMEOUT = timeout.
//
//*****************************************************************************
extern uint32_t am_hal_delay_us_status_change(uint32_t ui32usMaxDelay, uint32_t ui32Address,
                                 uint32_t ui32Mask, uint32_t ui32Value);

//*****************************************************************************
//
//! @brief Delays for a desired amount of cycles while also waiting for a
//! status to equal OR not-equal to a value.
//!
//! @param ui32usMaxDelay - Maximum number of ~1uS delay loops.
//! @param ui32Address    - Address of the register for the status change.
//! @param ui32Mask   - Mask for the status change.
//! @param ui32Value  - Target value for the status change.
//! @param bIsEqual   - Check for equal if true; not-equal if false.
//!
//! This function will delay for approximately the given number of microseconds
//! while checking for a status change, exiting when either the given time has
//! expired or the status change is detected.
//!
//! @returns 0 = timeout.
//!          1 = status change detected.
//
//*****************************************************************************
extern uint32_t am_hal_delay_us_status_check(uint32_t ui32usMaxDelay, uint32_t ui32Address,
                                uint32_t ui32Mask, uint32_t ui32Value,
                                bool bIsEqual);

//*****************************************************************************
//
//! @brief Read a uint32 value from a valid memory or peripheral location.
//!
//! @param pui32Address - The location to be read.
//!
//! Use this function to safely read a value from peripheral or memory locations.
//!
//! This function calls a function that resides BOOTROM or SRAM to do the actual
//! read, thus completely avoiding any conflict with flash or INFO space.
//!
//! @return The value read from the given address.
//
//*****************************************************************************
extern uint32_t am_hal_load_ui32(uint32_t *pui32Address);

//*****************************************************************************
//
//! @brief Use the bootrom to write to a location in SRAM or the system bus.
//!
//! @param pui32Address - Store the data value corresponding to this location.
//! @param ui32Data - 32-bit Data to be stored.
//!
//! Use this function to store a value to various peripheral or SRAM locations
//! that can not be touched from code running in SRAM or FLASH.  There is no
//! known need for this function in Apollo5 at this time.
//
//*****************************************************************************
extern void am_hal_store_ui32(uint32_t *pui32Address, uint32_t ui32Data);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_UTILS_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

