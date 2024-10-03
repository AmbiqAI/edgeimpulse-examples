//*****************************************************************************
//
//! @file flash_write_apollo2_info_block.h
//!
//! @brief Flash write examples for info block specifically for Apollo 2.
//!
//! This example shows how to modify the internal Flash using HAL flash helper
//! functions.
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
#ifndef FLASH_WRITEA2_INFO_BLOCK_H
#define FLASH_WRITEA2_INFO_BLOCK_H

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
extern int info_block_erase(void);
extern int info_block_pattern_write(void);
extern int info_block_write_board_name(char *pChar);
extern int info_block_write_ble_address(uint64_t ui64BLEAddress);
extern int info_block_disable_erase(void);
extern int info_block_disable_program(uint32_t ui32Mask, uint32_t ui32ExpectMask);
extern int info_block_enable_flash_wipe(void);

#ifdef __cplusplus
}
#endif

#endif // FLASH_WRITEA2_INFO_BLOCK_H


