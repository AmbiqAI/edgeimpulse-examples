//*****************************************************************************
//
//! @file am_widget_gpio.h
//!
//! @brief
//!
//! @addtogroup hal Hardware Abstraction Layer (HAL)
//! @addtogroup ctimer Counter/Timer (CTIMER)
//! @ingroup hal
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
#ifndef AM_WIDGET_GPIO_H
#define AM_WIDGET_GPIO_H

#ifdef __cplusplus
extern "C"
{
#endif

extern uint32_t am_widget_gpio_int_check(uint32_t ui32PinTestIntrpt,
                                         uint32_t ui32PinDrive,
                                         am_hal_gpio_intdir_e eIntType,
                                         am_hal_gpio_int_channel_e eChl);



#ifdef __cplusplus
}
#endif

#endif // AM_WIDGET_GPIO_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
