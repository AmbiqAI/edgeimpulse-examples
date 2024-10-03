//*****************************************************************************
//
//  am_reg_iomstr_cmd.h
//! @file
//!
//! @brief Register macros for the IOMSTR module
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#ifndef AM_REG_IOMSTR_CMD_H
#define AM_REG_IOMSTR_CMD_H

#if AM_PART_APOLLO2
//*****************************************************************************
//
// IOMSTR_CMD - Command Register
//
//*****************************************************************************
#define AM_REG_IOMSTR_CMD_CMD_POS_LENGTH             0x00000000
#define AM_REG_IOMSTR_CMD_CMD_POS_OFFSET             0x00000008
#define AM_REG_IOMSTR_CMD_CMD_POS_ADDRESS            0x00000010
#define AM_REG_IOMSTR_CMD_CMD_POS_CHNL               0x00000010
#define AM_REG_IOMSTR_CMD_CMD_POS_UPLNGTH            0x00000017
#define AM_REG_IOMSTR_CMD_CMD_POS_10BIT              0x0000001A
#define AM_REG_IOMSTR_CMD_CMD_POS_LSB                0x0000001B
#define AM_REG_IOMSTR_CMD_CMD_POS_CONT               0x0000001C
#define AM_REG_IOMSTR_CMD_CMD_POS_OPER               0x0000001D
#define AM_REG_IOMSTR_CMD_CMD_MSK_LENGTH             0x000000FF
#define AM_REG_IOMSTR_CMD_CMD_MSK_OFFSET             0x0000FF00
#define AM_REG_IOMSTR_CMD_CMD_MSK_ADDRESS            0x00FF0000
#define AM_REG_IOMSTR_CMD_CMD_MSK_CHNL               0x00070000
#define AM_REG_IOMSTR_CMD_CMD_MSK_UPLNGTH            0x07800000
#define AM_REG_IOMSTR_CMD_CMD_MSK_10BIT              0x04000000
#define AM_REG_IOMSTR_CMD_CMD_MSK_LSB                0x08000000
#define AM_REG_IOMSTR_CMD_CMD_MSK_CONT               0x10000000
#define AM_REG_IOMSTR_CMD_CMD_MSK_OPER               0xE0000000
#endif // AM_PART_APOLLO2

#endif // AM_REG_IOMSTR_CMD_H
