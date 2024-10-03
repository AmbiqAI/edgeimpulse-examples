//*****************************************************************************
//
//  am_hal_bootrom_helper.c
//! @file
//!
//! @brief BootROM Helper Function Table
//!
//! @addtogroup bootrom4 MRAM
//! @ingroup apollo4hal
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
// Look-up table
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
         ((int  (*)(uint32_t, uint32_t))                                    0x0800004D),    // nv_mass_erase
         ((int  (*)(uint32_t, uint32_t, uint32_t))                          0x08000051),    // nv_page_erase
         ((int  (*)(uint32_t, uint32_t *, uint32_t *, uint32_t))            0x08000055),    // nv_program_main
         ((int  (*)(uint32_t, uint32_t *, uint32_t, uint32_t))              0x08000059),    // nv_program_info_area
         ((int  (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))      0x0800006D),    // nv_program_main2
         ((uint32_t (*)(uint32_t *))                                        0x08000075),    // br_util_read_word
         ((void (*)( uint32_t *, uint32_t))                                 0x08000079),    // br_util_write_word
         ((int  (*)( uint32_t))                                             0x08000081),    // nv_info_erase
         ((int  (*)( uint32_t ))                                            0x08000099),    // nv_recovery
         ((void (*)(uint32_t ))                                             0x0800009D),    // br_util_delay_cycles
};

