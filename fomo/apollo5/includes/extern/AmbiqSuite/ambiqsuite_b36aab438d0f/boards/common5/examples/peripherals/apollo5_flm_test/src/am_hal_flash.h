//*****************************************************************************
//
//! @file am_hal_flash.h
//!
//! @brief Functions for performing Flash operations.
//!
//! @addtogroup hal Hardware Abstraction Layer (HAL)
//! @addtogroup flash Flash
//! @ingroup hal
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
#ifndef AM_HAL_FLASH_H
#define AM_HAL_FLASH_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

//*****************************************************************************
//
// Flash Program keys.
//
//*****************************************************************************
//#define AM_HAL_FLASH_PROGRAM_KEY            0x12344321
//#define AM_HAL_FLASH_OTP_KEY                0x87655678

#define NV_WIPE     0
#define NV_PROGRAM  1

//
// Structure of function pointers to helper functions for invoking various
// flash operations.
//
typedef struct am_hal_bootrom_helper_struct
{
#if (AM_DEVICE == AM_DEV_APOLLO4) || (AM_DEVICE == AM_DEV_APOLLO5)
    //
    // From am_hal_bootrom_helper.h
    //
    int  (*nv_mass_erase)(uint32_t, uint32_t);
    int  (*nv_page_erase)(uint32_t, uint32_t, uint32_t);
    int  (*nv_program_main)(uint32_t, uint32_t *, uint32_t *, uint32_t);
    int  (*nv_program_info_area)(uint32_t, uint32_t *, uint32_t, uint32_t);
    void (*nv_program_main_from_sram)(void);
    void (*nv_program_info_area_from_sram)(void);
    void (*nv_erase_main_pages_from_sram)(void);
    void (*nv_mass_erase_from_sram)(void);
    int  (*nv_program_main2)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
    uint32_t (*bootrom_util_read_word)( uint32_t *);
    void (*bootrom_util_write_word)( uint32_t *, uint32_t);
    int  (*nv_info_erase)( uint32_t);
    void (*nv_info_erase_from_sram)(void);
    int  (*nv_recovery)( uint32_t value);
    void (*bootrom_delay_cycles)(uint32_t ui32Cycles);
    void (*nv_recovery_from_sram)(void);
#else
    int  (*am_hal_flash_mass_erase)(uint32_t, uint32_t);
    int  (*am_hal_flash_page_erase)(uint32_t, uint32_t, uint32_t);
    int  (*am_hal_flash_program_main)(uint32_t, uint32_t *, uint32_t *, uint32_t);
    int  (*am_hal_flash_program_otp)(uint32_t,  uint32_t, uint32_t *, uint32_t, uint32_t);
    void (*am_hal_flash_program_main_sram)(void);
    void (*am_hal_flash_program_otp_sram)(void);
    void (*am_hal_flash_erase_main_pages_sram)(void);
    void (*am_hal_flash_mass_erase_sram)(void);
#endif
} g_am_hal_flash_t;

extern g_am_hal_flash_t g_am_hal_flash;

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_FLASH_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
