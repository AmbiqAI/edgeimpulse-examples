//*****************************************************************************
//
//! @file psram.h
//!
//! @brief Header files for aps25616n/w958d6nw.
//!
//! This is just a temporary solution to make these two drivers coexist.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifdef WINBOND_PSRAM
#include "devices/am_devices_mspi_psram_w958d6nw.h"
#else
#include "devices/am_devices_mspi_psram_aps25616n.h"
#endif