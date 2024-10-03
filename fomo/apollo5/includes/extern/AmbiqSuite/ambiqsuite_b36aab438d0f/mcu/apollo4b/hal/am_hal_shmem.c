//*****************************************************************************
//
//! @file am_hal_shmem.c
//!
//! @brief Shared memory data structure shared across cores
//!
//! @addtogroup sharemem4_4b ShMem - Share Memory
//! @ingroup apollo4b_hal
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
#include <stdint.h>
#include <stdbool.h>
#include "am_hal_shmem.h"
//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
//
//! The sHalSharedMem is defined in this file and then used by other hal functions.
//! sHalSharedMem must be initialized to pointing to a correct location in shaed MEM
//! before DSP or ARM using the hal interrupt functions.
//
HalSharedMem_t *sHalSharedMem = (HalSharedMem_t *) AM_HAL_SHARED_MEM_ADDR;

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
