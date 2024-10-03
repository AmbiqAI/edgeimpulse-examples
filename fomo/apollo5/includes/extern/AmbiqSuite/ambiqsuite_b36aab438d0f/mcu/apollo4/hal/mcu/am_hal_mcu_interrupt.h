//*****************************************************************************
//
//  am_hal_mcu_interrupt.h
//! @file
//!
//! @brief Helper functions supporting interrupts and NVIC operation.
//!
//! These functions may be used for NVIC-level interrupt configuration.
//!
//! @addtogroup interrupt4 Interrupt (ARM NVIC support functions)
//! @ingroup apollo4hal
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
#ifndef AM_HAL_MCU_INTERRUPT_H
#define AM_HAL_MCU_INTERRUPT_H

#ifdef __cplusplus
extern "C"
{
#endif

//
// Define the last peripheral interrupt as AM_HAL_INTERRUPT_MAX.
// The total number of interrupts in the vector table is therefore
// (AM_HAL_INTERRUPT_MAX + 1 + 16).
//
#define AM_HAL_INTERRUPT_MAX                (MAX_IRQn - 1)

extern uint32_t am_hal_interrupt_master_disable(void);
extern uint32_t am_hal_interrupt_master_enable(void);
extern void am_hal_interrupt_master_set(uint32_t ui32InterruptState);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_MCU_INTERRUPT_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
