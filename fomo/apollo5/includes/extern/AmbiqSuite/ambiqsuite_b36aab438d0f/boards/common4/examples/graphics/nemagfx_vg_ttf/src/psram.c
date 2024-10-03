//*****************************************************************************
//
//! @file psram.c
//!
//! @brief A duplicate of aps12808l/aps25616n driver.
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
#include "devices/am_devices_mspi_psram_aps25616n.c"
#else
#include "devices/am_devices_mspi_psram_aps12808l.c"
#endif