//*****************************************************************************
//
//  am_hal_dsp_common.h
//! @file
//!
//! @brief all structures and variables shared between DSP0 , DSP1 and ARM.
//!
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
#ifndef AM_HAL_COMMON_H
#define AM_HAL_COMMON_H

#include <stdint.h>
#include <stdbool.h>
#include "..\am_hal_ipc.h"
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

#define MAX_NO_OF_DSP_INTERRUPT  160  //!< number of raw interrupt exist
#define  INVALID_INTERRUPT   31  //!< when a pin does not connected to any interrupt
//*****************************************************************************
//
//! @brief Interrupt structure definition populate by ARM(by default)
//!
//! This structure defines how different raw interrupts are connected to DSP
//! interrupts. The 160 interrupts are passing through two OR structures and
//! then reach to DSP interrupt pins. This structure defines the one to one
//! mapping between Draw interrupts and DSP interrupt pin.
//
//*****************************************************************************
typedef struct
{
// todo: After defining this structure, combine all the fields in one byte/word
    //! 5 bits: from 0 to 31(the actual DSP interrupt number)
    uint8_t intNo;

    //! 1 bit for OR1.
    uint8_t Or1;

    //! 1 bit for OR2.
    uint8_t Or2;
}HalDSPInt_t;

//*****************************************************************************
//
//! @brief The placeholder for shared memory structure for all DSP-ARM data
//!
//! This structure has all the shared data between DSP and ARM. This struture is
//! initialize by ARM and then DSP can access it. In case of IPC DSP also can
//! write to this structure.
//!
//! sHalDSPInt  :   for each raw interrupt has a structure which defines
//                  the routing between the raw interrupt and the DSP interrupt
//!                 pins
//
//*****************************************************************************
typedef struct
{
    //! The interrupt connection between raw interrupts and DSP interrupt pins
    HalDSPInt_t sHalDSPInt[MAX_NO_OF_DSP_INTERRUPT];
}HalSharedMem_t;

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_COMMON_H
