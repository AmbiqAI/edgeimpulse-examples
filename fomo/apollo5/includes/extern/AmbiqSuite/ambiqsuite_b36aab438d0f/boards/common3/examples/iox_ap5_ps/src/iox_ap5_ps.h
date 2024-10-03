//*****************************************************************************
//
//! @file iox_ap5_ps.h
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#ifndef IOX_AP5_PS_H
#define IOX_AP5_PS_H

//#define DEBUG_ON_AP3_EVB // uncomment to debug on Apollo3 EVB

#define IOX_FW_REV  4 // increment for each version released

#define GPIO_PDM0_UART_MUX_SEL0     0
#define GPIO_PDM0_UART_MUX_SEL1     1
#define GPIO_PDM0_UART_MUX_SEL2     2
#define GPIO_PDM1_UART_MUX_SEL0     3
#define GPIO_PDM1_UART_MUX_SEL1     4
#define GPIO_AP3_SCL                5
#define GPIO_AP3_SDA                6
// GPIO7 is NC
#define GPIO_DISP_MUX_SEL           8
// GPIO9, 10, 11 are NC
#define GPIO_UART_SRC_SEL           12
#define GPIO_MSPIX8_MUX_SEL         13
#define GPIO_MSPIX16_MUX_SEL        14
#define GPIO_PSRAMX16_MUX_SEL       15
#define GPIO_IOM0_MUX_SEL0          16
#define GPIO_IOM0_MUX_SEL1          17
#define GPIO_IOM1_MUX_SEL0          18
#define GPIO_IOM1_MUX_SEL1          19
// GPIO20/21 are SWDCK/SWDIO
#define GPIO_SDIO0_MUX_SEL0         22
#define GPIO_SDIO0_MUX_SEL1         23
#define GPIO_SDIO1_MUX_SEL0         24
// GPIO25 is NC
#define GPIO_SDIO0_RST_MUX_SEL0     26
#define GPIO_SDIO0_RST_MUX_SEL1     27
#define GPIO_SDIO1_RST_MUX_SEL0     28
#define GPIO_SDIO1_RST_MUX_SEL1     29
// GPIO30, 31 are NC
#define GPIO_IOM_MUX_SEL0           32
#define GPIO_IOM_MUX_SEL1           33
#define GPIO_IOM_MUX_SEL2           34
#define GPIO_IOM_MUX_SEL3           35
#define GPIO_IOM_MUX_SEL4           36
#define GPIO_IOM_MUX_SEL5           37
#define GPIO_IOM_MUX_SEL6           38
#define GPIO_IOM_MUX_SEL7           39
#define GPIO_SDIO_RSTn_I2S_MUX_SEL  40

//
// Button
//
#define GPIO_CFG_SW                 41

// GPIO42, 43 are NC

//
// nRST - this pin will be blue-wired to Apollo3 nRST
//
#define GPIO_AP3_nRST               44

//
// SWO
//
// IOX uses pin 45 for SWO ITM output
#define GPIO_AP3_SWO                45

//
// LEDs
//
#define GPIO_LED5                   46
#define GPIO_LED4                   47
#define GPIO_LED3                   48
#define GPIO_LED2                   49

// TODO: Verify these based on the
// board placement
#define GPIO_LED_MODE_B3    GPIO_LED5
#define GPIO_LED_MODE_B2    GPIO_LED4
#define GPIO_LED_MODE_B1    GPIO_LED3
#define GPIO_LED_MODE_B0    GPIO_LED2

// If debugging on Apollo3 EVB, redefine some pins
#ifdef DEBUG_ON_AP3_EVB
// LEDs
#undef GPIO_LED_MODE_B3
#undef GPIO_LED_MODE_B2
#undef GPIO_LED_MODE_B1
#undef GPIO_LED_MODE_B0
#define GPIO_LED_MODE_B3    AM_BSP_GPIO_LED4
#define GPIO_LED_MODE_B2    AM_BSP_GPIO_LED3
#define GPIO_LED_MODE_B1    AM_BSP_GPIO_LED2
#define GPIO_LED_MODE_B0    AM_BSP_GPIO_LED1
// Button
#undef GPIO_CFG_SW
#define GPIO_CFG_SW                 19 // BTN2 on AP3 EVB
#endif

//
// Enums for pin group configurations
//

// MSPIx16
typedef enum
{
    MSPI16_AP_PSRAM,
    MSPI16_WB_PSRAM,
    MSPI16_PC
} mspi16_options_e;

// SDIO
typedef enum
{
    SDIO_eMMC,
    SDIO_EXT,
    SDIO_PC,
    SDIO_UNUSED // SDIO1 only
} sdio_options_e;

// MSPIx8
typedef enum
{
    MSPI8_FLASH,
    MSPI8_PC
} mspi8_options_e;

// PDM
typedef enum
{
    PDM_OPEN,
    PDM_HDR,
    PDM_UART,
    PDM_PC,
    PDM_SDIO1 // PDM0 only
} pdm_options_e;

// IOM
typedef enum
{
    IOM_OPEN,
    IOM_PSRAM,
    IOM_I2S_HDR,
    IOM_IOS_LB,
    IOM_SDIO1,
    IOM_PC
} iom_options_e;

// UART
typedef enum
{
    UART_HDR_FTDI,
    UART_VCOM
} uart_options_e;

// DISP
typedef enum
{
    DISP_OB_DISPLAY,
    DISP_PC
} disp_options_e;

#endif
