//*****************************************************************************
//
//! @file am_hal_bootrom_helper.c
//!
//! @brief BootROM Helper Function Table
//!
//! @addtogroup bootrom4 Bootrom Functionality
//! @ingroup bronco_hal
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
#ifdef BOOTROM_2M_MRAM
         ((int  (*)(uint32_t, uint32_t))                                    0x08000B8B),    // nv_mass_erase
         ((int  (*)(uint32_t, uint32_t, uint32_t))                          0x08000BA7),    // nv_page_erase
         ((int  (*)(uint32_t, uint32_t *, uint32_t *, uint32_t))            0x08000E0D),    // nv_program_main
         ((int  (*)(uint32_t, uint32_t *, uint32_t, uint32_t))              0x08000DF9),    // nv_program_info_area
         ((int  (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))      0x08000E25),    // nv_program_main2
         ((uint32_t (*)(uint32_t *))                                        0x08000799),    // br_util_read_word
         ((void (*)( uint32_t *, uint32_t))                                 0x0800079D),    // br_util_write_word
         ((int  (*)( uint32_t))                                             0x08000BC5),    // nv_info_erase
         ((int  (*)( uint32_t ))                                            0x08000BDD),    // nv_recovery
         ((void (*)(uint32_t ))                                             0x080000C9),    // br_util_delay_cycles
#else
         ((int  (*)(uint32_t, uint32_t))                                    0x08000B89),    // nv_mass_erase
         ((int  (*)(uint32_t, uint32_t, uint32_t))                          0x08000BA1),    // nv_page_erase
         ((int  (*)(uint32_t, uint32_t *, uint32_t *, uint32_t))            0x08000E01),    // nv_program_main
         ((int  (*)(uint32_t, uint32_t *, uint32_t, uint32_t))              0x08000DED),    // nv_program_info_area
         ((int  (*)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t))      0x08000E19),    // nv_program_main2
         ((uint32_t (*)(uint32_t *))                                        0x08000797),    // br_util_read_word
         ((void (*)( uint32_t *, uint32_t))                                 0x0800079B),    // br_util_write_word
         ((int  (*)( uint32_t))                                             0x08000BBF),    // nv_info_erase
         ((int  (*)( uint32_t ))                                            0x08000BD7),    // nv_recovery
         ((void (*)(uint32_t ))                                             0x080000C9),    // br_util_delay_cycles
#endif
};

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
