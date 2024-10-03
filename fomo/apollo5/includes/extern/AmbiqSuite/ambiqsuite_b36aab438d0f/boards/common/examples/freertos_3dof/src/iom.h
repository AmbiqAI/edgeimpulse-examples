//*****************************************************************************
//
//! @file iom.c
//!
//! @brief Functions to handle IOM operation.
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef IOM_H
#define IOM_H

#include <stdint.h>
#include <stdbool.h>

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************
extern void iom1_setup(void);
extern void iom1_shutdown(void);
extern void iom1_restart(void);

#endif // IOM_H
