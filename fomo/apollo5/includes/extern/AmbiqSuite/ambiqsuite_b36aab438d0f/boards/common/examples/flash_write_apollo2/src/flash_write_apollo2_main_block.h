//*****************************************************************************
//
//! @file flash_write_apollo2_main_block.h
//!
//! @brief Flash write examples for main block specifically for Apollo 2.
//!
//! This example shows how to modify the internal Flash using HAL flash helper
//! functions.
//!
//! This example works on instance 1 of the Flash, i.e. the portion of the
//! Flash above 512KB. It is intended to execute from instance 0.
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
#ifndef FLASH_WRITEA2_MAIN_BLOCK_H
#define FLASH_WRITEA2_MAIN_BLOCK_H

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
// External variable definitions
//
//*****************************************************************************

//*****************************************************************************
//
// External function definitions
//
//*****************************************************************************
extern int main_block_erase(void);
extern int main_block_pattern_write(void);
extern int main_block_page_erase(void);

#ifdef __cplusplus
}
#endif

#endif // FLASH_WRITEA2_MAIN_BLOCK_H


