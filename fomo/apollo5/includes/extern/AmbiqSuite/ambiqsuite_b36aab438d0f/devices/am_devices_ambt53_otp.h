//*****************************************************************************
//
//! @file am_devices_ambt53_otp.h
//!
//! @brief The implementation of Apollo interface to AMBT53 OTP.
//!
//! @addtogroup ambt53 AMBT53 Device Driver
//! @ingroup devices
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

#ifndef AM_DEVICES_AMBT53_OTP_H
#define AM_DEVICES_AMBT53_OTP_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "am_util.h"
#include "am_mcu_apollo.h"

// ********************************EFUSE********************************

//! @defgroup EFUSE EFUSE
//! @{

#define REG_EFUSE_BASE_ADDR                             0x03000400

#define EFUSE_GCR_ADDR                                  (REG_EFUSE_BASE_ADDR + 0x0)
#define EFUSE_GCR_RW_MASK                               ((uint32_t)0x00000007)
#define EFUSE_GCR_RESET                                 0x00000000

#define EFUSE_READ_CFG_ADDR                             (REG_EFUSE_BASE_ADDR + 0x4)
#define EFUSE_READ_CFG_RW_MASK                          ((uint32_t)0x003FFFFF)
#define EFUSE_READ_CFG_RESET                            0x00200000

#define EFUSE_PGM_DATA_ADDR                             (REG_EFUSE_BASE_ADDR + 0x8)
#define EFUSE_PGM_DATA_MASK                             ((uint32_t)0xFFFFFFFF)
#define EFUSE_PGM_DATA_RESET                            0x00000000

#define EFUSE_RD_DATA_ADDR                              (REG_EFUSE_BASE_ADDR + 0xC)
#define EFUSE_RD_DATA_MASK                              ((uint32_t)0xFFFFFFFF)
#define EFUSE_RD_DATA_RESET                             0x00000000

#define EFUSE_IER_ADDR                                  (REG_EFUSE_BASE_ADDR + 0x10)
#define EFUSE_IER_RW_MASK                               ((uint32_t)0x00000003)
#define EFUSE_IER_RESET                                 0x00000000

#define EFUSE_ISR_ADDR                                  (REG_EFUSE_BASE_ADDR + 0x14)
#define EFUSE_ISR_MASK                                  ((uint32_t)0x0000000F)
#define EFUSE_ISR_RESET                                 0x00000000

#define EFUSE_ICR_ADDR                                  (REG_EFUSE_BASE_ADDR + 0x18)
#define EFUSE_ICR_MASK                                  ((uint32_t)0x00000007)
#define EFUSE_ICR_RESET                                 0x00000000

#define EFUSE_MR_CFG_ADDR                               (REG_EFUSE_BASE_ADDR + 0x1C)
#define EFUSE_MR_CFG_RW_MASK                            ((uint32_t)0x00000003)
#define EFUSE_MR_CFG_RESET                              0x00000000

#define EFUSE_PGM_PRE_ADDR                              (REG_EFUSE_BASE_ADDR + 0x20)
#define EFUSE_PGM_PRE_RW_MASK                           ((uint32_t)0x0000FFFF)
#define EFUSE_PGM_PRE_RESET                             0x0000009B

#define EFUSE_PGM_STB_H_ADDR                            (REG_EFUSE_BASE_ADDR + 0x24)
#define EFUSE_PGM_STB_H_RW_MASK                         ((uint32_t)0x0000FFFF)
#define EFUSE_PGM_STB_H_RESET                           0x000003E8

#define EFUSE_PGM_STB_L_ADDR                            (REG_EFUSE_BASE_ADDR + 0x28)
#define EFUSE_PGM_STB_L_RW_MASK                         ((uint32_t)0x0000FFFF)
#define EFUSE_PGM_STB_L_RESET                           0x00000012

#define EFUSE_PGM_STB_TO_ADDR                           (REG_EFUSE_BASE_ADDR + 0x2C)
#define EFUSE_PGM_STB_TO_RW_MASK                        ((uint32_t)0x0000FFFF)
#define EFUSE_PGM_STB_TO_RESET                          0x00000002

#define EFUSE_PGM_END_ADDR                              (REG_EFUSE_BASE_ADDR + 0x30)
#define EFUSE_PGM_END_RW_MASK                           ((uint32_t)0x0000FFFF)
#define EFUSE_PGM_END_RESET                             0x00000024

#define EFUSE_RD_PRE_ADDR                               (REG_EFUSE_BASE_ADDR + 0x34)
#define EFUSE_RD_PRE_RW_MASK                            ((uint32_t)0x0000FFFF)
#define EFUSE_RD_PRE_RESET                              0x00000005

#define EFUSE_RD_STB_H_ADDR                             (REG_EFUSE_BASE_ADDR + 0x38)
#define EFUSE_RD_STB_H_RW_MASK                          ((uint32_t)0x0000FFFF)
#define EFUSE_RD_STB_H_RESET                            0x00000003

#define EFUSE_RD_STB_L_ADDR                             (REG_EFUSE_BASE_ADDR + 0x3C)
#define EFUSE_RD_STB_L_RW_MASK                          ((uint32_t)0x0000FFFF)
#define EFUSE_RD_STB_L_RESET                            0x00000004

#define EFUSE_RD_STB_TO_ADDR                            (REG_EFUSE_BASE_ADDR + 0x40)
#define EFUSE_RD_STB_TO_RW_MASK                         ((uint32_t)0x0000FFFF)
#define EFUSE_RD_STB_TO_RESET                           0x00000001

#define EFUSE_RD_END_ADDR                               (REG_EFUSE_BASE_ADDR + 0x44)
#define EFUSE_RD_END_RW_MASK                            ((uint32_t)0x0000FFFF)
#define EFUSE_RD_END_RESET                              0x00000001
//! @} group

//*****************************************************************************
//
// External function definitions.
//
//*****************************************************************************

#ifdef __cplusplus
}
#endif

#endif // AM_DEVICES_AMBT53_OTP_H

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************

