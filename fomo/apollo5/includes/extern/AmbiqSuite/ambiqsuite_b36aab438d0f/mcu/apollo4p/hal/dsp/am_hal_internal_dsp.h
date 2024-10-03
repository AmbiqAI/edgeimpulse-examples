//*****************************************************************************
//
//! @file am_hal_internal_dsp.h
//!
//! @brief Definitions used internally for DSP HAL implementation.
//!
//! @addtogroup dsp_internal DSP Internal Functionality
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
#ifndef AM_HAL_INTERNAL_DSP_H
#define AM_HAL_INTERNAL_DSP_H

#ifdef __cplusplus
extern "C"
{
#endif

//#define NULL 0
#define MAX_NO_OF_HANDLERS    32   //!< number of handlers supported per DSP core
//*****************************************************************************
//
//! @brief the linked list structure for the interrupt handlers defined by the user code
//!
//! This structure keeps the handler address with some information about the
//! status register bit associated with the interrupt.
//!
//!
//
//*****************************************************************************
typedef struct
{
    void     *pHandler;
    void     *pHandlerCtxt;
    void     *pIRQStatusRegAdd;
    uint32_t IRQStatusRegVal;
    void     *dNextIntHandler;
    uint8_t ui5IntterruptNo;      // This is a 5bit value(3 to 25) for the current interrupt for this handler
    //  this is used by the am_hal_interrupt_register_handler to find all interrupts associated for the current
    // DSP interrupt.
    uint8_t ui1FirstInterrupt;       // when this flag is set. Then this Handler is the head of the linked list.
    uint8_t ui8PeripheralIntNo;       // The source interrupt number for this handler. This is useful for unregister function
    uint8_t ui8PreviousElementIndex;       // when 0xff then this Handler is the head of the linked list, else this is the link.
    uint32_t reserve[2];     // two of the members here can be used for one extra register access
}IntHandlerStruct;

#ifdef __cplusplus
}
#endif

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
void     am_hal_dsp_dispatcher(IntHandlerStruct * ihs); // this function is defined in assembly

#endif // AM_HAL_INTERNAL_DSP_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
