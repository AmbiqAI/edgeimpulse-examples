//*****************************************************************************
//
//! @file iox_ap5_eb_tornado.c
//!
//! @brief IOX code for Apollo5 Tornado board.
//!
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
#include "iox_ap5_eb_tornado.h"

// #define DEBUG_OUTPUT_EN  // Uncomment to enable debug output

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
int8_t g_mode = -1; // variable to track mode switch setting
volatile bool g_bModeSwitch = true; // set true for initial mode config

// pin group configurations (default to PC settings)
mspi03_options_e g_mspi0State = MSPI03_UNUSE;
mspi03_options_e g_mspi3State = MSPI03_UNUSE;
mspi12_options_e g_mspi1State = MSPI12_UNUSE;
mspi12_options_e g_mspi2State = MSPI12_UNUSE;
iom_options_e g_iom0State = IOM_UNUSE;
iom_options_e g_iom1State = IOM_UNUSE;
iom_options_e g_iom2State = IOM_UNUSE;
iom_options_e g_iom3State = IOM_UNUSE;
iom_options_e g_iom4State = IOM_UNUSE;
iom_options_e g_iom5State = IOM_UNUSE;
iom_options_e g_iom6State = IOM_UNUSE;
iom_options_e g_iom7State = IOM_UNUSE;
sdio_options_e g_sdio0State = SDIO_UNUSE;
sdio_options_e g_sdio1State = SDIO_UNUSE;
disp_options_e g_displayState = DISP_UNUSE;

// list of all mux sel GPIOs
uint32_t g_ui32MuxGpioList[] = {
    VDD_18_EN       ,
    MSPI0_SEL       ,
    MSPI0_OEn       ,
    MSPI1_SEL0      ,
    MSPI1_SEL1      ,
    MSPI1_SEL2      ,
    MSPI1_SEL3      ,
    MSPI1_x4CLK_SEL ,
    MSPI1_x4CLK_OEn ,
    MSPI2_SEL0      ,
    MSPI2_SEL1      ,
    MSPI2_SEL2      ,
    MSPI2_SEL3      ,
    MSPI2_x4CLK_SEL ,
    MSPI2_x4CLK_OEn ,
    MSPI3_SEL       ,
    MSPI3_OEn       ,
    SDIO0_MUX_CTRL  ,
    SDIO1_MUX_CTRL  ,
    EMMC0_32_EN     ,
    EMMC1_32_EN     ,
    IOM0_SEL1       ,
    IOM0_SEL0       ,
    IOM1_SEL        ,
    IOM2_SEL        ,
    IOM3_SEL        ,
    IOM4_SEL        ,
    IOM5_SEL        ,
    IOM6_SEL        ,
    IOM7_SEL        ,
    IOM1_OEn        ,
    IOM2_OEn        ,
    IOM3_OEn        ,
    IOM4_OEn        ,
    IOM5_OEn        ,
    IOM6_OEn        ,
    IOM7_OEn        ,
    MIP_ENn         ,
    TOUCH_EN
};

// Mode button GPIO config
const am_hal_gpio_pincfg_t g_sModeCfg =
{
    .uFuncSel = 3,
    .eIntDir = AM_HAL_GPIO_PIN_INTDIR_HI2LO,
    .eGPInput = AM_HAL_GPIO_PIN_INPUT_ENABLE,
};


//*****************************************************************************
//
// am_gpio_isr
//
//*****************************************************************************
void am_gpio_isr()
{
    // Debounce
    am_util_delay_ms(10);

    uint64_t ui64Status;

    am_hal_gpio_interrupt_status_get(false, &ui64Status);
    am_hal_gpio_interrupt_clear(ui64Status);

    if ( am_hal_gpio_input_read(MODE_CFG_GPIO_M0) == 0 )
    {
        g_bModeSwitch = true;
    }
}


//*****************************************************************************
//
// GPIO initialization
//
//*****************************************************************************
void gpio_init(void)
{
    //
    // Init LEDs as outputs, OFF
    //
    am_hal_gpio_output_clear(APX_LED);
    am_hal_gpio_pinconfig(APX_LED, g_AM_HAL_GPIO_OUTPUT);

    //
    // Init Muxes as outputs
    //
    uint32_t ui32MuxGpioCount = sizeof(g_ui32MuxGpioList) / sizeof(g_ui32MuxGpioList[0]);
    for (uint32_t i = 0; i < ui32MuxGpioCount; i++)
    {
        am_hal_gpio_pinconfig(g_ui32MuxGpioList[i], g_AM_HAL_GPIO_OUTPUT);
    }

    //
    // Initialize mode button as Input, interrupt on falling edge
    //
    am_hal_gpio_pinconfig(MODE_CFG_GPIO_M0, g_sModeCfg);
    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, MODE_CFG_GPIO_M0));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, MODE_CFG_GPIO_M0));
    //
    // Enable GPIO interrupts to the NVIC.
    //
    NVIC_EnableIRQ(GPIO_IRQn);

}

//*****************************************************************************
//
// write_mux_pin(gpio, value): Writes value to mux sel pin
//
// gpio: gpio pin
// value: 0 or 1 (pin will be set to one if value is non-zero)
//
//*****************************************************************************
void write_mux_pin(uint32_t gpio, uint8_t value)
{
    if (value == 0)
    {
        am_hal_gpio_output_clear(gpio);
    }
    else
    {
        am_hal_gpio_output_set(gpio);
    }
}

//*****************************************************************************
//
// The following functions are for setting mux lines based on pin group
// configurations.  For each pin group, there is an enum defined in
// iox_ap5_ps.h which denotes the possible settings for that group.
//
//*****************************************************************************

//*****************************************************************************
//
// config_mspi03
//
//*****************************************************************************
void config_mspi03(uint8_t mspi, mspi03_options_e eState)
{
    if ( mspi == 0 )
    {
        switch (eState)
        {
            case MSPI03_HEX_AP_PSRAM:
                write_mux_pin(MSPI0_SEL, 0);
                write_mux_pin(MSPI0_OEn, 0);
                write_mux_pin(VDD_18_EN, 1);
                break;
            case MSPI03_HEX_WB_PSRAM:
                write_mux_pin(MSPI0_SEL, 1);
                write_mux_pin(MSPI0_OEn, 0);
                write_mux_pin(VDD_18_EN, 0);
                break;
            case MSPI03_UNUSE:
                write_mux_pin(MSPI0_SEL, 0);    // this is a "don't care"
                write_mux_pin(MSPI0_OEn, 1);
                write_mux_pin(VDD_18_EN, 0);
            break;
            default:
                am_util_stdio_printf("ERROR: config_mspi03 - state %d is invalid\n", eState);
                break;
        }
    }
    else if ( mspi == 3 )
    {
        switch (eState)
        {
            case MSPI03_HEX_AP_PSRAM:
                write_mux_pin(MSPI3_SEL, 0);
                write_mux_pin(MSPI3_OEn, 0);
                write_mux_pin(VDD_18_EN, 1);
                break;
            case MSPI03_HEX_WB_PSRAM:
                write_mux_pin(MSPI3_SEL, 1);
                write_mux_pin(MSPI3_OEn, 0);
                write_mux_pin(VDD_18_EN, 0);
                break;
            case MSPI03_UNUSE:
                write_mux_pin(MSPI3_SEL, 0);    // this is a "don't care"
                write_mux_pin(MSPI3_OEn, 1);
                write_mux_pin(VDD_18_EN, 0);
            default:
                am_util_stdio_printf("ERROR: config_mspi03 - state %d is invalid\n", eState);
                break;
        }
    }
    else
    {
        am_util_stdio_printf("ERROR: config_mspi03 - mspi %d is invalid\n", mspi);
    }
}

//*****************************************************************************
//
// config_mspi12
//
//*****************************************************************************
void config_mspi12(uint8_t mspi, mspi12_options_e eState)
{
    if ( mspi == 1 )
    {
        switch (eState)
        {
            case MSPI12_OCTAL_PSRAM:
                write_mux_pin(MSPI1_SEL0, 0);
                write_mux_pin(MSPI1_SEL1, 0);
                write_mux_pin(MSPI1_SEL2, 0);
                write_mux_pin(MSPI1_SEL3, 0);
                break;

            case MSPI12_OCTAL_FLASH:
                write_mux_pin(MSPI1_SEL0, 1);
                write_mux_pin(MSPI1_SEL1, 0);
                write_mux_pin(MSPI1_SEL2, 1);
                write_mux_pin(MSPI1_SEL3, 0);
                break;

            case MSPI12_QUAD_PSRAM:
                write_mux_pin(MSPI1_SEL0, 0);
                write_mux_pin(MSPI1_SEL1, 1);
                write_mux_pin(MSPI1_SEL2, 1);
                write_mux_pin(MSPI1_SEL3, 1);
                write_mux_pin(MSPI1_x4CLK_SEL, 0);
                write_mux_pin(MSPI1_x4CLK_OEn, 0);
                break;
            case MSPI12_QUAD_D4_PSRAM:
                write_mux_pin(MSPI1_SEL0, 0);
                write_mux_pin(MSPI1_SEL1, 1);
                write_mux_pin(MSPI1_SEL2, 1);
                write_mux_pin(MSPI1_SEL3, 1);
                write_mux_pin(MSPI1_x4CLK_SEL, 1);
                write_mux_pin(MSPI1_x4CLK_OEn, 0);
                break;
            case MSPI12_DISP:
                write_mux_pin(MSPI1_SEL0, 0);
                write_mux_pin(MSPI1_SEL1, 1);
                write_mux_pin(MSPI1_SEL2, 0);
                write_mux_pin(MSPI1_SEL3, 1);
                write_mux_pin(MSPI1_x4CLK_SEL, 1);
                write_mux_pin(MSPI1_x4CLK_OEn, 0);
                break;
            case MSPI12_UNUSE:
                write_mux_pin(MSPI1_SEL0, 1);
                write_mux_pin(MSPI1_SEL1, 1);
                write_mux_pin(MSPI1_SEL2, 1);
                write_mux_pin(MSPI1_SEL3, 1);
                write_mux_pin(MSPI1_x4CLK_SEL, 1);
                write_mux_pin(MSPI1_x4CLK_OEn, 1);
            default:
                am_util_stdio_printf("ERROR: config_mspi12 - state %d is invalid\n", eState);
                break;
        }
    }
    else if ( mspi == 2 )
    {
        switch (eState)
        {
            case MSPI12_OCTAL_PSRAM:
                write_mux_pin(MSPI2_SEL0, 0);
                write_mux_pin(MSPI2_SEL1, 0);
                write_mux_pin(MSPI2_SEL2, 0);
                write_mux_pin(MSPI2_SEL3, 0);
                break;

            case MSPI12_OCTAL_FLASH:
                write_mux_pin(MSPI2_SEL0, 1);
                write_mux_pin(MSPI2_SEL1, 0);
                write_mux_pin(MSPI2_SEL2, 1);
                write_mux_pin(MSPI2_SEL3, 0);
                break;

            case MSPI12_QUAD_PSRAM:
                write_mux_pin(MSPI2_SEL0, 0);
                write_mux_pin(MSPI2_SEL1, 1);
                write_mux_pin(MSPI2_SEL2, 1);
                write_mux_pin(MSPI2_SEL3, 1);
                write_mux_pin(MSPI2_x4CLK_SEL, 0);
                write_mux_pin(MSPI2_x4CLK_OEn, 0);
                break;
            case MSPI12_QUAD_D4_PSRAM:
                write_mux_pin(MSPI2_SEL0, 0);
                write_mux_pin(MSPI2_SEL1, 1);
                write_mux_pin(MSPI2_SEL2, 1);
                write_mux_pin(MSPI2_SEL3, 1);
                write_mux_pin(MSPI2_x4CLK_SEL, 1);
                write_mux_pin(MSPI2_x4CLK_OEn, 0);
                break;
            case MSPI12_DISP:
                write_mux_pin(MSPI2_SEL0, 0);
                write_mux_pin(MSPI2_SEL1, 1);
                write_mux_pin(MSPI2_SEL2, 0);
                write_mux_pin(MSPI2_SEL3, 1);
                write_mux_pin(MSPI2_x4CLK_SEL, 1);
                write_mux_pin(MSPI2_x4CLK_OEn, 0);
                break;
            case MSPI12_UNUSE:
                write_mux_pin(MSPI2_SEL0, 1);
                write_mux_pin(MSPI2_SEL1, 1);
                write_mux_pin(MSPI2_SEL2, 1);
                write_mux_pin(MSPI2_SEL3, 1);
                write_mux_pin(MSPI2_x4CLK_SEL, 1);
                write_mux_pin(MSPI2_x4CLK_OEn, 1);
            default:
                am_util_stdio_printf("ERROR: config_mspi12 - state %d is invalid\n", eState);
                break;
        }
    }
    else
    {
        am_util_stdio_printf("ERROR: config_mspi12 - mspi %d is invalid\n", mspi);
    }
}


//*****************************************************************************
//
// config_sdio
//
// uint8_t sdio: SDIO # (0 or 1)
// sdio_options_e eState: SDIO state (SDIO_EMMC, SDIO_EXT, SDIO_UNUSE, or SDIO_UNUSED*)
//                  *SDIO_UNUSED is valid for SDIO1 only
//
//*****************************************************************************
void config_sdio(uint8_t sdio, sdio_options_e eState)
{
    if (sdio == 0) // config SDIO0
    {
        switch (eState)
        {
            case SDIO_eMMC:
                write_mux_pin(EMMC0_32_EN, 0);
                am_util_delay_ms(500);
                write_mux_pin(EMMC0_32_EN, 1);
                am_util_delay_ms(100);
                write_mux_pin(SDIO0_MUX_CTRL, 1);
                break;
            case SDIO_UNUSE:
                write_mux_pin(SDIO0_MUX_CTRL, 0);
                write_mux_pin(EMMC0_32_EN, 0);
                break;
            default:
                break;
        }
    }
    else // config SDIO1
    {
        switch (eState)
        {
            case SDIO_eMMC:
                write_mux_pin(EMMC1_32_EN, 0);
                am_util_delay_ms(500);
                write_mux_pin(EMMC1_32_EN, 1);
                am_util_delay_ms(100);
                write_mux_pin(SDIO1_MUX_CTRL, 1);

                break;
            case SDIO_UNUSE:
                write_mux_pin(SDIO1_MUX_CTRL, 0);
                write_mux_pin(EMMC1_32_EN, 0);
                break;
            default:
                break;
        }
    }
}

//*****************************************************************************
//
// config_iom
//
//*****************************************************************************
void config_iom(uint8_t iom, iom_options_e eState)
{
    if (iom == 0) // config IOM0
    {
        switch (eState)
        {
            case IOM_IOS:
                write_mux_pin(IOM0_SEL0, 0);
                write_mux_pin(IOM0_SEL1, 0);
                break;
            case IOM_PSRAM:
                write_mux_pin(IOM0_SEL0, 1);
                write_mux_pin(IOM0_SEL1, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(IOM0_SEL0, 0);
                write_mux_pin(IOM0_SEL1, 1);
                break;
            case IOM_TOUCH:
                write_mux_pin(IOM0_SEL0, 0);
                write_mux_pin(IOM0_SEL1, 1);
                break;
            case IOM_UNUSE:
                write_mux_pin(IOM0_SEL0, 1);
                write_mux_pin(IOM0_SEL1, 1);
                break;
            default:
                break;
        }
    }
    else if (iom == 1)
    {
        switch (eState)
        {
            case IOM_PSRAM:
                write_mux_pin(IOM1_OEn, 0);
                write_mux_pin(IOM1_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(IOM1_OEn, 0);
                write_mux_pin(IOM1_SEL, 1);
                break;
            case IOM_TOUCH:
                write_mux_pin(IOM1_OEn, 0);
                write_mux_pin(IOM1_SEL, 1);
                break;
            case IOM_UNUSE:
                write_mux_pin(IOM1_OEn, 1);
                break;
            default:
                break;
        }
    }
    else if (iom == 2)
    {
        switch (eState)
        {
            case IOM_PSRAM:
                write_mux_pin(IOM2_OEn, 0);
                write_mux_pin(IOM2_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(IOM2_OEn, 0);
                write_mux_pin(IOM2_SEL, 1);
                break;
            case IOM_TOUCH:
                write_mux_pin(IOM2_OEn, 0);
                write_mux_pin(IOM2_SEL, 1);
                break;
            case IOM_UNUSE:
                write_mux_pin(IOM2_OEn, 1);
                break;
            default:
                break;
        }
    }
    else if (iom == 3)
    {
        switch (eState)
        {
            case IOM_PSRAM:
                write_mux_pin(IOM3_OEn, 0);
                write_mux_pin(IOM3_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(IOM3_OEn, 0);
                write_mux_pin(IOM3_SEL, 1);
                break;
            case IOM_TOUCH:
                write_mux_pin(IOM3_OEn, 0);
                write_mux_pin(IOM3_SEL, 1);
                break;
            case IOM_UNUSE:
                write_mux_pin(IOM3_OEn, 1);
                break;
            default:
                break;
        }
    }
    else if (iom == 4)
    {
        switch (eState)
        {
            case IOM_PSRAM:
                write_mux_pin(IOM4_OEn, 0);
                write_mux_pin(IOM4_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(IOM4_OEn, 0);
                write_mux_pin(IOM4_SEL, 1);
                break;
            case IOM_TOUCH:
                write_mux_pin(IOM4_OEn, 0);
                write_mux_pin(IOM4_SEL, 1);
                break;
            case IOM_UNUSE:
                write_mux_pin(IOM4_OEn, 1);
                break;
            default:
                break;
        }
    }
    else if (iom == 5)
    {
        switch (eState)
        {
            case IOM_PSRAM:
                write_mux_pin(IOM5_OEn, 0);
                write_mux_pin(IOM5_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(IOM5_OEn, 0);
                write_mux_pin(IOM5_SEL, 1);
                break;
            case IOM_TOUCH:
                write_mux_pin(IOM5_OEn, 0);
                write_mux_pin(IOM5_SEL, 1);
                break;
            case IOM_UNUSE:
                write_mux_pin(IOM5_OEn, 1);
                break;
            default:
                break;
        }
    }
    else if (iom == 6)
    {
        switch (eState)
        {
            case IOM_PSRAM:
                write_mux_pin(IOM6_OEn, 0);
                write_mux_pin(IOM6_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(IOM6_OEn, 0);
                write_mux_pin(IOM6_SEL, 1);
                break;
            case IOM_TOUCH:
                write_mux_pin(IOM6_OEn, 0);
                write_mux_pin(IOM6_SEL, 1);
                break;
            case IOM_UNUSE:
                write_mux_pin(IOM6_OEn, 1);
                break;
            default:
                break;
        }
    }
    else if (iom == 7)
    {
        switch (eState)
        {
            case IOM_PSRAM:
                write_mux_pin(IOM7_OEn, 0);
                write_mux_pin(IOM7_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(IOM7_OEn, 0);
                write_mux_pin(IOM7_SEL, 1);
                break;
            case IOM_TOUCH:
                write_mux_pin(IOM7_OEn, 0);
                write_mux_pin(IOM7_SEL, 1);
                break;
            case IOM_UNUSE:
                write_mux_pin(IOM7_OEn, 1);
                break;
            default:
                break;
        }
    }
}



//*****************************************************************************
//
// config_disp
//
//*****************************************************************************
void config_disp(disp_options_e eState)
{
    switch (eState)
    {
        case DISP_ENABLE:
            write_mux_pin(MIP_ENn, 0);
            write_mux_pin(TOUCH_EN, 1) ;
            break;
        case DISP_UNUSE:
            write_mux_pin(MIP_ENn, 1);
            write_mux_pin(TOUCH_EN, 0) ;
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
// update_iox_state: configures all of the mux sel lines based on the current
// values of the global state variables for each pin group.
//
//*****************************************************************************
void update_iox_state(void)
{
    config_mspi03(0, g_mspi0State);
    config_mspi03(3, g_mspi3State);
    config_mspi12(1, g_mspi1State);
    config_mspi12(2, g_mspi2State);
    config_iom(0, g_iom0State);
    config_iom(1, g_iom1State);
    config_iom(2, g_iom2State);
    config_iom(3, g_iom3State);
    config_iom(4, g_iom4State);
    config_iom(5, g_iom5State);
    config_iom(6, g_iom6State);
    config_iom(7, g_iom7State);
    config_sdio(0, g_sdio0State);
    config_sdio(1, g_sdio1State);
    config_disp(g_displayState);
}

//*****************************************************************************
//
// led_on(led_gpio): turn on the LED connected to GPIO # led_gpio
//
//*****************************************************************************
void led_on(uint32_t led_gpio)
{
    am_hal_gpio_output_set(led_gpio);
}

//*****************************************************************************
//
// led_off(led_gpio): turn off the LED connected to GPIO # led_gpio
//
//*****************************************************************************
void led_off(uint32_t led_gpio)
{
    am_hal_gpio_output_clear(led_gpio);
}

//*****************************************************************************
//
// update_leds(): Update the bank of LEDs to reflect the current mode
//
//*****************************************************************************
void update_leds(void)
{
    if (g_mode == 0)
    {
        led_off(APX_LED);
    }
    else
    {
        for ( uint32_t i = 0; i < g_mode; i++)
        {
            led_off(APX_LED);
            am_util_delay_ms(200);
            led_on(APX_LED);
            am_util_delay_ms(200);
        }
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
    // am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    // am_util_stdio_terminal_clear();
    // am_util_stdio_printf("Apollo5 Presilicon Board IOX\n");
    // am_util_stdio_printf("Firmware version: %d\n\n", IOX_FW_REV);

    gpio_init(); // Initialize all GPIOs

    //
    // Enable interrupts to the core.
    //
    am_hal_interrupt_master_enable();

    //
    // Loop forever waiting for button press
    //
    while (1)
    {
        if (g_bModeSwitch)
        {
            g_bModeSwitch = false;
            g_mode = (g_mode + 1) & 0xf; // mode is limited to 4 bits

#ifdef DEBUG_OUTPUT_EN
            am_util_stdio_printf("Button pressed - new mode is %d\n", g_mode);
#endif
            switch (g_mode)
            {
                case 0:
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 0 - All groups are configured to unuse.\n");
#endif
                    g_mspi0State = MSPI03_UNUSE;
                    g_mspi1State = MSPI12_UNUSE;
                    g_mspi2State = MSPI12_UNUSE;
                    g_mspi3State = MSPI03_UNUSE;
                    g_iom0State = IOM_UNUSE;
                    g_iom1State = IOM_UNUSE;
                    g_iom2State = IOM_UNUSE;
                    g_iom3State = IOM_UNUSE;
                    g_iom4State = IOM_UNUSE;
                    g_iom5State = IOM_UNUSE;
                    g_iom6State = IOM_UNUSE;
                    g_iom7State = IOM_UNUSE;
                    g_sdio0State = SDIO_UNUSE;
                    g_sdio1State = SDIO_UNUSE;
                    g_displayState = DISP_UNUSE;
                    break;
                case 1:
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 1 - AP x16 PSRAM + Display +  Touch ALS\n");
#endif
                    g_mspi0State = MSPI03_HEX_AP_PSRAM;
                    g_mspi1State = MSPI12_UNUSE;
                    g_mspi2State = MSPI12_DISP;
                    g_mspi3State = MSPI03_HEX_AP_PSRAM;

                    g_iom0State = IOM_TOUCH; // EC1: Touch ALS
                    g_iom1State = IOM_UNUSE;
                    g_iom2State = IOM_UNUSE;
                    g_iom3State = IOM_UNUSE;
                    g_iom4State = IOM_UNUSE;
                    g_iom5State = IOM_UNUSE;
                    g_iom6State = IOM_UNUSE;
                    g_iom7State = IOM_UNUSE;

                    g_sdio0State = SDIO_UNUSE;
                    g_sdio1State = SDIO_UNUSE;

                    g_displayState = DISP_ENABLE;
                    break;
                case 2:
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 2 - Display + Touch ALS\n");
#endif
                    g_mspi0State = MSPI03_UNUSE;
                    g_mspi1State = MSPI12_UNUSE;
                    g_mspi2State = MSPI12_DISP;
                    g_mspi3State = MSPI03_UNUSE;

                    g_iom0State = IOM_TOUCH; // EC1: Touch ALS
                    g_iom1State = IOM_UNUSE;
                    g_iom2State = IOM_UNUSE;
                    g_iom3State = IOM_UNUSE;
                    g_iom4State = IOM_UNUSE;
                    g_iom5State = IOM_UNUSE;
                    g_iom6State = IOM_UNUSE;
                    g_iom7State = IOM_UNUSE;

                    g_sdio0State = SDIO_UNUSE;
                    g_sdio1State = SDIO_UNUSE;

                    g_displayState = DISP_ENABLE;
                    break;

                case 3:
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 3 - WB x16\n");
#endif
                    g_mspi0State = MSPI03_HEX_WB_PSRAM;
                    g_mspi1State = MSPI12_QUAD_PSRAM;
                    g_mspi2State = MSPI12_OCTAL_FLASH;
                    g_mspi3State = MSPI03_HEX_WB_PSRAM;

                    g_iom0State = IOM_IOS;
                    g_iom1State = IOM_FRAM;
                    g_iom2State = IOM_PSRAM;
                    g_iom3State = IOM_PSRAM;
                    g_iom4State = IOM_PSRAM;
                    g_iom5State = IOM_PSRAM;
                    g_iom6State = IOM_PSRAM;
                    g_iom7State = IOM_PSRAM;

                    g_sdio0State = SDIO_eMMC;
                    g_sdio1State = SDIO_eMMC;

                    g_displayState = DISP_UNUSE;
                    break;

                case 4:
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 4 - Quad PSRAM CLKOND4\n");
#endif
                    g_mspi0State = MSPI03_HEX_WB_PSRAM;
                    g_mspi1State = MSPI12_QUAD_D4_PSRAM;
                    g_mspi2State = MSPI12_QUAD_D4_PSRAM;
                    g_mspi3State = MSPI03_HEX_WB_PSRAM;

                    g_iom0State = IOM_IOS;
                    g_iom1State = IOM_FRAM;
                    g_iom2State = IOM_PSRAM;
                    g_iom3State = IOM_PSRAM;
                    g_iom4State = IOM_PSRAM;
                    g_iom5State = IOM_PSRAM;
                    g_iom6State = IOM_PSRAM;
                    g_iom7State = IOM_PSRAM;

                    g_sdio0State = SDIO_eMMC;
                    g_sdio1State = SDIO_eMMC;
                    g_displayState = DISP_UNUSE;
                    break;
                //
                // Placeholders for new modes
                //
#ifdef DEBUG_ON_AP3_EVB // cycle through all modes if debugging on Apollo3 EVB
                case 5:
                    break;
                case 6:
                    break;
                case 7:
                    break;
                case 8:
                    break;
                case 9:
                    break;
                case 10:
                    break;
                case 11:
                    break;
                case 12:
                    break;
                case 13:
                    break;
                case 14:
                    break;
                case 15:
                    break;
#endif
                default:
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Undefined Mode - resetting\n");
#endif
                    g_mode = 0; // reset mode to 0 if not defined
                    //g_bModeSwitch = true;
                    break;
            };
            update_iox_state(); // update iox pins based on new state variable settings
            update_leds();
        }
    }
}
