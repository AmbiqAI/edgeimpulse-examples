//*****************************************************************************
//
//! @file am_hal_dsp_interrupt.h
//!
//! @brief Helper functions supporting DSP interrupts.
//!
//! @addtogroup dsp_interrupt Interrupt (DSP NVIC support functions)
//! @ingroup apollo4p_hal
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

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
//*****************************************************************************
//
//! @brief am_hal_interrupt_disable disables the raw interrupt( no more interrupt from the source)
//!
//! The function disable the raw interrupt by disconnecting the interrupt on the
//! source. The interrupt can be disable from the source or disable inside the DSP
//! by INTERRUPT register. This function masks the interrupt on the source and does
//! change the INTERRUPT register.
//!
//! @param ui8InterruptNo - The raw interrupt number (0 to 159)
//
//*****************************************************************************
extern void am_hal_interrupt_disable(uint32_t ui8InterruptNo);

//*****************************************************************************
//
//! @brief am_hal_interrupt_enable enables the raw interrupt
//!
//! The function restore the raw interrupt connection between the source and DSP
//! pin. To enable an interrupt the raw source should be connected to DSP input
//! pins and then the corresponding INTERRUPT flag should be enabled.
//!
//! @param ui8InterruptNo  : The raw interrupt number (0 to 159)
//
//*****************************************************************************
extern void am_hal_interrupt_enable(uint32_t ui8InterruptNo);

//*****************************************************************************
//
//! @brief am_hal_interrupt_master_disable disables all interrupt on DSP
//!
//! To disable all interrupts in DSP, the interrupt level should be raised to a
//! level higher the maximum level. Currently the maximum level is 6. This function
//! raises the interrupt level to 15 to disable all interrupt (the PS register have
//! 4 bits for interrupt level).
//! Note: disabling interrupt can not disable the exceptions.
//!
//! @return the contents of PS register.
//
//*****************************************************************************
extern uint32_t am_hal_interrupt_master_disable(void);

//*****************************************************************************
//
//! @brief am_hal_interrupt_master_enable enables all interrupt on DSP
//!
//! To enable all interrupts in DSP, the interrupt level should be set to zero.
//! This function regardless of the current interrupt level, changes the interrupt
//! level to zero. The interrupt level is specified by 4 bits inside PS register.
//!
//! @return the contents of PS register.
//
//*****************************************************************************
extern uint32_t am_hal_interrupt_master_enable(void);

//*****************************************************************************
//
//! @brief am_hal_interrupt_master_restore restores the interrupt level
//!
//! The function restore the interrupt level to its previous known state. DSP has
//! six level of interrupt. 18+7 of interrupts are level one and 5 more interrupts
//! are at level 2 to 6. changing the interrupt level can enable or disable all
//! interrupts with the same levels.
//!
//! @param ui32Restoreval
//! @return the contents of PS register.
//
//*****************************************************************************
extern uint32_t am_hal_interrupt_master_restore(uint32_t ui32Restoreval);

//*****************************************************************************
//
//! @brief am_hal_interrupt_master_set set the interrupt level
//!
//! The function can change the interrupt level to any level. By changing the
//! interrupt level, all interrupts with the lower level are disabled.
//!
//! @param ui32Level - The required interrupt level
//! @return the contents of PS register.
//
//*****************************************************************************
extern uint32_t am_hal_interrupt_master_set(uint32_t ui32Level);

//*****************************************************************************
//
//! @brief am_hal_interrupt_register_handler registers a handler for interrupt
//!
//! The function can change the interrupt level to any level. By changing the
//! interrupt level, all interrupts with the lower level are disabled.
//!
//! @param ui32Interrupt - The interrupt number (0 .. XCHAL_NUM_INTERRUPTS - 1)
//!
//! @param pHandler      - Address of interrupt handler to be registered.
//!                        Passing NULL will uninstall an existing handler.
//!
//! @param pHandlerCtxt  - Parameter to be passed to handler when invoked.
//!
//! @return             Zero on success, negative number on error.
//
//*****************************************************************************
extern uint32_t am_hal_interrupt_register_handler(uint32_t ui32Interrupt,
                                                    void *pHandler,
                                                    void *pHandlerCtxt);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_DSP_INTERRUPT_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
