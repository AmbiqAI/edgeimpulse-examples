//*****************************************************************************
//
//  ap3bp_mux_define.h
//! @file
//!
//! @brief
//!
//! @addtogroup
//! @addtogroup
//! @ingroup
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

#ifndef AP3BP_MUX_DEFINE_H
#define AP3BP_MUX_DEFINE_H

#include <stdint.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp_pins.h"

//IOS.
#define AP3BP_IOS_SCLK          0
#define AP3BP_IOS_MOSI          1
#define AP3BP_IOS_MISO          2
#define AP3BP_IOS_CE            3

//DIPSW. From front to end.
#define DIPSW_0                 60
#define DIPSW_1                 61
#define DIPSW_2                 62
#define DIPSW_3                 63
#define DIPSW_4                 64
#define DIPSW_5                 65

#define DIPSW_0_BIT             (1 << 0)
#define DIPSW_1_BIT             (1 << 1)
#define DIPSW_2_BIT             (1 << 2)
#define DIPSW_3_BIT             (1 << 3)
#define DIPSW_4_BIT             (1 << 4)
#define DIPSW_5_BIT             (1 << 5)

//LED.
#define AP3BP_LED               66

//MSPI.
#define MUX_MSPI0_OE            4
#define MUX_MSPI0_L1_SEL        5
#define MUX_MSPI0_L2_SEL        6
#define MUX_MSPI0_L3_SEL        7
#define MUX_MSPI1_OE            8
#define MUX_MSPI1_L1_SEL        9
#define MUX_MSPI1_L2_SEL        10
#define MUX_MSPI1_L3A_SEL       11
#define MUX_MSPI1_L3B_SEL       12
#define MUX_MSPI2_OE            13
#define MUX_MSPI2_L1_SEL        14
#define MUX_MSPI2_L2_SEL        15
#define MUX_MSPI2_L3_SEL        16

//SDIO.
#define MUX_SDIO_OE             17
#define MUX_SDIO_L1_SEL         18

//IOM.
#define MUX_IOM0_OE             19
#define MUX_IOM0_L1_SEL         22
#define MUX_IOM0_L2_SEL         23
#define MUX_IOM0_L3_SEL         24
#define MUX_IOM1_OE             25
#define MUX_IOM1_L1_SEL         26
#define MUX_IOM1_L2_SEL         27
#define MUX_IOM2_OE             28
#define MUX_IOM2_L1_SEL         29
#define MUX_IOM2_L2_SEL         30
#define MUX_IOM2_L3_SEL         31
#define MUX_IOM3_OE             32
#define MUX_IOM3_L1_SEL         33
#define MUX_IOM3_L2_SEL         34
//#define MUX_IOM4_OE             35
//#define MUX_IOM4_L1_SEL         36
//#define MUX_IOM4_L2_SEL         37
//#define MUX_IOM4_L3_SEL         38
#define MUX_IOM5_OE             39
#define MUX_IOM5_L1_SEL         40
#define MUX_IOM5_L2_SEL         42
#define MUX_IOM5_L3_SEL         43
#define MUX_IOM6_OE             44
#define MUX_IOM6_L1_SEL         45
#define MUX_IOM6_L2_SEL         46
#define MUX_IOM7_OE             47
#define MUX_IOM7_L1_SEL         48
#define MUX_IOM7_L2_SEL         49
#define MUX_IOM7_L3_SEL         50

//IOS.
#define MUX_IOS_OE              51
#define MUX_IOS_L1_SEL          52
#define MUX_IOS_L2_SEL          53

//UART.
#define MUX_UART_OE             54
#define MUX_UART_L1_SEL         55

//SWD.
#define MUX_SWD_OE              56
#define MUX_SWD_L1_SEL          57

//SWO.
#define AP3BP_SWO               41

typedef enum
{
    DIPSW_NONE = 0, //use ios.
//MSPI
    DIPSW_MSPI0_L1B,
    DIPSW_MSPI0_L2B,
    DIPSW_MSPI0_L3A_MSPI1_L3C,
    DIPSW_MSPI0_L3B_MSPI1_L3D,
    DIPSW_MSPI1_L1B,
    DIPSW_MSPI1_L3A,
    DIPSW_MSPI1_L3B,
    DIPSW_MSPI2_L1B,
    DIPSW_MSPI2_L2B,
    DIPSW_MSPI2_L3A,
    DIPSW_MSPI2_L3B,
//SDIO
    DIPSW_MSPI2_L3B_SDIO_L1A,
    DIPSW_SDIO_L1B,
//IOM.
    DIPSW_IOM0_L1B,
    DIPSW_IOM0_L2B,
    DIPSW_IOM0_L3A,
    DIPSW_IOM0_L3B,
    DIPSW_IOM1_L1B,
    DIPSW_IOM1_L2A,
    DIPSW_IOM1_L2B,
    DIPSW_IOM2_L1B,
    DIPSW_IOM2_L2B,
    DIPSW_IOM2_L3A,
    DIPSW_IOM2_L3B,
    DIPSW_IOM3_L1B,
    DIPSW_IOM3_L2A,
    DIPSW_IOM3_L2B,
    DIPSW_IOM4_L1B,
    DIPSW_IOM4_L2B,
    DIPSW_IOM4_L3A,
    DIPSW_IOM4_L3B,
    DIPSW_IOM5_L1B,
    DIPSW_IOM5_L2B,
    DIPSW_IOM5_L3A,
    DIPSW_IOM5_L3B,
    DIPSW_IOM6_L1B,
    DIPSW_IOM6_L2A,
    DIPSW_IOM6_L2B,
    DIPSW_IOM7_L1B,
    DIPSW_IOM7_L2B,
    DIPSW_IOM7_L3A,
    DIPSW_IOM7_L3B,
//IOS.
    DIPSW_IOS_L1B,
    DIPSW_IOS_L2A,
    DIPSW_IOS_L2B,
//UART.
    DIPSW_UART_L1A,
    DIPSW_UART_L1B,
//SWD.
    //DIPSW_SWD_L1A,
    //DIPSW_SWD_L1A_IOM7_L2B,  // + SWO.

    DIPSW_SWD_L1B,
//PC HARNESS
    DIPSW_PC_HARNESS_ALL,

} DIPSW_STATUS_Enum;

typedef enum
{
  DIPSW_DEVICE_ATXP032   = 0,      //DIPSW_MSPI0_L2B,
  DIPSW_DEVICE_APS25616N,          //DIPSW_MSPI0_L3A_MSPI1_L3C,
  DIPSW_DEVICE_W958D6NW,           //DIPSW_MSPI0_L3B_MSPI1_L3D,

  DIPSW_DEVICE_MSPI1_APS12808L,    //DIPSW_MSPI1_L3A,
  DIPSW_DEVICE_DISP_QSPI,          //DIPSW_MSPI1_L3B,

  DIPSW_DEVICE_MSPI2_APS12808L,    //DIPSW_MSPI2_L2B,
  DIPSW_DEVICE_MX35UF2GE4AD,       //DIPSW_MSPI2_L3A,
  DIPSW_DEVICE_DISP_COOPER_B,      //DIPSW_MSPI2_L3B,
  DIPSW_DEVICE_THGBMNG5D1LBAIT,    //DIPSW_MSPI2_L3B_SDIO_L1A,

  DIPSW_DEVICE_IOM0_IOS_LOOPBACK,  //DIPSW_IOM0_L2B,
  DIPSW_DEVICE_IOM0_APS6404L,      //DIPSW_IOM0_L3A,
  DIPSW_DEVICE_IOM0_FRAM_I2C,      //DIPSW_IOM0_L3B,

  DIPSW_DEVICE_IOM1_APS6404L,      //DIPSW_IOM1_L2A,
  DIPSW_DEVICE_IOM1_FRAM_I2C,      //DIPSW_IOM1_L2B,

  DIPSW_DEVICE_IOM2_DISP_TPIF,     //DIPSW_IOM2_L2B,
  DIPSW_DEVICE_IOM2_APS6404L,      //DIPSW_IOM2_L3A,
  DIPSW_DEVICE_IOM2_FRAM_I2C,      //DIPSW_IOM2_L3B,

  DIPSW_DEVICE_IOM3_APS6404L,      //DIPSW_IOM3_L2A,
  DIPSW_DEVICE_IOM3_FRAM_I2C,      //DIPSW_IOM3_L2B,

  DIPSW_DEVICE_DISP_COOPER_A,      //DIPSW_IOM4_L2B,
  DIPSW_DEVICE_IOM4_APS6404L,      //DIPSW_IOM4_L3A,
  DIPSW_DEVICE_IOM4_FRAM_I2C,      //DIPSW_IOM4_L3B,

  DIPSW_DEVICE_AP3BP_IOS,          //DIPSW_IOM5_L2B,
  DIPSW_DEVICE_IOM5_APS6404L,      //DIPSW_IOM5_L3A,
  DIPSW_DEVICE_IOM5_FRAM_I2C,      //DIPSW_IOM5_L3B,

  DIPSW_DEVICE_IOM6_APS6404L,      //DIPSW_IOM6_L2A,
  DIPSW_DEVICE_IOM6_FRAM_I2C,      //DIPSW_IOM6_L2B,

  DIPSW_DEVICE_IOM7_APS6404L,      //DIPSW_IOM7_L3A,
  DIPSW_DEVICE_IOM7_FRAM_I2C,      //DIPSW_IOM7_L3B,

  DIPSW_DEVICE_IOS_IOM0_LOOPBACK,  //DIPSW_IOS_L2A,
  DIPSW_DEVICE_ETM_SWTRACE,        //DIPSW_IOS_L2B,

  DIPSW_DEVICE_JLINK_UART,         //DIPSW_UART_L1A,

//PC HARNESS ALL.
  DIPSW_DEVICE_PC_HARNEES_ALL,     //DIPSW_PC_HARNESS_ALL,

//!enabled by default.
  DIPSW_DEVICE_SWD_SWO,

}DIPSW_DEVICES_SEL_Enum;

typedef enum
{
    DIPSW_SWITCH_ON,
    DIPSW_SWITCH_OFF,
} DIPSW_SWITCH_Enum;


#define MUX_SEL_A                 AM_HAL_GPIO_OUTPUT_CLEAR
#define MUX_SEL_B                 AM_HAL_GPIO_OUTPUT_SET

// MSPI0 group
#define MSPI0_GROUP_HZ                  0
#define MSPI0_GROUP_PC_HARNESS          1
#define MSPI0_GROUP_NOR_FLASH           2
#define MSPI0_GROUP_HEX_PSRAM_AP        3
#define MSPI0_GROUP_HEX_PSRAM_WB        4

// MSPI1 group
#define MSPI1_GROUP_HZ                  0
#define MSPI1_GROUP_PC_HARNESS          1
#define MSPI1_GROUP_OCTAL_PSRAM         2
#define MSPI1_GROUP_ETM                 3
#define MSPI1_GROUP_HEX_PSRAM_AP        4
#define MSPI1_GROUP_HEX_PSRAM_WB        5

//MSPI2 group
#define MSPI2_GROUP_HZ                      0
#define MSPI2_GROUP_PC_HARNESS              1
#define MSPI2_GROUP_OCTAL_PSRAM             2
#define MSPI2_GROUP_QUAD_NAND_FLASH         3
#define MSPI2_GROUP_EMMC_D4_D7_QSPI_DISP    4

//SDIO group
#define SDIO_GROUP_HZ                   0
#define SDIO_GROUP_PC_HARNESS           1
#define SDIO_GROUP_EMMC_D0_D3           2

//IOM0 group
#define IOM0_GROUP_HZ                   0
#define IOM0_GROUP_PC_HARNESS           1
#define IOM0_GROUP_IOM_IOS_LOOP_BACK    2
#define IOM0_GROUP_PSRAM                3
#define IOM0_GROUP_FRAM                 4

//IOM1 group
#define IOM1_GROUP_HZ                   0
#define IOM1_GROUP_PC_HARNESS           1
#define IOM1_GROUP_PSRAM                2
#define IOM1_GROUP_FRAM                 3

//IOM2 group
#define IOM2_GROUP_HZ                   0
#define IOM2_GROUP_PC_HARNESS           1
#define IOM2_GROUP_PSRAM                2
#define IOM2_GROUP_FRAM                 3
#define IOM2_GROUP_DISP_TPIF            4

//IOM3 group
#define IOM3_GROUP_HZ                   0
#define IOM3_GROUP_PC_HARNESS           1
#define IOM3_GROUP_EMMC_LOAD_SW_CTRL    2
#define IOM3_GROUP_FRAM                 3

//IOM5 group
#define IOM5_GROUP_HZ                   0
#define IOM5_GROUP_PC_HARNESS           1
#define IOM5_GROUP_IOS                  2
#define IOM5_GROUP_PSRAM                3
#define IOM5_GROUP_FRAM                 4

//IOM6 group
#define IOM6_GROUP_HZ                   0
#define IOM6_GROUP_PC_HARNESS           1
#define IOM6_GROUP_PSRAM                2
#define IOM6_GROUP_FRAM                 3

//IOM7 group
#define IOM7_GROUP_HZ                       0
#define IOM7_GROUP_PC_HARNESS               1
#define IOM7_GROUP_JLINK_SWO_DISP_RESET     2
#define IOM7_GROUP_PSRAM                    3
#define IOM7_GROUP_FRAM                     4

//IOS group
#define IOS_GROUP_HZ                    0
#define IOS_GROUP_PC_HARNESS            1
#define IOS_GROUP_IOM_IOS_LOOP_BACK     2

//UART group
#define UART_GROUP_HZ                   0
#define UART_GROUP_PC_HARNESS           1
#define UART_GROUP_JLINK_UART           2

//SWD group
#define SWD_GROUP_HZ                    0
#define SWD_GROUP_PC_HARNESS            1
#define SWD_GROUP_SWD                   2

#define LED_ON()        am_hal_gpio_state_write(AP3BP_LED, AM_HAL_GPIO_OUTPUT_SET)
#define LED_OFF()       am_hal_gpio_state_write(AP3BP_LED, AM_HAL_GPIO_OUTPUT_CLEAR)


#endif
