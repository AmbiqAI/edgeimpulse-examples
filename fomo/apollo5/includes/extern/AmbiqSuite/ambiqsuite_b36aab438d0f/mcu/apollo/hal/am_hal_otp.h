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
#define AM_HAL_OTP_DBGR_O           0
#define AM_HAL_OTP_WRITPROT_O       4
#define AM_HAL_OTP_COPYPROT_O       8

#define AM_HAL_OTP_ADDR             0x50020400
#define AM_HAL_OTP_DBGRPROT_ADDR    (AM_HAL_OTP_ADDR + AM_HAL_OTP_DBGR_O)
#define AM_HAL_OTP_WRITPROT_ADDR    (AM_HAL_OTP_ADDR + AM_HAL_OTP_WRITPROT_O)
#define AM_HAL_OTP_COPYPROT_ADDR    (AM_HAL_OTP_ADDR + AM_HAL_OTP_COPYPROT_O)

#define AM_HAL_OTP_CHUNKSIZE        (16*1024)

//
// AM_HAL_OTP_CHUNK2ADDR: Convert a chunk number to an address
// AM_HAL_OTP_CHUNK2INST: Convert a chunk number to an instance number
//
#define AM_HAL_OTP_CHUNK2ADDR(n)    (AM_HAL_FLASH_ADDR + (n << 14))
#define AM_HAL_OTP_CHUNK2INST(n)    ((n >> 4) & 1)

//
// Debugger port lockout macros.
//
#define AM_OTP_DBGR_LOCKOUT_S       (0)
#define AM_OTP_DBGR_LOCKOUT_M       (0xf << AM_OTP_DBGR_LOCKOUT_S)
#define AM_OTP_STRM_LOCKOUT_S       (4)
#define AM_OTP_STRM_LOCKOUT_M       (0xf << AM_OTP_STRM_LOCKOUT_S)
#define AM_OTP_SRAM_LOCKOUT_S       (8)
#define AM_OTP_SRAM_LOCKOUT_M       (0xf << AM_OTP_SRAM_LOCKOUT_S)

//
// Define a macro which will compute the appropriate bitmask for setting
//  copy or write protection for a given range of addresses.
// Important note: The begaddr and endaddr parameters will be truncated/
//  expanded to include the entire 16KB chunk of memory in which it resides.
//
#define AM_HAL_OTP_PROT_M(begaddr, endaddr)                                                                                                 \
        ~(  (AM_HAL_OTP_PROT_W((begaddr), (endaddr)) < 32)                                                                              ?   \
                (((1 << AM_HAL_OTP_PROT_W((begaddr), (endaddr)))-1) << (((begaddr) & ~(AM_HAL_OTP_CHUNKSIZE-1))/AM_HAL_OTP_CHUNKSIZE) ) :   \
                0xffffffff )

//
// Macro to determine the width in chunks.
// Note that the address parameters are truncated/expanded to 16KB chunks.
//
#define AM_HAL_OTP_PROT_W(begaddr, endaddr)                                  \
        ( ( ( ((endaddr) | (AM_HAL_OTP_CHUNKSIZE-1)) - ((begaddr) & ~(AM_HAL_OTP_CHUNKSIZE-1)) ) / AM_HAL_OTP_CHUNKSIZE) + 1)

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

