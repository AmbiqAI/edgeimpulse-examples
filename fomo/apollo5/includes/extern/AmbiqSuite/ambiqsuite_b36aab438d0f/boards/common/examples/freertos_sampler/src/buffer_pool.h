//*****************************************************************************
//
//! @file buffer_pool.h
//!
//! @brief String Storage Utility for cross thread communication.
//!
//! These functions provide a quick allocate and free mechanism for 64 byte
//! string storage arrays. These are thread safe (use ENTER/EXIT Critical
//! section brackets), but they use a very fast allocate/deallocate method.
//!
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef BUFFER_POOL_H
#define BUFFER_POOL_H

#ifdef __cplusplus
extern "C"
{
#endif
//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************

//*****************************************************************************
//
// Typedefs
//
//*****************************************************************************

//*****************************************************************************
//
// External variable definitions
//
//*****************************************************************************


//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern volatile uint32_t * buffer_pool_get(void);
extern void buffer_pool_free(uint32_t * Buffer);

#ifdef __cplusplus
}
#endif

#endif // BUFFER_POOL_H




