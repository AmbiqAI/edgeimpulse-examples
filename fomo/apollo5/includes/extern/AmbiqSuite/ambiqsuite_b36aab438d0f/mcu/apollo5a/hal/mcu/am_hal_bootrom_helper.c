//*****************************************************************************
//
//! @file am_hal_bootrom_helper.c
//!
//! @brief BootROM Helper Function Table
//!
//! @addtogroup bootrom4 Bootrom Functionality
//! @ingroup apollo5a_hal
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
#include "am_mcu_apollo.h"
#include "am_hal_bootrom_helper.h"

//
//! Look-up table
//
// #### INTERNAL BEGIN ####
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%                                                               %%%%%%%%%
//%%%%%%%%  WARNING ADDRESS CONSTANTS ARE MANUALLY EXTRACTED FROM .txt   %%%%%%%%%
//%%%%%%%%  in the boot loader compilation directory in the chip area.   %%%%%%%%%
//%%%%%%%%  If you change the boot loader you have to change this file   %%%%%%%%%
//%%%%%%%%  by hand.                                                     %%%%%%%%%
//%%%%%%%%                                                               %%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
// #### INTERNAL END ####
const am_hal_bootrom_helper_t g_am_hal_bootrom_helper =
{
         ((int  (*)(uint32_t, uint32_t))                                            (0x0200ff00 + 1)),  // nv_mass_erase
         ((int  (*)(uint32_t, uint32_t, uint32_t))                                  (0x0200ff04 + 1)),  // nv_page_erase
         ((int  (*)(uint32_t, uint32_t *, uint32_t *, uint32_t))                    (0x0200ff08 + 1)),  // nv_program_main
         ((int  (*)(uint32_t, uint32_t *, uint32_t, uint32_t))                      (0x0200ff0c + 1)),  // nv_program_info_area
         ((int  (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))              (0x0200ff20 + 1)),  // nv_program_main2
         ((uint32_t (*)(uint32_t *))                                                (0x0200ff28 + 1)),  // br_util_read_word
         ((void (*)(uint32_t *, uint32_t))                                          (0x0200ff2c + 1)),  // br_util_write_word
         ((int  (*)(uint32_t))                                                      (0x0200ff34 + 1)),  // nv_info_erase
         ((bool (*) (void *, uint32_t, uint32_t))                                   (0x0200ff40 + 1)),  // valid_address_range
         ((void (*)(uint32_t ))                                                     (0x0200ff50 + 1)),  // br_util_delay_cycles
         ((int  (*)(uint32_t, uint32_t*, uint32_t, uint32_t))                       (0x0200ff54 + 1)),  // otp_program_info_area
         ((uint32_t (*)(void))                                                      (0x0200ff5c + 1)),  // br_util_rom_version
};

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
