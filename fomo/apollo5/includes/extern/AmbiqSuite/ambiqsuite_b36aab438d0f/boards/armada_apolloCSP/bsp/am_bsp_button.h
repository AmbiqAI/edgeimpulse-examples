//*****************************************************************************
//
//! @file am_bsp_button.h
//!
//! @brief armada-apolloBGA board support for button interaction.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_BSP_BUTTON_H
#define AM_BSP_BUTTON_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// BUTTON Definitions
//
//*****************************************************************************
#define AM_BSP_BUTTON_NUM                   0

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_bsp_button_init(void);
extern int32_t am_bsp_button_get(uint32_t num);
extern int32_t am_bsp_button_get_all(void);

#ifdef __cplusplus
}
#endif

#endif // AM_BSP_BUTTON_H

