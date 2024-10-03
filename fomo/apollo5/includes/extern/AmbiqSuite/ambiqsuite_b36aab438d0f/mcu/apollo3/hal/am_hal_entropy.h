//*****************************************************************************
//
//! @file am_hal_entropy.h
//!
//! @brief Functions for Generating True Random Numbers.
//!
//! @addtogroup entropy3 Entropy - Random Number Generation Functions
//! @ingroup apollo3_hal
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
#ifndef AM_HAL_ENTROPY_H
#define AM_HAL_ENTROPY_H

#include "stdint.h"
#include "stdbool.h"

#ifdef __cplusplus
extern "C"
{
#endif

//******************************************************************************
//
//! ENTROPY configuration
//
//! Use these macros to set the timer number and timer segment to be used for the
//! ENTROPY.
//
//******************************************************************************
#define AM_HAL_ENTROPY_CTIMER                  0
#define AM_HAL_ENTROPY_CTIMER_SEGMENT          A

//******************************************************************************
//
//! Helper macros for ENTROPY.
//
//******************************************************************************
#define AM_HAL_ENTROPY_CTIMER_SEG2(X) AM_HAL_CTIMER_TIMER ## X
#define AM_HAL_ENTROPY_CTIMER_SEG1(X) AM_HAL_ENTROPY_CTIMER_SEG2(X)
#define AM_HAL_ENTROPY_CTIMER_TIMERX AM_HAL_ENTROPY_CTIMER_SEG1(AM_HAL_ENTROPY_CTIMER_SEGMENT)

#define AM_HAL_ENTROPY_CTIMER_INT2(SEG, NUM) AM_HAL_CTIMER_INT_TIMER ## SEG ## NUM
#define AM_HAL_ENTROPY_CTIMER_INT1(SEG, NUM) AM_HAL_ENTROPY_CTIMER_INT2(SEG, NUM)
#define AM_HAL_ENTROPY_CTIMER_INT \
    AM_HAL_ENTROPY_CTIMER_INT1(AM_HAL_ENTROPY_CTIMER_SEGMENT, AM_HAL_ENTROPY_CTIMER)

//******************************************************************************
//
//! Types.
//
//******************************************************************************
typedef void (*am_hal_entropy_callback_t)(void *context);

//******************************************************************************
//
//! External Globals.
//
//******************************************************************************
extern uint32_t am_hal_entropy_timing_error_count;

//******************************************************************************
//
//! Prototypes.
//
//******************************************************************************
//*****************************************************************************
//! @brief Initialize the ENTROPY
//!
//*****************************************************************************
extern void am_hal_entropy_init(void);

//*****************************************************************************
//
//! @brief Start the entropy timers
//
//*****************************************************************************
extern void am_hal_entropy_enable(void);

//*****************************************************************************
//
//! @brief Stop the entropy timers
//
//*****************************************************************************
extern void am_hal_entropy_disable(void);

//******************************************************************************
//
//! @brief Place the next N values from the entropy collector into the output
//!        location.
//!
//! @param pui8Output  - where to put the data
//! @param ui32Length  - how much data you want
//! @param pfnCallback - function to call when the data is ready
//! @param pvContext   - will be passed to the callback. can be used for anything
//! @return uint32_t   - status
//
//*****************************************************************************
extern uint32_t am_hal_entropy_get_values(uint8_t *pui8Output, uint32_t ui32Length,
                                          am_hal_entropy_callback_t pfnCallback,
                                          void *pvContext);


#ifdef __cplusplus
}
#endif

#endif // AM_HAL_ENTROPY_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
