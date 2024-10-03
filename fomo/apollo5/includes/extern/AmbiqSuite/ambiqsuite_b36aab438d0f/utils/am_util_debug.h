//*****************************************************************************
//
//! @file am_util_debug.h
//!
//! @brief Useful functions for debugging.
//!
//! These functions and macros were created to assist with debugging. They are
//! intended to be as unintrusive as possible and designed to be removed from
//! the compilation of a project when they are no longer needed.
//!
//! @addtogroup debug Debug
//! @ingroup utils
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
#ifndef AM_UTIL_DEBUG_H
#define AM_UTIL_DEBUG_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
//! @name Debug printf macros.
//! @{
//
//*****************************************************************************
#ifdef AM_DEBUG_PRINTF

#define am_util_debug_printf_init(x)                                          \
    am_util_stdio_printf_init(x);

#define am_util_debug_printf(...)                                             \
    am_util_stdio_printf(__VA_ARGS__);

#else

#define am_util_debug_printf_init(...)
#define am_util_debug_printf(...)

#endif // AM_DEBUG_PRINTF
//! @}

//*****************************************************************************
//
//! @name Debug trace macros.
//! @{
//
//*****************************************************************************
#ifdef AM_DEBUG_TRACE

#define am_util_debug_trace_init(PinNumber)                                   \
    do                                                                        \
    {                                                                         \
        am_hal_gpio_out_bit_clear(PinNumber);                                 \
        am_hal_gpio_pin_config(PinNumber, AM_HAL_GPIO_OUTPUT);                \
    }                                                                         \
    while (0)

#define am_util_debug_trace_start(PinNumber)                                  \
    am_hal_gpio_out_bit_set(PinNumber)

#define am_util_debug_trace_end(PinNumber)                                    \
    am_hal_gpio_out_bit_clear(PinNumber)

#else

#define am_util_debug_trace_init(PinNumber)
#define am_util_debug_trace_start(PinNumber)
#define am_util_debug_trace_end(PinNumber)

#endif // AM_DEBUG_TRACE
//! @}

#ifdef __cplusplus
}
#endif

#endif // AM_UTIL_DEBUG_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
