//*****************************************************************************
//
//  am_hal_dsp_interrupt.h
//! @file
//!
//! @brief Helper functions supporting DSP interrupts.
//!
//! @addtogroup interrupt4 Interrupt (DSP NVIC support functions)
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
#ifndef AM_HAL_DSP_INTERRUPT_H
#define AM_HAL_DSP_INTERRUPT_H

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
}
#endif

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
void     am_hal_interrupt_disable(uint32_t ui8InterruptNo);
void     am_hal_interrupt_enable(uint32_t ui8InterruptNo);
uint32_t am_hal_interrupt_master_disable(void);
uint32_t am_hal_interrupt_master_enable(void);
uint32_t am_hal_interrupt_master_restore(uint32_t ui32Restoreval);
uint32_t am_hal_interrupt_master_set(uint32_t ui32Level);
uint32_t am_hal_interrupt_register_handler(uint32_t ui32Interrupt, void *pHandler, void *pHandlerCtxt);

#endif // AM_HAL_DSP_INTERRUPT_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
