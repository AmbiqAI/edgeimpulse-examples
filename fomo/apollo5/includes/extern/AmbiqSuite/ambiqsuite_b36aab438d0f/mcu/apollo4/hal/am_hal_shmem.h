//*****************************************************************************
//
//  am_hal_shmem.h
//! @file
//!
//! @brief all structures and variables shared between DSP0 , DSP1 and ARM.
//!
//!
//! @addtogroup sharemem4 SharedMem
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
#ifndef AM_HAL_SHMEM_H
#define AM_HAL_SHMEM_H

#include <stdint.h>
#include <stdbool.h>
#include "am_hal_ipc.h"
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define AM_HAL_SHARED_MEM_ADDR   0x12345678 // TBD
#define MAX_NO_OF_DSP_INTERRUPT  160  //!< number of raw interrupt exist
#define INVALID_INTERRUPT        31  //!< when a pin does not connected to any interrupt
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
} HalDSPInt_t;

// This data structure will be allocated at a fixed location in shared memory
typedef struct
{
    // Each core update only ipcInfo[coreID] and the other cores only read this value.
    // So all addresses inside ipcInfo[coreID] are the memory map view of the 'coreID'.

    struct
    {
        am_hal_shortMsgQ_descr_t *pReadDescr; // Address of Receiver's local Read Descriptor
        am_hal_shortMsgQ_descr_t *pWriteDescr; // Address of Sender's local Write Descriptor
    } ipcInfo[AM_HAL_NUM_CORES][AM_HAL_NUM_CORES]; // Receiver: Sender
    uint32_t                 eventMask[AM_HAL_NUM_CORES]; // Possible Events bitmask for receiver
} am_hal_ipc_shared_state_t;

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
    HalDSPInt_t  sHalDSPInt[MAX_NO_OF_DSP_INTERRUPT];
    am_hal_ipc_shared_state_t sIpcSharedState;
} HalSharedMem_t;

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************
extern HalSharedMem_t *sHalSharedMem;


//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************


#ifdef __cplusplus
}
#endif

#endif // AM_HAL_SHMEM_H
