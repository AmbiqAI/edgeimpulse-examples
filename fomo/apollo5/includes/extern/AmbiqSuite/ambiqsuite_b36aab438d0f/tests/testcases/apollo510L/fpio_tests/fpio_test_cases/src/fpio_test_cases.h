//*****************************************************************************
//
//! @file fpio_test_cases.h 
//!
//! @brief General Purpose Input Output Functionality
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef FGPIO_TEST_CASE_H
#define FGPIO_TEST_CASE_H

#ifdef __cplusplus
extern "C"
{
#endif


//*****************************************************************************
//
//! @name Helper macros
//! @{
//
//*****************************************************************************
#define AM_HAL_FPIO_RDn(pin)    ((volatile uint32_t *)&FPIO->RD0  + (((pin) >> 5) & 0x7))
#define AM_HAL_FPIO_WTn(pin)    ((volatile uint32_t *)&FPIO->WT0  + (((pin) >> 5) & 0x7))
#define AM_HAL_FPIO_WTCn(pin)   ((volatile uint32_t *)&FPIO->WTC0 + (((pin) >> 5) & 0x7))
#define AM_HAL_FPIO_WTSn(pin)   ((volatile uint32_t *)&FPIO->WTS0 + (((pin) >> 5) & 0x7))
#define AM_HAL_FPIO_ENn(pin)    ((volatile uint32_t *)&FPIO->EN0  + (((pin) >> 5) & 0x7))
#define AM_HAL_FPIO_ENCn(pin)   ((volatile uint32_t *)&FPIO->ENC0 + (((pin) >> 5) & 0x7))
#define AM_HAL_FPIO_ENSn(pin)   ((volatile uint32_t *)&FPIO->ENS0 + (((pin) >> 5) & 0x7))
//! @}

//*****************************************************************************
//
//! @brief Macros to read GPIO values in an optimized manner.
//!
//! @param n - The GPIO number to be read.
//!
//! In almost all cases, it is reasonable to use am_hal_fpio_state_read() to
//! read GPIO values with all of the inherent error checking, critical
//! sectioning, and general safety.
//!
//! However, occasionally there is a need to read a GPIO value in an optimized
//! manner.  These 3 macros will accomplish that.  Each macro will return a
//! value of 1 or 0.
//!
//! Note that the macros are named as lower-case counterparts to the
//! enumerations for the am_hal_fpio_state_read() function.  That is:
//!
//!     AM_HAL_FPIO_INPUT_READ  -> am_hal_fpio_input_read(n)
//!     AM_HAL_FPIO_OUTPUT_READ -> am_hal_fpio_output_read(n)
//!     AM_HAL_FPIO_ENABLE_READ -> am_hal_fpio_enable_read(n)
//!
//! @return Each macro will return a 1 or 0 per the value of the requested GPIO.
//!
//
//*****************************************************************************
#define am_hal_fpio_input_read(n)   ((*AM_HAL_FPIO_RDn((n)) >> ((n) % 32)) & 1)
#define am_hal_fpio_output_read(n)  ((*AM_HAL_FPIO_WTn((n)) >> ((n) % 32)) & 1)
#define am_hal_fpio_enable_read(n)  ((*AM_HAL_FPIO_ENn((n)) >> ((n) % 32)) & 1)

//*****************************************************************************
//
//! @brief Macros to write GPIO values in an optimized manner.
//!
//! @param n - The GPIO number to be written.
//!
//! In almost all cases, it is reasonable to use am_hal_fpio_state_write() to
//! write GPIO values with all of the inherent error checking, critical
//! sectioning, and general safety.
//!
//! However, occasionally there is a need to write a GPIO value in an optimized
//! manner.  These 3 macros will accomplish that.
//!
//! Note that the macros are named as lower-case counterparts to the
//! enumerations for the am_hal_fpio_state_read() function.  That is:
//!
//!    AM_HAL_FPIO_OUTPUT_CLEAR                -> am_hal_fpio_output_clear(n)
//!    AM_HAL_FPIO_OUTPUT_SET                  -> am_hal_fpio_output_set(n)
//!    AM_HAL_FPIO_OUTPUT_TOGGLE               -> am_hal_fpio_output_toggle(n)
//!    AM_HAL_FPIO_OUTPUT_TRISTATE_OUTPUT_DIS  -> am_hal_fpio_output_tristate_output_dis(n)
//!    AM_HAL_FPIO_OUTPUT_TRISTATE_OUTPUT_EN   -> am_hal_fpio_output_tristate_output_en(n)
//!    AM_HAL_FPIO_OUTPUT_TRISTATE_OUTPUT_TOG  -> am_hal_fpio_output_toggle(n).
//!
//! It's important to note that the macros:
//!     am_hal_fpio_output_tristate_output_en()
//!     am_hal_fpio_output_tristate_output_dis()
//! operate on the output enable of the pin. Therefore,
//!     am_hal_fpio_output_tristate_output_en() enables the output,
//!     am_hal_fpio_output_tristate_output_dis() puts the pin into hi-impedance.
//! Given this behavior, perhaps more appropriate names might have been:
//!     am_hal_fpio_output_tristate_outputen()
//!     am_hal_fpio_output_tristate_outputdis()
//!
//*****************************************************************************
#define am_hal_fpio_output_clear(n)             (*AM_HAL_FPIO_WTCn((n)) = AM_HAL_MASK32(n))
#define am_hal_fpio_output_set(n)               (*AM_HAL_FPIO_WTSn((n)) = AM_HAL_MASK32(n))
#define am_hal_fpio_output_toggle(n)                                            \
    if ( 1 )                                                                    \
    {                                                                           \
        AM_CRITICAL_BEGIN                                                       \
        (*AM_HAL_FPIO_WTn((n)) ^= AM_HAL_MASK32(n));                            \
        AM_CRITICAL_END                                                         \
    }

#define am_hal_fpio_output_tristate_output_dis(n)  (*AM_HAL_FPIO_ENCn((n)) = AM_HAL_MASK32(n))
#define am_hal_fpio_output_tristate_output_en(n)   (*AM_HAL_FPIO_ENSn((n)) = AM_HAL_MASK32(n))
#define am_hal_fpio_output_tristate_output_tog(n)                                   \
    if ( 1 )                                                                    \
    {                                                                           \
        AM_CRITICAL_BEGIN                                                       \
        (*AM_HAL_FPIO_ENn((n)) ^=  AM_HAL_MASK32(n));                           \
        AM_CRITICAL_END                                                         \
    }

//*****************************************************************************
//
// These macros have been deprecated due to improper naming conventions.
//
//*****************************************************************************
#define am_hal_fpio_output_tristate_disable(n)   am_hal_fpio_output_tristate_output_dis(n)
#define am_hal_fpio_output_tristate_enable(n)    am_hal_fpio_output_tristate_output_en(n)
#define am_hal_fpio_output_tristate_toggle(n)    am_hal_fpio_output_tristate_output_tog(n)

//*****************************************************************************
//
// External functions.
//
//*****************************************************************************


//*****************************************************************************
//
//! @brief Read FPIO state values
//!
//! @param ui32GpioNum is the pin to read.
//! @param eReadType is the type of read to perform.
//! @param pui32ReadState returns the requested value.
//!
//! This function allows the caller to read any of the following values
//! associated with a FPIO:
//! - Input value
//! - Output value
//! - Output enable value
//!
//! @return Standard HAL status code.
//
//*****************************************************************************
extern uint32_t am_hal_fpio_state_read(uint32_t ui32GpioNum,
                                       am_hal_gpio_read_type_e eReadType,
                                       uint32_t *pui32ReadState);

//*****************************************************************************
//
//! @brief Write FPIO state values
//!
//! @param ui32GpioNum is the pin to write to
//! @param eWriteType is the type of write to perform.
//!
//! This function allows the caller to write any of the following values
//! associated with a FPIO:
//! - Ouput drive value
//! - Output enable value
//!
//! @return Standard HAL status code.
//
//*****************************************************************************
extern uint32_t am_hal_fpio_state_write(uint32_t ui32GpioNum,
                                        am_hal_gpio_write_type_e eWriteType);

#ifdef __cplusplus
}
#endif

#endif // FGPIO_TEST_CASE_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
