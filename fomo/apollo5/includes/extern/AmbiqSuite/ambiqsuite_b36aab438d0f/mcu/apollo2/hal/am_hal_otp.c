//*****************************************************************************
//
//  am_hal_otp.c
//! @file
//!
//! @brief Functions for handling the OTP interface.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "am_mcu_apollo.h"
#include "am_hal_flash.h"

//*****************************************************************************
//
//! THIS FUNCTION IS DEPRECATED!
//! Use the respective HAL flash function instead.
//!
// @brief Check if debugger is currently locked out.
//
// @param None.
//
// Determine if the debugger is already locked out.
//
// @return non-zero if debugger is currently locked out.
//     Specifically:
//     0 = debugger is not locked out.
//     1 = debugger is locked out.
//
//*****************************************************************************
int
am_hal_otp_is_debugger_lockedout(void)
{
    return am_hal_flash_debugger_disable_check();
}

//*****************************************************************************
//
//! THIS FUNCTION IS DEPRECATED!
//! Use the respective HAL flash function instead.
//!
// @brief Lock out debugger access.
//
// @param None.
//
// This function locks out access by a debugger.
//
// @return 0 if lockout was successful or if lockout was already enabled.
//
//*****************************************************************************
int
am_hal_otp_debugger_lockout(void)
{
    return am_hal_flash_debugger_disable();
}

//*****************************************************************************
//
//! THIS FUNCTION IS DEPRECATED!
//! Use the respective HAL flash function instead.
//!
// @brief Lock out SRAM access.
//
// @param None.
//
// This function locks out access by a debugger to SRAM.
//
// @return 0 if lockout was successful or if lockout was already enabled.
//         Low byte=0xff, byte 1 contains current value of lockout.
//         Else, return value from HAL programming function.
//
//*****************************************************************************
int
am_hal_otp_sram_lockout(void)
{
    return am_hal_flash_wipe_sram_enable();
}

//*****************************************************************************
//
//! THIS FUNCTION IS DEPRECATED!
//! Use the respective HAL flash function instead.
//!
// @brief Set copy (read) protection.
//
// @param @ui32BegAddr The beginning address to be copy protected.
//        @ui32EndAddr The ending address to be copy protected.
//
// @note For Apollo, the ui32BegAddr parameter should be on a 16KB boundary, and
//       the ui32EndAddr parameter should be on a (16KB-1) boundary. Otherwise
//       both parameters will be truncated/expanded to do so.
//       For example, if ui32BegAddr=0x1000 and ui32EndAddr=0xC200, the actual
//       range that protected is: 0x0 - 0xFFFF.
//
// This function enables copy protection on a given flash address range.
//
// @return 0 if copy protection was successfully enabled.
//
//*****************************************************************************
int
am_hal_otp_set_copy_protection(uint32_t ui32BegAddr, uint32_t ui32EndAddr)
{
    return am_hal_flash_copy_protect_set((uint32_t*)ui32BegAddr,
                                         (uint32_t*)ui32EndAddr);
}

//*****************************************************************************
//
//! THIS FUNCTION IS DEPRECATED!
//! Use the respective HAL flash function instead.
//!
// @brief Set write protection.
//
// @param @ui32BegAddr The beginning address to be write protected.
//        @ui32EndAddr The ending address to be write protected.
//
// @note For Apollo, the ui32BegAddr parameter should be on a 16KB boundary, and
//       the ui32EndAddr parameter should be on a (16KB-1) boundary. Otherwise
//       both parameters will be truncated/expanded to do so.
//       For example, if ui32BegAddr=0x1000 and ui32EndAddr=0xC200, the actual
//       range that protected is: 0x0 - 0xFFFF.
//
// This function enables write protection on a given flash address range.
//
// @return 0 if write protection was successfully enabled.
//
//*****************************************************************************
int
am_hal_otp_set_write_protection(uint32_t ui32BegAddr, uint32_t ui32EndAddr)
{
    return am_hal_flash_write_protect_set((uint32_t*)ui32BegAddr,
                                          (uint32_t*)ui32EndAddr);
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
