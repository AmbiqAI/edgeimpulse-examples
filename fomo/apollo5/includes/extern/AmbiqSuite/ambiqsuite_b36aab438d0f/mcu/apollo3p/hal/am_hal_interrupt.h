//*****************************************************************************
//
//! @file am_hal_interrupt.h
//!
//! @brief Helper functions supporting interrupts and NVIC operation.
//!
//! These functions may be used for NVIC-level interrupt configuration.
//!
//! @addtogroup interrupt3p Interrupt - ARM NVIC support functions
//! @ingroup apollo3p_hal
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
#ifndef AM_HAL_INTERRUPT_H
#define AM_HAL_INTERRUPT_H

#ifdef __cplusplus
extern "C"
{
#endif

//
//! Define the last peripheral interrupt as AM_HAL_INTERRUPT_MAX.
//! The total number of interrupts in the vector table is therefore
//! (AM_HAL_INTERRUPT_MAX + 1 + 16).
//
#define AM_HAL_INTERRUPT_MAX                (CLKGEN_IRQn)


//*****************************************************************************
//! @brief disable all interrupts
//! @details CPSID i sets PRIMASK.PM to 1
//!
//! @return uint32_t
//*****************************************************************************
extern uint32_t am_hal_interrupt_master_disable(void);
//*****************************************************************************
//! @brief  enable all interrupts
//! @details CPSIE i sets PRIMASK.PM to 0
//!
//! @return uint32_t
//*****************************************************************************
extern uint32_t am_hal_interrupt_master_enable(void);
//*****************************************************************************
//! @brief set primask
//!
//! @param ui32InterruptState set primask to this value
//*****************************************************************************
extern void am_hal_interrupt_master_set(uint32_t ui32InterruptState);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_INTERRUPT_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
