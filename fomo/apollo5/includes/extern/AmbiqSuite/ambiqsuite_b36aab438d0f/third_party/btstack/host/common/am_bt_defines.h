//*****************************************************************************
//
//! @file am_bt_defines.h
//!
//! @brief This file contains the common Bluetooth defines, enumerations and structures.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_BT_DEFINES_H
#define AM_BT_DEFINES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "am_types.h"

#define BD_ADDR_LEN         6

typedef uint8_t bd_addr[BD_ADDR_LEN];         /* Device address */


#ifdef __cplusplus
};
#endif
#endif /* AM_BT_DEFINES_H */
