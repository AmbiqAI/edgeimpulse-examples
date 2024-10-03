//*****************************************************************************
//
//  am_hal_otp.h
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
#ifndef AM_HAL_OTP_H
#define AM_HAL_OTP_H

//*****************************************************************************
//
// Define some OTP values and macros.
//
//*****************************************************************************
#define AM_HAL_OTP_SIG0             0x00
#define AM_HAL_OTP_SIG1             0x04
#define AM_HAL_OTP_SIG2             0x08
#define AM_HAL_OTP_SIG3             0x0C

#define AM_HAL_OTP_DBGR_O           0x10
#define AM_HAL_OTP_WRITPROT0_O      0x20
#define AM_HAL_OTP_WRITPROT1_O      0x24
#define AM_HAL_OTP_COPYPROT0_O      0x30
#define AM_HAL_OTP_COPYPROT1_O      0x34

#define AM_HAL_OTP_ADDR             0x50020000
#define AM_HAL_OTP_DBGRPROT_ADDR    (AM_HAL_OTP_ADDR + AM_HAL_OTP_DBGR_O)
#define AM_HAL_OTP_WRITPROT_ADDR    (AM_HAL_OTP_ADDR + AM_HAL_OTP_WRITPROT0_O)
#define AM_HAL_OTP_COPYPROT_ADDR    (AM_HAL_OTP_ADDR + AM_HAL_OTP_COPYPROT0_O)

#define AM_HAL_OTP_CHUNKSIZE        (16*1024)

//
// Debugger port lockout macros.
//
#define AM_OTP_DBGR_LOCKOUT_S       (0)
#define AM_OTP_DBGR_LOCKOUT_M       (0x1 << AM_OTP_DBGR_LOCKOUT_S)
#define AM_OTP_STRM_LOCKOUT_S       (1)
#define AM_OTP_STRM_LOCKOUT_M       (0x1 << AM_OTP_STRM_LOCKOUT_S)
#define AM_OTP_SRAM_LOCKOUT_S       (2)
#define AM_OTP_SRAM_LOCKOUT_M       (0x1 << AM_OTP_SRAM_LOCKOUT_S)

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Function prototypes
//
//*****************************************************************************
extern int am_hal_otp_is_debugger_lockedout(void);
extern int am_hal_otp_debugger_lockout(void);
extern int am_hal_otp_sram_lockout(void);
extern int am_hal_otp_set_copy_protection(uint32_t ui32BegAddr, uint32_t ui32EndAddr);
extern int am_hal_otp_set_write_protection(uint32_t ui32BegAddr, uint32_t ui32EndAddr);

#ifdef __cplusplus
}
#endif

#endif // AM_HAL_OTP_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

