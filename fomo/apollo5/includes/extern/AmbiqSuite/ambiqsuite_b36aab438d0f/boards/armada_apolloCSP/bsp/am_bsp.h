//*****************************************************************************
//
//! @file am_bsp.h
//!
//! @brief Functions common to the BSP.
//!
//! The functions supported by this include file implement the common portions
//! of the Ambiq Board Suppoprt Package. Device specific functions are
//! implemented within related files.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_BSP_H
#define AM_BSP_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// All BSP includes
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_bsp_button.h"
#include "am_bsp_gpio.h"
#include "am_bsp_iom.h"
#include "am_bsp_led.h"
#include "am_bsp_uart.h"

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_bsp_init(void);
extern void am_bsp_itm_string_print(char *pcString);
extern void am_bsp_uart_string_print(char *pcString);

//##### INTERNAL BEGIN #####
extern void am_bsp_rig4lp(void);
//##### INTERNAL END #####

#ifdef __cplusplus
}
#endif

#endif // AM_BSP_H
