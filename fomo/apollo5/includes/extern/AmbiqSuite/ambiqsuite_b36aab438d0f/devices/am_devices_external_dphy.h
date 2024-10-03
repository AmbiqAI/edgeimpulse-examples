//*****************************************************************************
//
//! @file
//!
//! @brief
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef AM_DPHY_SPI_INIT_H
#define AM_DPHY_SPI_INIT_H

#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Macro definitions.
//
//*****************************************************************************
#define IOM_0    0
#define CMD_NUM  45

#define CLN_CNT_HS_TRAIL_ADDR 0x07
#define CLN_CNT_HS_ZERO_ADDR  0x0B
#define CLN_CNT_HS_EXIT_ADDR  0x08
#define CLN_CNT_PREP_ADDR     0x0A
#define CLN_CNT_LPX_ADDR      0x09
#define DLN_CNT_HS_PREP_ADDR  0x00
#define DLN_CNT_HS_ZERO_ADDR  0x01
#define DLN_CNT_HS_TRAIL_ADDR 0x02
#define DLN_CNT_HS_EXIT_ADDR  0x03
#define DLN_CNT_LPX_ADDR      0x06
#define DLN_RX_POST_CNT_ADDR  0x0F
#define DLN_RX_CNT_ADDR       0x04
#define DLN_SYNC_CNT_ADDR     0x05

#if 1 // 500M
#define CLN_CNT_HS_TRAIL_DATA 0x0A
#define CLN_CNT_HS_ZERO_DATA  0x23
#define CLN_CNT_HS_EXIT_DATA  0x0E
#define CLN_CNT_PREP_DATA     0x08
#define CLN_CNT_LPX_DATA      0x08
#define DLN_CNT_HS_PREP_DATA  0x0A
#define DLN_CNT_HS_ZERO_DATA  0x0F
#define DLN_CNT_HS_TRAIL_DATA 0x0E
#define DLN_CNT_HS_EXIT_DATA  0x0E
#define DLN_CNT_LPX_DATA      0x08
#define DLN_RX_POST_CNT_DATA  0x00
#define DLN_RX_CNT_DATA       0x0E
#define DLN_SYNC_CNT_DATA     0xFF
#else
//156M
#define CLN_CNT_HS_TRAIL_DATA 0x03
#define CLN_CNT_HS_ZERO_DATA  0x0D
#define CLN_CNT_HS_EXIT_DATA  0x05
#define CLN_CNT_PREP_DATA     0x02
#define CLN_CNT_LPX_DATA      0x03
#define DLN_CNT_HS_PREP_DATA  0x03
#define DLN_CNT_HS_ZERO_DATA  0x06
#define DLN_CNT_HS_TRAIL_DATA 0x05
#define DLN_CNT_HS_EXIT_DATA  0x05
#define DLN_CNT_LPX_DATA      0x03
#define DLN_RX_POST_CNT_DATA  0x00
#define DLN_RX_CNT_DATA       0x04
#define DLN_SYNC_CNT_DATA     0xFF
#endif
//*****************************************************************************
//
// Type definitions.
//
//*****************************************************************************

typedef enum
{
    DPHY_X2 = 2,    //0x01,0x38
    DPHY_X3,        //0x41,0x38
    DPHY_X4,        //0x02,0x38
    DPHY_X5,        //0x42,0x38
    DPHY_X6,        //0x03,0x38
    DPHY_X7,        //0x43,0x38
    DPHY_X8,        //0x04,0x38
    DPHY_X9,        //0x44,0x38
    DPHY_X10,       //0x05,0x38
    DPHY_X11,       //0x45,0x34
    DPHY_X12,       //0x06,0x34
    DPHY_X13,       //0x46,0x34
    DPHY_X14,       //0x07,0x34
    DPHY_X15,       //0x47,0x34
    DPHY_X16,       //0x08,0x34
    DPHY_X17,       //0x48,0x34
    DPHY_X18,       //0x09,0x34
    DPHY_X19,       //0x49,0x34
    DPHY_X20        //0x0A,0x34
}dphy_trim_t;

extern uint32_t dphy_init_with_para(dphy_trim_t trim);
extern uint32_t dphy_init(void);
extern uint32_t dphy_write_reg(uint32_t ui32Addr, uint32_t ui32OutData);

#ifdef __cplusplus
}
#endif

#endif
