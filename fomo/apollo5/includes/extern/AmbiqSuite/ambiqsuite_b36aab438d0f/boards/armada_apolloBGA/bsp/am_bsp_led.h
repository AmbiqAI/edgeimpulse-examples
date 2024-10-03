//*****************************************************************************
//
//! @file am_bsp_led.h
//!
//! @brief armada-apolloBGA board support for GPIO
//!
//! Structures and functions to support LED operations.
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

#ifndef AM_BSP_LED_H
#define AM_BSP_LED_H

#ifdef __cplusplus
extern "C"
{
#endif


//*****************************************************************************
//
// BUTTON Definitions
//
//*****************************************************************************
#define AM_BSP_LED_NUM                      7

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_bsp_led_init(void);
extern void am_bsp_led_on(uint32_t ui32LEDNum);
extern void am_bsp_led_off(uint32_t ui32LEDNum);
extern void am_bsp_led_out(uint32_t ui32Value);
extern int am_bsp_led_get(uint32_t ui32LEDNum);
extern void am_bsp_led_toggle(uint32_t ui32LEDNum);

#ifdef __cplusplus
}
#endif

#endif // AM_BSP_LED_H

