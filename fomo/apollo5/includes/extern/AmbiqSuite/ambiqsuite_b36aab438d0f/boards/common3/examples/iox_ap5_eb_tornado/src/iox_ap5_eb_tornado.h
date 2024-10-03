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

#ifndef IOX_AP5_EB_TORNADO_H
#define IOX_AP5_EB_TORNADO_H

//#define DEBUG_ON_AP3_EVB // uncomment to debug on Apollo3 EVB

#define IOX_FW_REV  1 // increment for each version released

//***********************************************************
// GPIO Net Assignments for Tornado expansion Card
//***********************************************************
#define VDD_18_EN        33
#define MSPI0_SEL        19
#define MSPI0_OEn        22
#define MSPI1_SEL0       24
#define MSPI1_SEL1       23
#define MSPI1_SEL2       9
#define MSPI1_SEL3       8
#define MSPI1_x4CLK_SEL  25
#define MSPI1_x4CLK_OEn  26
#define MSPI2_SEL0       28
#define MSPI2_SEL1       27
#define MSPI2_SEL2       41
#define MSPI2_SEL3       40
#define MSPI2_x4CLK_SEL  29
#define MSPI2_x4CLK_OEn  30
#define MSPI3_SEL        31
#define MSPI3_OEn        32
#define SDIO0_MUX_CTRL   37
#define SDIO1_MUX_CTRL   42
#define EMMC0_32_EN      34
#define EMMC1_32_EN      35
#define IOM0_SEL1        10
#define IOM0_SEL0        11
#define IOM1_SEL         43
#define IOM2_SEL         44
#define IOM3_SEL         45
#define IOM4_SEL         46
#define IOM5_SEL         47
#define IOM6_SEL         48
#define IOM7_SEL         49
#define IOM1_OEn         12
#define IOM2_OEn         13
#define IOM3_OEn         14
#define IOM4_OEn         15
#define IOM5_OEn         16
#define IOM6_OEn         17
#define IOM7_OEn         18
#define MIP_ENn          36
#define TOUCH_EN         38

//
// Mode Config GPIO
//
#define MODE_CFG_GPIO_M0    0


//
// LEDs
//
#define APX_LED             39


//
// Enums for pin group configurations
//

// MSPI03
typedef enum
{
    MSPI03_HEX_AP_PSRAM,
    MSPI03_HEX_WB_PSRAM,
    MSPI03_UNUSE
} mspi03_options_e;

// MSPI12
typedef enum
{
    MSPI12_OCTAL_FLASH,
    MSPI12_OCTAL_PSRAM,
    MSPI12_QUAD_PSRAM,
    MSPI12_QUAD_D4_PSRAM,
    MSPI12_DISP,
    MSPI12_UNUSE
} mspi12_options_e;

// SDIO
typedef enum
{
    SDIO_eMMC,
    SDIO_UNUSE
} sdio_options_e;

// IOM
typedef enum
{
    IOM_IOS, // IOM0 only
    IOM_PSRAM,
    IOM_FRAM,
    IOM_TOUCH,
    IOM_UNUSE
} iom_options_e;



// DISP
typedef enum
{
    DISP_ENABLE,
    DISP_UNUSE
} disp_options_e;

#endif
