//*****************************************************************************
//
//  am_hal_systick.h
//! @file
//!
//! @brief Functions for accessing and configuring the SYSTICK.
//!
//! @addtogroup systick1 System Timer (SYSTICK)
//! @ingroup apollo1hal
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
#ifndef AM_HAL_SYSTICK_H
#define AM_HAL_SYSTICK_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern void am_hal_systick_start(void);
extern void am_hal_systick_stop(void);
extern void am_hal_systick_int_enable(void);
extern void am_hal_systick_int_disable(void);
extern uint32_t am_hal_systick_int_status_get(void);
extern void am_hal_systick_reset(void);
extern void am_hal_systick_load(uint32_t ui32LoadVal);
extern uint32_t am_hal_systick_count(void);
extern uint32_t am_hal_systick_wait_ticks(uint32_t ui32Ticks);
extern uint32_t am_hal_systick_delay_us(uint32_t ui32NumUs);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_SYSTICK_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
