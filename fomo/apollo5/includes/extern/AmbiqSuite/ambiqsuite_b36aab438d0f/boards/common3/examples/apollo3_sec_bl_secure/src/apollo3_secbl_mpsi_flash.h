//*****************************************************************************
//!
//! @file apollo3_secbl_mspi_flash.h
//!
//! @brief the header file of atxp032/128 flash interface
//!
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef APOLLO3_SECBL_MSPI_FLASH_H
#define APOLLO3_SECBL_MSPI_FLASH_H

#include "apollo3_secbl_flash.h"

//
// ATXP032 MSPI external flash start address
//
#define ATXP032_FLASH_BASE_ADDR 0x04000000

//
// ATXP032 MSPI external flash size
//
#define ATXP032_FLASH_SIZE      (32 << 20)

extern am_ssbl_flash_t mspi_atxp032_flash_info;

#endif // APOLLO3_SECBL_MSPI_FLASH_H
