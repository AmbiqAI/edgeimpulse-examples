//*****************************************************************************
//
//! @file psram.h
//!
//! @brief Header files for aps12808l or aps25616n.
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

#ifdef AM_PART_APOLLO4P
#include "devices/am_devices_mspi_psram_aps25616n.h"
#else
#include "devices/am_devices_mspi_psram_aps12808l.h"
#endif
