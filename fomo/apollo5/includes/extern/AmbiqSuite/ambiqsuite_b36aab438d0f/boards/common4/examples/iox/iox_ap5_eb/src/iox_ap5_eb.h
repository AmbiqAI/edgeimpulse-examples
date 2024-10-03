//*****************************************************************************
//
//! @file iox_ap5_eb.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef IOX_AP5_EB_H
#define IOX_AP5_EB_H

//#define DEBUG_ON_AP4P_EVB // uncomment to debug on Apollo4P EVB

#define IOX_FW_REV  6 // increment for each version released

#define GPIO_FT_SCL                 0 // FTDI I2C SCL
#define GPIO_FT_SDA                 1 // FTDI I2C SDA
#define GPIO_LED4                   2 // Mode LED
// GPIO3 is NC
#define GPIO_CFG_SW                 4 // CFG Button
// GPIO5, GPIO6 are NC
#define GPIO_IOM4_PC_FRAM_SEL       7
#define GPIO_IOM5_PC_FRAM_OEn       8
// GPIO9, GPIO10, GPIO11 are NC
#define GPIO_MSPI1_OEn_SEL1         12
#define GPIO_MSPI1_QUAD_SEL         13
#define GPIO_MSPI3_PC_SEL           14
// GPIO15, GPIO16, GPIO17 are NC
#define GPIO_MSPI1_PC_SEL           18
#define GPIO_MSPI1_OEn_SEL0         19
// GPIO20/21 are SWDCK/SWDIO
#define GPIO_MSPI1_LS_EN            22
#define GPIO_SDIO1_MUX_SEL1         23
#define GPIO_SDIO1_RSTn             24 // SDIO1 RSTn
// GPIO25, GPIO26, GPIO27 are NC
#define GPIO_IOX_SWO                28 // ITM Output
// GPIO29, GPIO30 are NC
#define GPIO_LED3                   31 // Mode LED
#define GPIO_LED2                   32 // Mode LED
#define GPIO_LED1                   33 // Mode LED
#define GPIO_SWD_OEn_SEL            34
#define GPIO_SWD_PC_SEL             35
#define GPIO_TRACE_OEn_SEL          36
#define GPIO_MSPI2_PC_SEL           37
#define GPIO_IOM1_PC_DISP_OEn       38
#define GPIO_MSPI2_OEn_SEL1         39
#define GPIO_IOM3_PSRAM_OEn         40
#define GPIO_MSPI2_QUAD_SEL         41
#define GPIO_IOM1_PC_DISP_SEL       42
#define GPIO_SDIO0_MUX_SEL1         43
#define GPIO_IOM1_FRAM_PSRAM_OEn    44
#define GPIO_IOM4_PC_FRAM_OEn       45
#define GPIO_IOM4_PSRAM_OEn         46
#define GPIO_TRACE_PC_SEL           47
// GPIO48 is NC
#define GPIO_HUB_I2C_EN             49 // I2C Enable
#define GPIO_IOM7_PSRAM_OEn         50
#define GPIO_SDIO0_RSTn             51 // SDIO0 RSTn
#define GPIO_IOM6_PC_FRAM_OEn       52
#define GPIO_IOM6_PC_FRAM_SEL       53
#define GPIO_MSPI2_LS_EN            54 // MSPI2 Load Switch
#define GPIO_MSPI0_OEn_SEL          55
// GPIO56 is NC
#define GPIO_MSPI2_OEn_SEL0         57
// GPIO58-62 are NC
#define GPIO_IOM3_PC_FRAM_OEn       63
// GPIO64 is NC
#define GPIO_SDIO0_RST_MUX_SEL0     65
#define GPIO_SDIO0_RST_MUX_SEL1     66
#define GPIO_IOM7_PC_FRAM_OEn       67
#define GPIO_IOM7_PC_FRAM_SEL       68
#define GPIO_IOM6_PSRAM_OEn         69
// GPIO70 is NC
#define GPIO_MSPI3_OEn_SEL          71
#define GPIO_MSPI0_PC_SEL           72
// GPIO73 is NC
#define GPIO_IOM0_FRAM_PSRAM_SEL    74
#define GPIO_IOM0_PC_LB_OEn         75
#define GPIO_IOM0_FRAM_PSRAM_OEn    76
#define GPIO_SDIO0_MUX_SEL0         77
#define GPIO_IOM0_PC_LB_SEL         78
#define GPIO_IOX_LED                79 // Blinks on power up to indicate FW version
#define GPIO_IOM2_PC_DISP_SEL       80
#define GPIO_IOS_LB_PC_SEL          81
#define GPIO_IOS_LB_PC_OEn          82
#define GPIO_IOM1_FRAM_PSRAM_SEL    83
#define GPIO_IOM2_FRAM_PSRAM_OEn    84
#define GPIO_IOM2_FRAM_PSRAM_SEL    85
#define GPIO_IOM3_PC_FRAM_SEL       86
// GPIO87 is NC
#define GPIO_IOM2_PC_DISP_OEn       88
// GPIO89 is NC
#define GPIO_IOM5_PC_FRAM_SEL       90
// GPIO91 is NC
#define GPIO_IOM5_PSRAM_OEn         92
// GPIO93-99 are NC
#define GPIO_SDIO1_RST_MUX_SEL0     100
#define GPIO_SDIO1_RST_MUX_SEL1     101
#define GPIO_SDIO1_MUX_SEL0         102
#define GPIO_UART_SEL1              103
#define GPIO_UART_SEL0              104

// TODO: Verify these based on the
// board placement
#define GPIO_LED_MODE_B3    GPIO_LED4
#define GPIO_LED_MODE_B2    GPIO_LED3
#define GPIO_LED_MODE_B1    GPIO_LED2
#define GPIO_LED_MODE_B0    GPIO_LED1

// If debugging on Apollo4P EVB, redefine some pins
#ifdef DEBUG_ON_AP4P_EVB
// LEDs (Apollo4P EVB only has 3)
#undef GPIO_LED_MODE_B2
#undef GPIO_LED_MODE_B1
#undef GPIO_LED_MODE_B0
#define GPIO_LED_MODE_B2    AM_BSP_GPIO_LED2
#define GPIO_LED_MODE_B1    AM_BSP_GPIO_LED1
#define GPIO_LED_MODE_B0    AM_BSP_GPIO_LED0
// Button
#undef GPIO_CFG_SW
#define GPIO_CFG_SW                 19 // BTN2 on AP4P EVB
#endif

//
// Enums for pin group configurations
//

// SWD
typedef enum
{
    SWD_AP5,
    SWD_PC
} swd_options_e;

// TRACE
typedef enum
{
    TRACE_AP5,
    TRACE_PC
} trace_options_e;

// MSPI03
typedef enum
{
    MSPI03_HEX_PSRAM,
    MSPI03_PC
} mspi03_options_e;

// MSPI12
typedef enum
{
    MSPI12_OCTAL_FLASH,
    MSPI12_QUAD_PSRAM,
    MSPI12_QUAD_NAND,
    MSPI12_PC
} mspi12_options_e;

// IOM
typedef enum
{
    IOM_IOS_LOOPBACK, // IOM0 only
    IOM_SERIAL_PSRAM,
    IOM_FRAM,
    IOM_PC
} iom_options_e;

// UART
typedef enum
{
    UART_HDR_FTDI, // FTDI comport + debug headers
    UART_VCOM, // JLink OB Virtual Comport
    UART_PC
} uart_options_e;

// SDIO
typedef enum
{
    SDIO_eMMC,
    SDIO_EXT,
    SDIO_PC
} sdio_options_e;

// SDIO RST
typedef enum
{
    SDIO_RST_OPEN,
    SDIO_RST_eMMC,
    SDIO_RST_EXT,
    SDIO_RST_PC
} sdio_rst_options_e;

#endif
