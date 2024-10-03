//*****************************************************************************
//
//! @file pwr_pwrControl_utils.h
//!
//! @brief Set power mode(s) for the example
//!
//! @addtogroup power_examples Power Examples
//!
//! @defgroup pwr_iom_spi SPI Power Example
//! @ingroup power_examples
//! @{
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
#ifndef PWRCONTROL_UTILS_H
#define PWRCONTROL_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"

//*****************************************************************************
//
//! @brief set the power modes for this example
//
//*****************************************************************************
uint32_t pwrctrl_set_low_power(void);

//*****************************************************************************
//
//! @brief Fill buffer with incrementing bytes ( 0-255 )
//!
//! This function fills the given array with incrementing bytes.
//!
//! @param pui8Array Pointer to the array to fill
//! @param i32Size Size of the array in bytes
//
//*****************************************************************************
extern void pwrctrl_fill_buff_incrementing_bytes(uint8_t *pui8Array, int32_t i32Size);
//*****************************************************************************
//
//! @brief Fill buffer with alternating bits patterns 0xA5 0x5A
//!
//! This function fills the given array with alternating bits for the given size.
//! Each two bytes alternate bits  so this will work well with octal or smaller transfers
//! With a hex transfer, the pattern is the same and output pins won't change
//!
//! @param pui8Array Pointer to the array to fill (longword-aligned)
//! @param ui32TransferBitWidth the (m)spi bit width 1,2,4,8,16
//! @param i32Size Size of the array in bytes (multiple of 4)
//
//*****************************************************************************
extern void pwrctrl_fill_buff_alternating_bits(uint8_t *pui8Array,
                                               uint32_t ui32TransferBitWidth,
                                               int32_t i32Size);

//*****************************************************************************
//
//! @brief fill a buffer with a repeating set of random values
//!
//! @param pui8Array   pointer to destination for random value
//! @param i32Size    number of bytes in destination
//
//*****************************************************************************
void pwrctrl_fill_buff_random(uint8_t *pui8Array, int32_t i32Size);

#endif //PWR_PWRCONTROL_UTILS_H
//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

