//*****************************************************************************
//
//! @file ap3bp_mux_apollo4l_blue_eb.c
//!
//! @brief ap3bp_mux_apollo4l_blue_eb example.
//!
//! Purpose: .
//!
//! Printing takes place over the ITM at 1M Baud.
//!
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
#include "am_bsp.h"
#include "am_util.h"
#include "ap3bp_mux.h"
#include "ap3bp_mux_define.h"

#define INVALID_STATUS                  (0xFFFFFFFF)

extern void ios_link_slave_init(void);

//*****************************************************************************
//
// dipsw read status
//
//*****************************************************************************
void disp_switch_scan(void)
{
    uint32_t status = 0;

    uint32_t dipsw0_status = 0;
    uint32_t dipsw1_status = 0;
    uint32_t dipsw2_status = 0;
    uint32_t dipsw3_status = 0;
    uint32_t dipsw4_status = 0;
    uint32_t dipsw5_status = 0;

    am_hal_gpio_state_read(DIPSW_0, AM_HAL_GPIO_INPUT_READ, &dipsw0_status);
    am_hal_gpio_state_read(DIPSW_1, AM_HAL_GPIO_INPUT_READ, &dipsw1_status);
    am_hal_gpio_state_read(DIPSW_2, AM_HAL_GPIO_INPUT_READ, &dipsw2_status);
    am_hal_gpio_state_read(DIPSW_3, AM_HAL_GPIO_INPUT_READ, &dipsw3_status);
    am_hal_gpio_state_read(DIPSW_4, AM_HAL_GPIO_INPUT_READ, &dipsw4_status);
    am_hal_gpio_state_read(DIPSW_5, AM_HAL_GPIO_INPUT_READ, &dipsw5_status);


    status = dipsw0_status * DIPSW_0_BIT +
             dipsw1_status * DIPSW_1_BIT +
             dipsw2_status * DIPSW_2_BIT +
             dipsw3_status * DIPSW_3_BIT +
             dipsw4_status * DIPSW_4_BIT +
             dipsw5_status * DIPSW_5_BIT;

    if ( status == mux_mode_value_get(MUX_MODE_DIP_SWITCH) )
    {
        return;
    }

    am_util_delay_ms(100);

    status = dipsw0_status * DIPSW_0_BIT +
         dipsw1_status * DIPSW_1_BIT +
         dipsw2_status * DIPSW_2_BIT +
         dipsw3_status * DIPSW_3_BIT +
         dipsw4_status * DIPSW_4_BIT +
         dipsw5_status * DIPSW_5_BIT;

    mux_mode_value_set(MUX_MODE_DIP_SWITCH, status);

    return ;
}


//*****************************************************************************
//
// led_init
//
//*****************************************************************************
void led_init()
{
  am_hal_gpio_pinconfig(AP3BP_LED, g_AM_HAL_GPIO_OUTPUT);
  am_hal_gpio_state_write(AP3BP_LED, AM_HAL_GPIO_OUTPUT_CLEAR);
}

//*****************************************************************************
//
// dipsw_init
//
//*****************************************************************************
void dipsw_init()
{
    am_hal_gpio_pinconfig(DIPSW_0, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_1, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_2, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_3, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_4, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(DIPSW_5, g_AM_HAL_GPIO_INPUT);
}


//*****************************************************************************
//
// iomux_gpio_init
//
//*****************************************************************************
void iomux_gpio_init(void)
{
    am_hal_gpio_pinconfig(MUX_MSPI0_OE,      g_AM_HAL_GPIO_OUTPUT);    //  4
    am_hal_gpio_pinconfig(MUX_MSPI0_L1_SEL,  g_AM_HAL_GPIO_OUTPUT);     //  5
    am_hal_gpio_pinconfig(MUX_MSPI0_L2_SEL,  g_AM_HAL_GPIO_OUTPUT);    //  6
    am_hal_gpio_pinconfig(MUX_MSPI0_L3_SEL,  g_AM_HAL_GPIO_OUTPUT);    //  7
    am_hal_gpio_pinconfig(MUX_MSPI1_OE,      g_AM_HAL_GPIO_OUTPUT);    //  8
    am_hal_gpio_pinconfig(MUX_MSPI1_L1_SEL,  g_AM_HAL_GPIO_OUTPUT);    //  9
    am_hal_gpio_pinconfig(MUX_MSPI1_L2_SEL,  g_AM_HAL_GPIO_OUTPUT);    // 10
    am_hal_gpio_pinconfig(MUX_MSPI1_L3A_SEL, g_AM_HAL_GPIO_OUTPUT);    // 11
    am_hal_gpio_pinconfig(MUX_MSPI1_L3B_SEL, g_AM_HAL_GPIO_OUTPUT);    // 12
    am_hal_gpio_pinconfig(MUX_MSPI2_OE,      g_AM_HAL_GPIO_OUTPUT);    // 13
    am_hal_gpio_pinconfig(MUX_MSPI2_L1_SEL,  g_AM_HAL_GPIO_OUTPUT);    // 14
    am_hal_gpio_pinconfig(MUX_MSPI2_L2_SEL,  g_AM_HAL_GPIO_OUTPUT);    // 15
    am_hal_gpio_pinconfig(MUX_MSPI2_L3_SEL,  g_AM_HAL_GPIO_OUTPUT);    // 16
    am_hal_gpio_pinconfig(MUX_SDIO_OE,       g_AM_HAL_GPIO_OUTPUT);    // 17
    am_hal_gpio_pinconfig(MUX_SDIO_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 18
    am_hal_gpio_pinconfig(MUX_IOM0_OE,       g_AM_HAL_GPIO_OUTPUT);    // 19
    am_hal_gpio_pinconfig(MUX_IOM0_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 22
    am_hal_gpio_pinconfig(MUX_IOM0_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 23
    am_hal_gpio_pinconfig(MUX_IOM0_L3_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 24
    am_hal_gpio_pinconfig(MUX_IOM1_OE,       g_AM_HAL_GPIO_OUTPUT);    // 25
    am_hal_gpio_pinconfig(MUX_IOM1_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 26
    am_hal_gpio_pinconfig(MUX_IOM1_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 27
    am_hal_gpio_pinconfig(MUX_IOM2_OE,       g_AM_HAL_GPIO_OUTPUT);    // 28
    am_hal_gpio_pinconfig(MUX_IOM2_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 29
    am_hal_gpio_pinconfig(MUX_IOM2_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 30
    am_hal_gpio_pinconfig(MUX_IOM2_L3_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 31
    am_hal_gpio_pinconfig(MUX_IOM3_OE,       g_AM_HAL_GPIO_OUTPUT);    // 32
    am_hal_gpio_pinconfig(MUX_IOM3_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 33
    am_hal_gpio_pinconfig(MUX_IOM3_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 34
    am_hal_gpio_pinconfig(MUX_IOM5_OE,       g_AM_HAL_GPIO_OUTPUT);    // 39
    am_hal_gpio_pinconfig(MUX_IOM5_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 40
    am_hal_gpio_pinconfig(MUX_IOM5_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 42
    am_hal_gpio_pinconfig(MUX_IOM5_L3_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 43
    am_hal_gpio_pinconfig(MUX_IOM6_OE,       g_AM_HAL_GPIO_OUTPUT);    // 44
    am_hal_gpio_pinconfig(MUX_IOM6_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 45
    am_hal_gpio_pinconfig(MUX_IOM6_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 46
    am_hal_gpio_pinconfig(MUX_IOM7_OE,       g_AM_HAL_GPIO_OUTPUT);    // 47
    am_hal_gpio_pinconfig(MUX_IOM7_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 48
    am_hal_gpio_pinconfig(MUX_IOM7_L2_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 49
    am_hal_gpio_pinconfig(MUX_IOM7_L3_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 50
    am_hal_gpio_pinconfig(MUX_IOS_OE,        g_AM_HAL_GPIO_OUTPUT);    // 51
    am_hal_gpio_pinconfig(MUX_IOS_L1_SEL,    g_AM_HAL_GPIO_OUTPUT);    // 52
    am_hal_gpio_pinconfig(MUX_IOS_L2_SEL,    g_AM_HAL_GPIO_OUTPUT);    // 53
    am_hal_gpio_pinconfig(MUX_UART_OE,       g_AM_HAL_GPIO_OUTPUT);    // 54
    am_hal_gpio_pinconfig(MUX_UART_L1_SEL,   g_AM_HAL_GPIO_OUTPUT);    // 55
    am_hal_gpio_pinconfig(MUX_SWD_OE,        g_AM_HAL_GPIO_OUTPUT);    // 56
    am_hal_gpio_pinconfig(MUX_SWD_L1_SEL,    g_AM_HAL_GPIO_OUTPUT);    // 57
}

//*****************************************************************************
//
// iomux_init
//
//*****************************************************************************
void iomux_init(void)
{
    led_init();
    dipsw_init();
    iomux_gpio_init();
#ifdef USE_IOS
    ios_set_up();
#endif
}

void mspi0_group_set(uint32_t mode)
{
    switch (mode)
    {
        case MSPI0_GROUP_HZ:
            am_hal_gpio_state_write(MUX_MSPI0_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case MSPI0_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_MSPI0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI0_L1_SEL, MUX_SEL_B);
            break;
        case MSPI0_GROUP_NOR_FLASH:
            am_hal_gpio_state_write(MUX_MSPI0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI0_L2_SEL, MUX_SEL_B);
            break;
        case MSPI0_GROUP_HEX_PSRAM_AP:
            am_hal_gpio_state_write(MUX_MSPI0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI0_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI0_L3_SEL, MUX_SEL_A);
            break;
        case MSPI0_GROUP_HEX_PSRAM_WB:
            am_hal_gpio_state_write(MUX_MSPI0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI0_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI0_L3_SEL, MUX_SEL_B);
            break;
    }
}

void mspi1_group_set(uint32_t mode)
{
    switch (mode)
    {
        case MSPI1_GROUP_HZ:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case MSPI1_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI1_L1_SEL, MUX_SEL_B);
            break;
        case MSPI1_GROUP_OCTAL_PSRAM:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI1_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI1_L3A_SEL, MUX_SEL_A);
            break;
        case MSPI1_GROUP_HEX_PSRAM_AP:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI1_L2_SEL, MUX_SEL_B);
            am_hal_gpio_state_write(MUX_MSPI1_L3B_SEL, MUX_SEL_A);
            break;
        case MSPI1_GROUP_HEX_PSRAM_WB:
            am_hal_gpio_state_write(MUX_MSPI1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI1_L2_SEL, MUX_SEL_B);
            am_hal_gpio_state_write(MUX_MSPI1_L3B_SEL, MUX_SEL_B);
            break;
    }
}

void mspi2_group_set(uint32_t mode)
{
    switch (mode)
    {
        case MSPI2_GROUP_HZ:
            am_hal_gpio_state_write(MUX_MSPI2_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case MSPI2_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_MSPI2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI2_L1_SEL, MUX_SEL_B);
            break;
        case MSPI2_GROUP_OCTAL_PSRAM:
            am_hal_gpio_state_write(MUX_MSPI2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI2_L2_SEL, MUX_SEL_B);
            break;
        case MSPI2_GROUP_QUAD_NAND_FLASH:
            am_hal_gpio_state_write(MUX_MSPI2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI2_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI2_L3_SEL, MUX_SEL_A);
            break;
        case MSPI2_GROUP_EMMC_D4_D7_QSPI_DISP:
            am_hal_gpio_state_write(MUX_MSPI2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_MSPI2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI2_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_MSPI2_L3_SEL, MUX_SEL_B);
            break;
    }
}

void sdio_group_set(uint32_t mode)
{
    switch (mode)
    {
        case SDIO_GROUP_HZ:
            am_hal_gpio_state_write(MUX_SDIO_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case SDIO_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_SDIO_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_SDIO_L1_SEL, MUX_SEL_B);
            break;
        case SDIO_GROUP_EMMC_D0_D3:
            am_hal_gpio_state_write(MUX_SDIO_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_SDIO_L1_SEL, MUX_SEL_A);
            break;
    }
}

void iom0_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM0_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM0_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM0_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM0_L1_SEL, MUX_SEL_B);
            break;
        case IOM0_GROUP_IOM_IOS_LOOP_BACK:
            am_hal_gpio_state_write(MUX_IOM0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM0_L2_SEL, MUX_SEL_B);
            break;
        case IOM0_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM0_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM0_L3_SEL, MUX_SEL_A);
            break;
        case IOM0_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM0_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM0_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM0_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM0_L3_SEL, MUX_SEL_B);
            break;
    }
}

void iom1_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM1_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM1_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM1_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM1_L1_SEL, MUX_SEL_B);
            break;
        case IOM1_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM1_L2_SEL, MUX_SEL_A);
            break;
        case IOM1_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM1_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM1_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM1_L2_SEL, MUX_SEL_B);
            break;
    }
}

void iom2_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM2_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM2_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM2_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM2_L1_SEL, MUX_SEL_B);
            break;
        case IOM2_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM2_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM2_L3_SEL, MUX_SEL_A);
            break;
        case IOM2_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM2_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM2_L3_SEL, MUX_SEL_B);
            break;
        case IOM2_GROUP_DISP_TPIF:
            am_hal_gpio_state_write(MUX_IOM2_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM2_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM2_L2_SEL, MUX_SEL_B);
            break;
    }
}

void iom3_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM3_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM3_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM3_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM3_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM3_L1_SEL, MUX_SEL_B);
            break;
        case IOM3_GROUP_EMMC_LOAD_SW_CTRL:
            am_hal_gpio_state_write(MUX_IOM3_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM3_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM3_L2_SEL, MUX_SEL_B);
            break;
        case IOM3_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM3_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM3_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM3_L2_SEL, MUX_SEL_A);
            break;
    }
}


void iom5_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM5_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM5_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM5_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM5_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM5_L1_SEL, MUX_SEL_B);
            break;
        case IOM5_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM5_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM5_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM5_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM5_L3_SEL, MUX_SEL_A);
            break;
        case IOM5_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM5_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM5_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM5_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM5_L3_SEL, MUX_SEL_B);
            break;
        case IOM5_GROUP_IOS:
            am_hal_gpio_state_write(MUX_IOM5_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM5_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM5_L2_SEL, MUX_SEL_B);
            break;
    }
}

void iom6_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM6_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM6_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM6_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM6_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM6_L1_SEL, MUX_SEL_B);
            break;
        case IOM6_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM6_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM6_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM6_L2_SEL, MUX_SEL_A);
            break;
        case IOM6_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM6_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM6_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM6_L2_SEL, MUX_SEL_B);
            break;
    }
}

void iom7_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOM7_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOM7_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOM7_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOM7_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM7_L1_SEL, MUX_SEL_B);
            break;
        case IOM7_GROUP_PSRAM:
            am_hal_gpio_state_write(MUX_IOM7_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM7_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM7_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM7_L3_SEL, MUX_SEL_A);
            break;
        case IOM7_GROUP_FRAM:
            am_hal_gpio_state_write(MUX_IOM7_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM7_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM7_L2_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM7_L3_SEL, MUX_SEL_B);
            break;
        case IOM7_GROUP_JLINK_SWO_DISP_RESET:
            am_hal_gpio_state_write(MUX_IOM7_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOM7_L1_SEL, MUX_SEL_A);
            am_hal_gpio_state_write(MUX_IOM7_L2_SEL, MUX_SEL_B);
            break;
    }
}

void ios_group_set(uint32_t mode)
{
    switch (mode)
    {
        case IOS_GROUP_HZ:
            am_hal_gpio_state_write(MUX_IOS_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case IOS_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_IOS_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOS_L1_SEL, MUX_SEL_B);
            break;
        case IOS_GROUP_IOM_IOS_LOOP_BACK:
            am_hal_gpio_state_write(MUX_IOS_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_IOS_L1_SEL, MUX_SEL_A);
            break;
    }
}

void uart_group_set(uint32_t mode)
{
    switch (mode)
    {
        case UART_GROUP_HZ:
            am_hal_gpio_state_write(MUX_UART_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case UART_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_UART_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_UART_L1_SEL, MUX_SEL_B);
            break;
        case UART_GROUP_JLINK_UART:
            am_hal_gpio_state_write(MUX_UART_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_UART_L1_SEL, MUX_SEL_A);
            break;
    }
}

void swd_group_set(uint32_t mode)
{
    switch (mode)
    {
        case SWD_GROUP_HZ:
            am_hal_gpio_state_write(MUX_SWD_OE, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case SWD_GROUP_PC_HARNESS:
            am_hal_gpio_state_write(MUX_SWD_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_SWD_L1_SEL, MUX_SEL_B);
            break;
        case SWD_GROUP_SWD:
            am_hal_gpio_state_write(MUX_SWD_OE, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(MUX_SWD_L1_SEL, MUX_SEL_A);
            break;
    }
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
    uint32_t preStatus = INVALID_STATUS;
    uint32_t status = INVALID_STATUS;
    bool led_on = true;
    //
    // Set the clock frequency.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("ap3bp_mux_apollo4l_eb!\n\n");


    iomux_init();
    ios_link_slave_init();

    am_hal_interrupt_master_enable();

    disp_switch_scan();
    mux_mode_set(MUX_MODE_DIP_SWITCH);
    while (1)
    {
        disp_switch_scan();
        status = mux_value_get();
        if (status != preStatus)
        {
            am_util_stdio_printf("MUX mode is :%d, MUX value is 0x%x: \n", mux_mode_get(), status);
            preStatus = status;
            led_on = true;
            switch (status)
            {
                case 0://ALL high Z
                    mspi0_group_set(MSPI0_GROUP_HZ);
                    mspi1_group_set(MSPI1_GROUP_HZ);
                    mspi2_group_set(MSPI2_GROUP_HZ);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_HZ);
                    iom1_group_set(IOM1_GROUP_HZ);
                    iom2_group_set(IOM2_GROUP_HZ);
                    iom3_group_set(IOM3_GROUP_HZ);
                    iom5_group_set(IOM5_GROUP_HZ);
                    iom6_group_set(IOM6_GROUP_HZ);
                    iom7_group_set(IOM7_GROUP_HZ);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_HZ);
                break;
                case 1://ALL to Peripheral card
                    mspi0_group_set(MSPI0_GROUP_PC_HARNESS);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_PC_HARNESS);
                    sdio_group_set(SDIO_GROUP_PC_HARNESS);
                    iom0_group_set(IOM0_GROUP_PC_HARNESS);
                    iom1_group_set(IOM1_GROUP_PC_HARNESS);
                    iom2_group_set(IOM2_GROUP_PC_HARNESS);
                    iom3_group_set(IOM3_GROUP_PC_HARNESS);
                    iom5_group_set(IOM5_GROUP_PC_HARNESS);
                    iom6_group_set(IOM6_GROUP_PC_HARNESS);
                    iom7_group_set(IOM7_GROUP_PC_HARNESS);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_PC_HARNESS);
                    swd_group_set(SWD_GROUP_PC_HARNESS);
                break;
                case 2://NAND Flash,MSPI0 NOR Flash
                    mspi0_group_set(MSPI0_GROUP_NOR_FLASH);
                    mspi1_group_set(MSPI1_GROUP_OCTAL_PSRAM);
                    mspi2_group_set(MSPI2_GROUP_QUAD_NAND_FLASH);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_PC_HARNESS);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 3://MSPI0~2 Oct PSRAM(x16 PSRAM AP in oct mode)
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_OCTAL_PSRAM);
                    mspi2_group_set(MSPI2_GROUP_OCTAL_PSRAM);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_PC_HARNESS);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 4://MSPI0~2 Oct PSRAM(x16 PSRAM WB in Oct mode)
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_WB);
                    mspi1_group_set(MSPI1_GROUP_OCTAL_PSRAM);
                    mspi2_group_set(MSPI2_GROUP_OCTAL_PSRAM);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_PC_HARNESS);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 5://MSPI0 x16 PSRAM AP, Display card,eMMC 8-bit
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_HEX_PSRAM_AP);
                    mspi2_group_set(MSPI2_GROUP_EMMC_D4_D7_QSPI_DISP);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_DISP_TPIF);
                    iom3_group_set(IOM3_GROUP_EMMC_LOAD_SW_CTRL);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 6://MSPI0 x16 PSRAM WB, Display card, IOMS LoopBack
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_WB);
                    mspi1_group_set(MSPI1_GROUP_HEX_PSRAM_WB);
                    mspi2_group_set(MSPI2_GROUP_EMMC_D4_D7_QSPI_DISP);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_IOM_IOS_LOOP_BACK);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_DISP_TPIF);
                    iom3_group_set(IOM3_GROUP_EMMC_LOAD_SW_CTRL);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_IOM_IOS_LOOP_BACK);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 7://MSPI0 NOR Flash, eMMC 8-bit,  Display card
                    mspi0_group_set(MSPI0_GROUP_NOR_FLASH);
                    mspi1_group_set(MSPI1_GROUP_OCTAL_PSRAM);
                    mspi2_group_set(MSPI2_GROUP_EMMC_D4_D7_QSPI_DISP);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_FRAM);
                    iom1_group_set(IOM1_GROUP_FRAM);
                    iom2_group_set(IOM2_GROUP_FRAM);
                    iom3_group_set(IOM3_GROUP_EMMC_LOAD_SW_CTRL);
                    iom5_group_set(IOM5_GROUP_FRAM);
                    iom6_group_set(IOM6_GROUP_FRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 8://IOM0~7 SPI
                    mspi0_group_set(MSPI0_GROUP_PC_HARNESS);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_PC_HARNESS);
                    sdio_group_set(SDIO_GROUP_PC_HARNESS);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_PC_HARNESS);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_PSRAM);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 9://IOM0~7 I2C
                    mspi0_group_set(MSPI0_GROUP_PC_HARNESS);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_PC_HARNESS);
                    sdio_group_set(SDIO_GROUP_PC_HARNESS);
                    iom0_group_set(IOM0_GROUP_FRAM);
                    iom1_group_set(IOM1_GROUP_FRAM);
                    iom2_group_set(IOM2_GROUP_FRAM);
                    iom3_group_set(IOM3_GROUP_FRAM);
                    iom5_group_set(IOM5_GROUP_FRAM);
                    iom6_group_set(IOM6_GROUP_FRAM);
                    iom7_group_set(IOM7_GROUP_FRAM);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 10://All to Peripheral card except debug IOs
                    mspi0_group_set(MSPI0_GROUP_PC_HARNESS);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_PC_HARNESS);
                    sdio_group_set(SDIO_GROUP_PC_HARNESS);
                    iom0_group_set(IOM0_GROUP_PC_HARNESS);
                    iom1_group_set(IOM1_GROUP_PC_HARNESS);
                    iom2_group_set(IOM2_GROUP_PC_HARNESS);
                    iom3_group_set(IOM3_GROUP_PC_HARNESS);
                    iom5_group_set(IOM5_GROUP_PC_HARNESS);
                    iom6_group_set(IOM6_GROUP_PC_HARNESS);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 11://MSPI0  x16 PSRAM,DISP card,external eMMC 4-bit,DISP TP_IF
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_HEX_PSRAM_AP);
                    mspi2_group_set(MSPI2_GROUP_EMMC_D4_D7_QSPI_DISP);
                    sdio_group_set(SDIO_GROUP_PC_HARNESS);
                    iom0_group_set(IOM0_GROUP_PC_HARNESS);
                    iom1_group_set(IOM1_GROUP_PC_HARNESS);
                    iom2_group_set(IOM2_GROUP_DISP_TPIF);
                    iom3_group_set(IOM3_GROUP_PC_HARNESS);
                    iom5_group_set(IOM5_GROUP_PC_HARNESS);
                    iom6_group_set(IOM6_GROUP_PC_HARNESS);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 12:        //MSPI Display Board(display on MSPI#1)
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_EMMC_D4_D7_QSPI_DISP);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PC_HARNESS);
                    iom3_group_set(IOM3_GROUP_EMMC_LOAD_SW_CTRL);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 13:        //Enhance stress testcase(display on MSPI#1)
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_QUAD_NAND_FLASH);
                    sdio_group_set(SDIO_GROUP_EMMC_D0_D3);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_EMMC_LOAD_SW_CTRL);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_PC_HARNESS);
                    uart_group_set(UART_GROUP_JLINK_UART);
                    swd_group_set(SWD_GROUP_SWD);
                break;



                case 32://SLT default
                    mspi0_group_set(MSPI0_GROUP_HZ);
                    mspi1_group_set(MSPI1_GROUP_HZ);
                    mspi2_group_set(MSPI2_GROUP_HZ);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_HZ);
                    iom1_group_set(IOM1_GROUP_HZ);
                    iom2_group_set(IOM2_GROUP_HZ);
                    iom3_group_set(IOM3_GROUP_HZ);
                    iom5_group_set(IOM5_GROUP_IOS);
                    iom6_group_set(IOM6_GROUP_HZ);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 33:// SLT MSPI#0 Oct PSRAM
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_HZ);
                    mspi2_group_set(MSPI2_GROUP_HZ);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_HZ);
                    iom1_group_set(IOM1_GROUP_HZ);
                    iom2_group_set(IOM2_GROUP_HZ);
                    iom3_group_set(IOM3_GROUP_HZ);
                    iom5_group_set(IOM5_GROUP_IOS);
                    iom6_group_set(IOM6_GROUP_HZ);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 34:// SLT MSPI#1 Quad PSRAM
                    mspi0_group_set(MSPI0_GROUP_HZ);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_HZ);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_HZ);
                    iom1_group_set(IOM1_GROUP_HZ);
                    iom2_group_set(IOM2_GROUP_HZ);
                    iom3_group_set(IOM3_GROUP_HZ);
                    iom5_group_set(IOM5_GROUP_IOS);
                    iom6_group_set(IOM6_GROUP_HZ);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 35:// SLT MSPI#2 Quad NAND
                    mspi0_group_set(MSPI0_GROUP_HZ);
                    mspi1_group_set(MSPI1_GROUP_HZ);
                    mspi2_group_set(MSPI2_GROUP_QUAD_NAND_FLASH);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_HZ);
                    iom1_group_set(IOM1_GROUP_HZ);
                    iom2_group_set(IOM2_GROUP_HZ);
                    iom3_group_set(IOM3_GROUP_HZ);
                    iom5_group_set(IOM5_GROUP_IOS);
                    iom6_group_set(IOM6_GROUP_HZ);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 36:    //IOM5 SPI
                    mspi0_group_set(MSPI0_GROUP_HZ);
                    mspi1_group_set(MSPI1_GROUP_HZ);
                    mspi2_group_set(MSPI2_GROUP_HZ);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_HZ);
                    iom1_group_set(IOM1_GROUP_HZ);
                    iom2_group_set(IOM2_GROUP_HZ);
                    iom3_group_set(IOM3_GROUP_HZ);
                    iom5_group_set(IOM5_GROUP_PSRAM);
                    iom6_group_set(IOM6_GROUP_HZ);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 37:    //Enhance stress testcase
                    mspi0_group_set(MSPI0_GROUP_HEX_PSRAM_AP);
                    mspi1_group_set(MSPI1_GROUP_PC_HARNESS);
                    mspi2_group_set(MSPI2_GROUP_QUAD_NAND_FLASH);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_PSRAM);
                    iom1_group_set(IOM1_GROUP_PSRAM);
                    iom2_group_set(IOM2_GROUP_PSRAM);
                    iom3_group_set(IOM3_GROUP_HZ);
                    iom5_group_set(IOM5_GROUP_IOS);
                    iom6_group_set(IOM6_GROUP_PSRAM);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_SWD);
                break;
                case 38:    //I2C0~3 FRAM testcase
                    mspi0_group_set(MSPI0_GROUP_HZ);
                    mspi1_group_set(MSPI1_GROUP_HZ);
                    mspi2_group_set(MSPI2_GROUP_HZ);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_FRAM);
                    iom1_group_set(IOM1_GROUP_FRAM);
                    iom2_group_set(IOM2_GROUP_FRAM);
                    iom3_group_set(IOM3_GROUP_FRAM);
                    iom5_group_set(IOM5_GROUP_IOS);
                    iom6_group_set(IOM6_GROUP_HZ);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_SWD);
                break;

                default:
                    mspi0_group_set(MSPI0_GROUP_HZ);
                    mspi1_group_set(MSPI1_GROUP_HZ);
                    mspi2_group_set(MSPI2_GROUP_HZ);
                    sdio_group_set(SDIO_GROUP_HZ);
                    iom0_group_set(IOM0_GROUP_HZ);
                    iom1_group_set(IOM1_GROUP_HZ);
                    iom2_group_set(IOM2_GROUP_HZ);
                    iom3_group_set(IOM3_GROUP_HZ);
                    iom5_group_set(IOM5_GROUP_IOS);
                    iom6_group_set(IOM6_GROUP_HZ);
                    iom7_group_set(IOM7_GROUP_JLINK_SWO_DISP_RESET);
                    ios_group_set(IOS_GROUP_HZ);
                    uart_group_set(UART_GROUP_HZ);
                    swd_group_set(SWD_GROUP_SWD);

                    led_on = false;
                break;
            }

            (led_on == true) ? LED_ON():LED_OFF();
        }

        am_util_delay_ms(500);
    }
}
