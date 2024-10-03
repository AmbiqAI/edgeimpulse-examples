//*****************************************************************************
//
//! @file iox_ap5_eb.c
//!
//! @brief IOX code for Apollo5 Engineering Board
//!
//! Purpose: Processes button press to cycle mux setting through different
//!     modes.  The current mode is indicated by a bank of 4 LEDs.
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
#include "iox_ap5b_eb.h"

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
uint32_t g_mode = 0; // variable to track mode switch setting
volatile bool g_bButtonPress = true; // set true for initial mode config

// pin group configurations (default to PC settings)
swd_options_e g_swdState = SWD_AP5;
trace_options_e g_traceState = TRACE_AP5;
mspi03_options_e g_mspi0State = MSPI03_PC;
mspi03_options_e g_mspi3State = MSPI03_PC;
mspi12_options_e g_mspi1State = MSPI12_PC;
mspi12_options_e g_mspi2State = MSPI12_PC;
iom_options_e g_iom0State = IOM_PC;
iom_options_e g_iom1State = IOM_PC;
iom_options_e g_iom2State = IOM_PC;
iom_options_e g_iom3State = IOM_PC;
iom_options_e g_iom4State = IOM_PC;
iom_options_e g_iom5State = IOM_PC;
iom_options_e g_iom6State = IOM_PC;
iom_options_e g_iom7State = IOM_PC;
uart_options_e g_uartState = UART_HDR_FTDI;
sdio_options_e g_sdio0State = SDIO_PC;
sdio_rst_options_e g_sdio0RstState = SDIO_RST_OPEN;
sdio_options_e g_sdio1State = SDIO_PC;
sdio_rst_options_e g_sdio1RstState = SDIO_RST_OPEN;
bool g_configTornadoBoard = false;

// list of all mux OEn GPIOs (these are pulled up to 3.3V and should be configured as open drain)
uint32_t g_ui32MuxOEnGpioList[] = {GPIO_MSPI0_OEn_SEL, GPIO_MSPI1_OEn_SEL0,
    GPIO_MSPI1_OEn_SEL1, GPIO_MSPI2_OEn_SEL0, GPIO_MSPI2_OEn_SEL1, GPIO_MSPI3_OEn_SEL,
    GPIO_IOM0_PC_LB_OEn, GPIO_IOM0_FRAM_PSRAM_OEn, GPIO_IOM1_PC_DISP_OEn, GPIO_IOM1_FRAM_PSRAM_OEn,
    GPIO_IOM2_PC_DISP_OEn, GPIO_IOM2_FRAM_PSRAM_OEn, GPIO_IOM3_PC_FRAM_OEn, GPIO_IOM3_PSRAM_OEn,
    GPIO_IOM4_PC_FRAM_OEn, GPIO_IOM4_PSRAM_OEn, GPIO_IOM5_PC_FRAM_OEn, GPIO_IOM5_PSRAM_OEn,
    GPIO_IOM6_PC_FRAM_OEn, GPIO_IOM6_PSRAM_OEn, GPIO_IOM7_PC_FRAM_OEn, GPIO_IOM7_PSRAM_OEn,
    GPIO_IOS_LB_PC_OEn, GPIO_TRACE_OEn_SEL, GPIO_SWD_OEn_SEL, GPIO_MSPI1_LS_MUX_OEn, GPIO_MSPI2_LS_MUX_OEn};

// list of all mux SEL GPIOS (these should be configured as pushpull outputs)
uint32_t g_ui32MuxSelGpioList[] = {GPIO_IOM4_PC_FRAM_SEL, GPIO_MSPI1_QUAD_SEL, GPIO_MSPI3_PC_SEL,
    GPIO_MSPI1_PC_SEL, GPIO_SDIO1_MUX_SEL1, GPIO_SWD_PC_SEL, GPIO_MSPI2_PC_SEL,
    GPIO_MSPI2_QUAD_SEL, GPIO_IOM1_PC_DISP_SEL, GPIO_SDIO0_MUX_SEL1, GPIO_TRACE_PC_SEL,
    GPIO_IOM6_PC_FRAM_SEL, GPIO_SDIO0_RST_MUX_SEL0, GPIO_SDIO0_RST_MUX_SEL1,
    GPIO_IOM7_PC_FRAM_SEL, GPIO_MSPI0_PC_SEL, GPIO_IOM0_FRAM_PSRAM_SEL, GPIO_SDIO0_MUX_SEL0,
    GPIO_IOM0_PC_LB_SEL, GPIO_IOM2_PC_DISP_SEL, GPIO_IOS_LB_PC_SEL, GPIO_IOM1_FRAM_PSRAM_SEL,
    GPIO_IOM2_FRAM_PSRAM_SEL, GPIO_IOM3_PC_FRAM_SEL, GPIO_IOM5_PC_FRAM_SEL,
    GPIO_SDIO1_RST_MUX_SEL0, GPIO_SDIO1_RST_MUX_SEL1, GPIO_SDIO1_MUX_SEL0, GPIO_UART_SEL1,
    GPIO_UART_SEL0, GPIO_MSPI1_LS_MUX_SEL, GPIO_MSPI2_LS_MUX_SEL};

// Mode button GPIO config
const am_hal_gpio_pincfg_t g_sModeButtonCfg =
{
    .GP.cfg_b.uFuncSel = 3,
    .GP.cfg_b.eIntDir = AM_HAL_GPIO_PIN_INTDIR_HI2LO,
    .GP.cfg_b.eGPInput = AM_HAL_GPIO_PIN_INPUT_ENABLE,
#ifdef DEBUG_ON_AP4P_EVB
    // Apollo4p EVB button GPIO does not have external pull-up
    .GP.cfg_b.ePullup = AM_HAL_GPIO_PIN_PULLUP_12K,
#endif
};

//*****************************************************************************
//
// am_gpio_isr
//
//*****************************************************************************
void am_gpio0_001f_isr()
{
    uint32_t ui32IntStatus;

    //
    // Delay for button debounce
    //
    am_util_delay_ms(500);

    //
    // Clear the GPIO Interrupt (write to clear).
    //
    AM_CRITICAL_BEGIN
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    AM_CRITICAL_END

    g_bButtonPress = true;
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
    am_hal_gpio_output_set(GPIO_LED_MODE_B3);
    am_hal_gpio_output_set(GPIO_LED_MODE_B2);
    am_hal_gpio_output_set(GPIO_LED_MODE_B1);
    am_hal_gpio_output_set(GPIO_LED_MODE_B0);
    am_hal_gpio_output_set(GPIO_IOX_LED);

    am_hal_gpio_pinconfig(GPIO_LED_MODE_B3, am_hal_gpio_pincfg_opendrain);
    am_hal_gpio_pinconfig(GPIO_LED_MODE_B2, am_hal_gpio_pincfg_opendrain);
    am_hal_gpio_pinconfig(GPIO_LED_MODE_B1, am_hal_gpio_pincfg_opendrain);
    am_hal_gpio_pinconfig(GPIO_LED_MODE_B0, am_hal_gpio_pincfg_opendrain);
    am_hal_gpio_pinconfig(GPIO_IOX_LED, am_hal_gpio_pincfg_opendrain);

    //
    // Init Mux OEn GPIOs as open drain since they are pulled up to 3.3V externally
    //
    uint32_t ui32MuxOEnGpioCount = sizeof(g_ui32MuxOEnGpioList) / sizeof(g_ui32MuxOEnGpioList[0]);
    for (uint32_t i = 0; i < ui32MuxOEnGpioCount; i++)
    {
        am_hal_gpio_output_set(g_ui32MuxOEnGpioList[i]); // Intialize to "disabled"
        am_hal_gpio_pinconfig(g_ui32MuxOEnGpioList[i], am_hal_gpio_pincfg_opendrain);
    }

    //
    // Init Mux SEL GPIOs as pushpull since they have no external pull-up resistors
    //
    uint32_t ui32MuxSelGpioCount = sizeof(g_ui32MuxSelGpioList) / sizeof(g_ui32MuxSelGpioList[0]);
    for (uint32_t i = 0; i < ui32MuxSelGpioCount; i++)
    {
        am_hal_gpio_output_clear(g_ui32MuxSelGpioList[i]); // Intialize to 0
        am_hal_gpio_pinconfig(g_ui32MuxSelGpioList[i], am_hal_gpio_pincfg_output);
    }

    //
    // Enable MSPI1/2 Load switch MUX SEL.  which are externally pulled to GND, so should be pushpull
    //
    am_hal_gpio_pinconfig(GPIO_MSPI1_LS_MUX_SEL, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_MSPI2_LS_MUX_SEL, am_hal_gpio_pincfg_output);

    //
    // Except for UART_SEL0/1 which are externally pulled to GND, so should be pushpull
    //
    am_hal_gpio_pinconfig(GPIO_UART_SEL1, am_hal_gpio_pincfg_output);
    am_hal_gpio_pinconfig(GPIO_UART_SEL0, am_hal_gpio_pincfg_output);

    //
    // HUB_I2C_EN must be configured as an input in order to be controlled
    // by automation with pyftdi.
    //
    am_hal_gpio_pinconfig(GPIO_HUB_I2C_EN, am_hal_gpio_pincfg_input);

    //
    // Configure FT_SCL and FT_SDA as inputs (Hi-Z) for now
    //
    am_hal_gpio_pinconfig(GPIO_FT_SCL, am_hal_gpio_pincfg_input);
    am_hal_gpio_pinconfig(GPIO_FT_SDA, am_hal_gpio_pincfg_input);

    //
    // Initialize mode button as Input, interrupt on falling edge
    //
    uint32_t ui32IntStatus;
    uint32_t ui32IntNum = GPIO_CFG_SW;

    //
    // Configure the button pin.
    //
    am_hal_gpio_pinconfig(GPIO_CFG_SW, g_sModeButtonCfg);

    //
    // Clear the GPIO Interrupt (write to clear).
    //
    AM_CRITICAL_BEGIN
    am_hal_gpio_interrupt_irq_status_get(GPIO0_001F_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(GPIO0_001F_IRQn, ui32IntStatus);
    AM_CRITICAL_END

    //
    // Enable the GPIO/button interrupt.
    //
    am_hal_gpio_interrupt_control(AM_HAL_GPIO_INT_CHANNEL_0,
                                  AM_HAL_GPIO_INT_CTRL_INDV_ENABLE,
                                  (void *)&ui32IntNum);

    //
    // Enable GPIO interrupts to the NVIC.
    //
    NVIC_SetPriority(GPIO0_001F_IRQn, AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ(GPIO0_001F_IRQn);

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
// config_swd
//
//*****************************************************************************
void config_swd(swd_options_e eState)
{
    switch (eState)
    {
        case SWD_AP5:
            write_mux_pin(GPIO_SWD_OEn_SEL, 0);
            write_mux_pin(GPIO_SWD_PC_SEL, 0);
            break;
        case SWD_PC:
            write_mux_pin(GPIO_SWD_OEn_SEL, 0);
            write_mux_pin(GPIO_SWD_PC_SEL, 1);
            break;
        default:
            am_util_stdio_printf("ERROR: config_swd - state %d is invalid\n", eState);
            break;
    }
}

//*****************************************************************************
//
// config_trace
//
//*****************************************************************************
void config_trace(trace_options_e eState)
{
    switch (eState)
    {
        case TRACE_AP5:
            write_mux_pin(GPIO_TRACE_OEn_SEL, 0);
            write_mux_pin(GPIO_TRACE_PC_SEL, 0);
            break;
        case TRACE_PC:
            write_mux_pin(GPIO_TRACE_OEn_SEL, 0);
            write_mux_pin(GPIO_TRACE_PC_SEL, 1);
            break;
        default:
            am_util_stdio_printf("ERROR: config_trace - state %d is invalid\n", eState);
            break;
    }
}

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
            case MSPI03_HEX_PSRAM:
                write_mux_pin(GPIO_MSPI0_OEn_SEL, 0);
                write_mux_pin(GPIO_MSPI0_PC_SEL, 0);
                break;
            case MSPI03_PC:
                write_mux_pin(GPIO_MSPI0_OEn_SEL, 0);
                write_mux_pin(GPIO_MSPI0_PC_SEL, 1);
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
            case MSPI03_HEX_PSRAM:
                write_mux_pin(GPIO_MSPI3_OEn_SEL, 0);
                write_mux_pin(GPIO_MSPI3_PC_SEL, 0);
                break;
            case MSPI03_PC:
                write_mux_pin(GPIO_MSPI3_OEn_SEL, 0);
                write_mux_pin(GPIO_MSPI3_PC_SEL, 1);
                break;
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
            case MSPI12_OCTAL_FLASH:
                write_mux_pin(GPIO_MSPI1_OEn_SEL1, 1);
                write_mux_pin(GPIO_MSPI1_OEn_SEL0, 0);
                write_mux_pin(GPIO_MSPI1_PC_SEL, 0);
                write_mux_pin(GPIO_MSPI1_QUAD_SEL, 0);
                write_mux_pin(GPIO_MSPI1_LS_MUX_OEn, 0);
                write_mux_pin(GPIO_MSPI1_LS_MUX_SEL, 0);
                break;
            case MSPI12_QUAD_PSRAM:
                write_mux_pin(GPIO_MSPI1_OEn_SEL1, 0);
                write_mux_pin(GPIO_MSPI1_OEn_SEL0, 1);
                write_mux_pin(GPIO_MSPI1_PC_SEL, 0);
                write_mux_pin(GPIO_MSPI1_QUAD_SEL, 0);
                write_mux_pin(GPIO_MSPI1_LS_MUX_OEn, 0);
                write_mux_pin(GPIO_MSPI1_LS_MUX_SEL, 0);
                break;
            case MSPI12_QUAD_NAND:
                write_mux_pin(GPIO_MSPI1_OEn_SEL1, 0);
                write_mux_pin(GPIO_MSPI1_OEn_SEL0, 1);
                write_mux_pin(GPIO_MSPI1_PC_SEL, 0);
                write_mux_pin(GPIO_MSPI1_QUAD_SEL, 1);
                write_mux_pin(GPIO_MSPI1_LS_MUX_OEn, 0);
                write_mux_pin(GPIO_MSPI1_LS_MUX_SEL, 0);
                break;
            case MSPI12_PC:
                write_mux_pin(GPIO_MSPI1_OEn_SEL1, 1);
                write_mux_pin(GPIO_MSPI1_OEn_SEL0, 0);
                write_mux_pin(GPIO_MSPI1_PC_SEL, 1);
                write_mux_pin(GPIO_MSPI1_QUAD_SEL, 0);
                write_mux_pin(GPIO_MSPI1_LS_MUX_OEn, 0);
                write_mux_pin(GPIO_MSPI1_LS_MUX_SEL, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_mspi12 - state %d is invalid\n", eState);
                break;
        }
    }
    else if ( mspi == 2 )
    {
        switch (eState)
        {
            case MSPI12_OCTAL_FLASH:
                write_mux_pin(GPIO_MSPI2_OEn_SEL1, 1);
                write_mux_pin(GPIO_MSPI2_OEn_SEL0, 0);
                write_mux_pin(GPIO_MSPI2_PC_SEL, 0);
                write_mux_pin(GPIO_MSPI2_QUAD_SEL, 0);
                write_mux_pin(GPIO_MSPI2_LS_MUX_OEn, 0);
                write_mux_pin(GPIO_MSPI2_LS_MUX_SEL, 0);
                break;
            case MSPI12_QUAD_PSRAM:
                write_mux_pin(GPIO_MSPI2_OEn_SEL1, 0);
                write_mux_pin(GPIO_MSPI2_OEn_SEL0, 1);
                write_mux_pin(GPIO_MSPI2_PC_SEL, 0);
                write_mux_pin(GPIO_MSPI2_QUAD_SEL, 0);
                write_mux_pin(GPIO_MSPI2_LS_MUX_OEn, 0);
                write_mux_pin(GPIO_MSPI2_LS_MUX_SEL, 0);
                break;
            case MSPI12_QUAD_NAND:
                write_mux_pin(GPIO_MSPI2_OEn_SEL1, 0);
                write_mux_pin(GPIO_MSPI2_OEn_SEL0, 1);
                write_mux_pin(GPIO_MSPI2_PC_SEL, 0);
                write_mux_pin(GPIO_MSPI2_QUAD_SEL, 1);
                write_mux_pin(GPIO_MSPI2_LS_MUX_OEn, 0);
                write_mux_pin(GPIO_MSPI2_LS_MUX_SEL, 0);
                break;
            case MSPI12_PC:
                write_mux_pin(GPIO_MSPI2_OEn_SEL1, 1);
                write_mux_pin(GPIO_MSPI2_OEn_SEL0, 0);
                write_mux_pin(GPIO_MSPI2_PC_SEL, 1);
                write_mux_pin(GPIO_MSPI2_QUAD_SEL, 0);
                write_mux_pin(GPIO_MSPI2_LS_MUX_OEn, 0);
                write_mux_pin(GPIO_MSPI2_LS_MUX_SEL, 1);
                break;
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
// config_iom
//
//*****************************************************************************
void config_iom(uint8_t iom, iom_options_e eState)
{
    if (iom == 0)
    {
        switch (eState)
        {
            case IOM_IOS_LOOPBACK: // IOM0 Only
                write_mux_pin(GPIO_IOM0_FRAM_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM0_PC_LB_OEn, 0);
                write_mux_pin(GPIO_IOM0_FRAM_PSRAM_SEL, 0);
                write_mux_pin(GPIO_IOM0_PC_LB_SEL, 0);
                write_mux_pin(GPIO_IOS_LB_PC_SEL, 0);
                write_mux_pin(GPIO_IOS_LB_PC_OEn, 0);
                break;
            case IOM_SERIAL_PSRAM:
                write_mux_pin(GPIO_IOM0_FRAM_PSRAM_OEn, 0);
                write_mux_pin(GPIO_IOM0_PC_LB_OEn, 1);
                write_mux_pin(GPIO_IOM0_FRAM_PSRAM_SEL, 0);
                write_mux_pin(GPIO_IOM0_PC_LB_SEL, 0);
                write_mux_pin(GPIO_IOS_LB_PC_SEL, 0);
                write_mux_pin(GPIO_IOS_LB_PC_OEn, 1);
                break;
            case IOM_FRAM:
                write_mux_pin(GPIO_IOM0_FRAM_PSRAM_OEn, 0);
                write_mux_pin(GPIO_IOM0_PC_LB_OEn, 1);
                write_mux_pin(GPIO_IOM0_FRAM_PSRAM_SEL, 1);
                write_mux_pin(GPIO_IOM0_PC_LB_SEL, 0);
                write_mux_pin(GPIO_IOS_LB_PC_SEL, 0);
                write_mux_pin(GPIO_IOS_LB_PC_OEn, 1);
                break;
            case IOM_PC:
                write_mux_pin(GPIO_IOM0_FRAM_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM0_PC_LB_OEn, 0);
                write_mux_pin(GPIO_IOM0_FRAM_PSRAM_SEL, 0);
                write_mux_pin(GPIO_IOM0_PC_LB_SEL, 1);
                write_mux_pin(GPIO_IOS_LB_PC_SEL, 1);
                write_mux_pin(GPIO_IOS_LB_PC_OEn, 0);
                break;
            default:
                am_util_stdio_printf("ERROR: config_iom - iom %d state %d is invalid\n", iom, eState);
        }
    }
    else if (iom == 1)
    {
        switch (eState)
        {
            case IOM_SERIAL_PSRAM:
                write_mux_pin(GPIO_IOM1_FRAM_PSRAM_OEn, 0);
                write_mux_pin(GPIO_IOM1_PC_DISP_OEn, 1);
                write_mux_pin(GPIO_IOM1_FRAM_PSRAM_SEL, 0);
                write_mux_pin(GPIO_IOM1_PC_DISP_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(GPIO_IOM1_FRAM_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM1_PC_DISP_OEn, 0);
                write_mux_pin(GPIO_IOM1_FRAM_PSRAM_SEL, 0);
                write_mux_pin(GPIO_IOM1_PC_DISP_SEL, 0);
                break;
            case IOM_PC:
                write_mux_pin(GPIO_IOM1_FRAM_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM1_PC_DISP_OEn, 0);
                write_mux_pin(GPIO_IOM1_FRAM_PSRAM_SEL, 0);
                write_mux_pin(GPIO_IOM1_PC_DISP_SEL, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_iom - iom %d state %d is invalid\n", iom, eState);
        }
    }
    else if (iom == 2)
    {
        switch (eState)
        {
            case IOM_SERIAL_PSRAM:
                write_mux_pin(GPIO_IOM2_FRAM_PSRAM_OEn, 0);
                write_mux_pin(GPIO_IOM2_PC_DISP_OEn, 1);
                write_mux_pin(GPIO_IOM2_FRAM_PSRAM_SEL, 0);
                write_mux_pin(GPIO_IOM2_PC_DISP_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(GPIO_IOM2_FRAM_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM2_PC_DISP_OEn, 0);
                write_mux_pin(GPIO_IOM2_FRAM_PSRAM_SEL, 0);
                write_mux_pin(GPIO_IOM2_PC_DISP_SEL, 0);
                break;
            case IOM_PC:
                write_mux_pin(GPIO_IOM2_FRAM_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM2_PC_DISP_OEn, 0);
                write_mux_pin(GPIO_IOM2_FRAM_PSRAM_SEL, 0);
                write_mux_pin(GPIO_IOM2_PC_DISP_SEL, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_iom - iom %d state %d is invalid\n", iom, eState);
        }
    }
    else if (iom == 3)
    {
        switch (eState)
        {
            case IOM_SERIAL_PSRAM:
                write_mux_pin(GPIO_IOM3_PSRAM_OEn, 0);
                write_mux_pin(GPIO_IOM3_PC_FRAM_OEn, 1);
                write_mux_pin(GPIO_IOM3_PC_FRAM_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(GPIO_IOM3_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM3_PC_FRAM_OEn, 0);
                write_mux_pin(GPIO_IOM3_PC_FRAM_SEL, 0);
                break;
            case IOM_PC:
                write_mux_pin(GPIO_IOM3_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM3_PC_FRAM_OEn, 0);
                write_mux_pin(GPIO_IOM3_PC_FRAM_SEL, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_iom - iom %d state %d is invalid\n", iom, eState);
        }
    }
    else if (iom == 4)
    {
        switch (eState)
        {
            case IOM_SERIAL_PSRAM:
                write_mux_pin(GPIO_IOM4_PSRAM_OEn, 0);
                write_mux_pin(GPIO_IOM4_PC_FRAM_OEn, 1);
                write_mux_pin(GPIO_IOM4_PC_FRAM_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(GPIO_IOM4_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM4_PC_FRAM_OEn, 0);
                write_mux_pin(GPIO_IOM4_PC_FRAM_SEL, 0);
                break;
            case IOM_PC:
                write_mux_pin(GPIO_IOM4_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM4_PC_FRAM_OEn, 0);
                write_mux_pin(GPIO_IOM4_PC_FRAM_SEL, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_iom - iom %d state %d is invalid\n", iom, eState);
        }
    }
    else if (iom == 5)
    {
        switch (eState)
        {
            case IOM_SERIAL_PSRAM:
                write_mux_pin(GPIO_IOM5_PSRAM_OEn, 0);
                write_mux_pin(GPIO_IOM5_PC_FRAM_OEn, 1);
                write_mux_pin(GPIO_IOM5_PC_FRAM_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(GPIO_IOM5_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM5_PC_FRAM_OEn, 0);
                write_mux_pin(GPIO_IOM5_PC_FRAM_SEL, 0);
                break;
            case IOM_PC:
                write_mux_pin(GPIO_IOM5_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM5_PC_FRAM_OEn, 0);
                write_mux_pin(GPIO_IOM5_PC_FRAM_SEL, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_iom - iom %d state %d is invalid\n", iom, eState);
        }
    }
    else if (iom == 6)
    {
        switch (eState)
        {
            case IOM_SERIAL_PSRAM:
                write_mux_pin(GPIO_IOM6_PSRAM_OEn, 0);
                write_mux_pin(GPIO_IOM6_PC_FRAM_OEn, 1);
                write_mux_pin(GPIO_IOM6_PC_FRAM_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(GPIO_IOM6_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM6_PC_FRAM_OEn, 0);
                write_mux_pin(GPIO_IOM6_PC_FRAM_SEL, 0);
                break;
            case IOM_PC:
                write_mux_pin(GPIO_IOM6_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM6_PC_FRAM_OEn, 0);
                write_mux_pin(GPIO_IOM6_PC_FRAM_SEL, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_iom - iom %d state %d is invalid\n", iom, eState);
        }
    }
    else if (iom == 7)
    {
        switch (eState)
        {
            case IOM_SERIAL_PSRAM:
                write_mux_pin(GPIO_IOM7_PSRAM_OEn, 0);
                write_mux_pin(GPIO_IOM7_PC_FRAM_OEn, 1);
                write_mux_pin(GPIO_IOM7_PC_FRAM_SEL, 0);
                break;
            case IOM_FRAM:
                write_mux_pin(GPIO_IOM7_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM7_PC_FRAM_OEn, 0);
                write_mux_pin(GPIO_IOM7_PC_FRAM_SEL, 0);
                break;
            case IOM_PC:
                write_mux_pin(GPIO_IOM7_PSRAM_OEn, 1);
                write_mux_pin(GPIO_IOM7_PC_FRAM_OEn, 0);
                write_mux_pin(GPIO_IOM7_PC_FRAM_SEL, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_iom - iom %d state %d is invalid\n", iom, eState);
        }
    }
    else
    {
        am_util_stdio_printf("ERROR: config_iom - iom %d is invalid\n", iom);
    }
}

//*****************************************************************************
//
// config_sdio
//
//*****************************************************************************
void config_sdio(uint8_t sdio, sdio_options_e eState)
{
    if (sdio == 0)
    {
        switch (eState)
        {
            case SDIO_eMMC:
                write_mux_pin(GPIO_SDIO0_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO0_MUX_SEL0, 0);
                break;
            case SDIO_EXT:
                write_mux_pin(GPIO_SDIO0_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO0_MUX_SEL0, 1);
                break;
            case SDIO_PC:
                write_mux_pin(GPIO_SDIO0_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO0_MUX_SEL0, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_sdio - sdio %d state %d is invalid\n", sdio, eState);
        }
    }
    else if (sdio == 1)
    {
        switch (eState)
        {
            case SDIO_eMMC:
                write_mux_pin(GPIO_SDIO1_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO1_MUX_SEL0, 0);
                break;
            case SDIO_EXT:
                write_mux_pin(GPIO_SDIO1_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO1_MUX_SEL0, 1);
                break;
            case SDIO_PC:
                write_mux_pin(GPIO_SDIO1_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO1_MUX_SEL0, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_sdio - sdio %d state %d is invalid\n", sdio, eState);
        }
    }
    else
    {
        am_util_stdio_printf("ERROR: config_sdio - sdio %d is invalid\n", sdio);
    }
}

//*****************************************************************************
//
// config_sdio_reset
//
//*****************************************************************************
void config_sdio_reset(uint8_t sdio, sdio_rst_options_e eState)
{
    if (sdio == 0)
    {
        switch (eState)
        {
            case SDIO_RST_OPEN:
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 0);
                break;
            case SDIO_RST_eMMC:
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 1);
                break;
            case SDIO_RST_EXT:
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 0);
                break;
            case SDIO_RST_PC:
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_sdio_reset - reset state %d is invalid\n", eState);
        }
    }
    else if (sdio == 1)
    {
        switch (eState)
        {
            case SDIO_RST_OPEN:
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 0);
                break;
            case SDIO_RST_eMMC:
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 1);
                break;
            case SDIO_RST_EXT:
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 0);
                break;
            case SDIO_RST_PC:
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 1);
                break;
            default:
                am_util_stdio_printf("ERROR: config_sdio_reset - reset state %d is invalid\n", eState);
        }
    }
    else
    {
        am_util_stdio_printf("ERROR: config_sdio_reset - sdio %d is invalid\n", sdio);
    }
}


//*****************************************************************************
//
// config_uart
//
//*****************************************************************************
void config_uart(uart_options_e eState)
{
    switch (eState)
    {
        case UART_HDR_FTDI:
            write_mux_pin(GPIO_UART_SEL1, 0);
            write_mux_pin(GPIO_UART_SEL0, 1);
            break;
        case UART_VCOM:
            write_mux_pin(GPIO_UART_SEL1, 1);
            write_mux_pin(GPIO_UART_SEL0, 0);
            break;
        case UART_PC:
            write_mux_pin(GPIO_UART_SEL1, 1);
            write_mux_pin(GPIO_UART_SEL0, 1);
            break;
        default:
            am_util_stdio_printf("ERROR: config_uart - uart state %d is invalid\n", eState);
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
    config_swd(g_swdState);
    config_trace(g_traceState);
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
    config_sdio_reset(0, g_sdio0RstState);
    config_sdio(1, g_sdio1State);
    config_sdio_reset(1, g_sdio1RstState);
    config_uart(g_uartState);
}

//*****************************************************************************
//
// led_on(led_gpio): turn on the LED connected to GPIO # led_gpio
//
//*****************************************************************************
void led_on(uint32_t led_gpio)
{
    am_hal_gpio_output_clear(led_gpio);
}

//*****************************************************************************
//
// led_off(led_gpio): turn off the LED connected to GPIO # led_gpio
//
//*****************************************************************************
void led_off(uint32_t led_gpio)
{
    am_hal_gpio_output_set(led_gpio);
}

//*****************************************************************************
//
// update_leds(): Update the bank of LEDs to reflect the current mode
//
//*****************************************************************************
void update_leds(void)
{
    uint32_t bit_mask[] = {0x1, 0x2, 0x4, 0x8};
    uint32_t led_gpio[] = {
                            GPIO_LED_MODE_B0, GPIO_LED_MODE_B1,
                            GPIO_LED_MODE_B2, GPIO_LED_MODE_B3
                          };
    uint32_t num_leds = 4;
#ifdef DEBUG_ON_AP4P_EVB
    num_leds = 3;
#endif

    for ( uint32_t i = 0; i < num_leds; i++)
    {
        if (g_mode & bit_mask[i])
        {
            led_on(led_gpio[i]);
        }
        else
        {
            led_off(led_gpio[i]);
        }
    }

}

//*****************************************************************************
//
// blink_iox_version(): Blink IOX LED to indicate firmware version
//
//*****************************************************************************
void blink_iox_version(void)
{
    for ( uint32_t i = 0; i < IOX_FW_REV; i++)
    {
        led_on(GPIO_IOX_LED);
        am_util_delay_ms(200);
        led_off(GPIO_IOX_LED);
        am_util_delay_ms(200);
    }
}

//*****************************************************************************
//
// update_tornado_config(): Update Tornado board IOX mode if g_configTornadoBoard is true
//
//*****************************************************************************
void update_tornado_config(void)
{
    if (g_configTornadoBoard)
    {
        //
        // Config HUB_I2C_EN as an output and set to 0 to disable the
        // FTDI I2C
        //
        am_hal_gpio_output_clear(GPIO_HUB_I2C_EN);
        am_hal_gpio_pinconfig(GPIO_HUB_I2C_EN, am_hal_gpio_pincfg_output);

        //
        // Config FT_SCL as an output and toggle the pin to control
        // the Tornado board IOX mode
        //
        am_hal_gpio_pinconfig(GPIO_FT_SCL, am_hal_gpio_pincfg_output);
        am_hal_gpio_state_write(GPIO_FT_SCL, AM_HAL_GPIO_OUTPUT_SET);
        am_util_delay_ms(200);
        am_hal_gpio_state_write(GPIO_FT_SCL, AM_HAL_GPIO_OUTPUT_CLEAR);
        am_util_delay_ms(200);
        am_hal_gpio_state_write(GPIO_FT_SCL, AM_HAL_GPIO_OUTPUT_SET);

    }
    else
    {
        //
        // Config HUB_I2C_EN and FT_SCL as input so that it can be controlled
        // externally by the pyftdi automation
        //
        am_hal_gpio_pinconfig(GPIO_HUB_I2C_EN, am_hal_gpio_pincfg_input);
        am_hal_gpio_pinconfig(GPIO_FT_SCL, am_hal_gpio_pincfg_input);
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
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Do Not configure the board for low power operation!
    // Doing so will brick the chip as it is not configured
    // for simobuck operation.
    //
    //am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_debug_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo5 Engineering Board IOX\n");
    am_util_stdio_printf("Firmware version: %d\n\n", IOX_FW_REV);
#ifdef DEBUG_ON_AP4P_EVB // cycle through 7 modes if debugging on Apollo4P EVB
    am_util_stdio_printf("WARNING: Running in Apollo4p EVB Debug Mode\n");
#endif

    gpio_init(); // Initialize all GPIOs

    blink_iox_version();

    //
    // Enable interrupts to the core.
    //
    am_hal_interrupt_master_enable();

    //
    // Loop forever waiting for button press
    //
    while (1)
    {
        if (g_bButtonPress)
        {
            g_bButtonPress = false;
#ifdef DEBUG_ON_AP4P_EVB // cycle through 7 modes if debugging on Apollo4P EVB
            g_mode = (g_mode + 1) & 0x7; // mode is limited to 3 bits
#else
            g_mode = (g_mode + 1) & 0xf; // mode is limited to 4 bits
#endif
            if (g_mode == 0)
            {
                g_mode++; // don't allow mode 0 (LEDs would all be off)
            }
            am_util_stdio_printf("Button pressed - new mode is %d\n", g_mode);
            switch (g_mode)
            {
                case 1:
                    am_util_stdio_printf("Mode 1 - GSW + EC1 Display\n");
                    g_swdState = SWD_AP5;
                    g_traceState = TRACE_AP5;
                    g_mspi0State = MSPI03_HEX_PSRAM;
                    g_mspi3State = MSPI03_PC; // EC1: 1.2V PSRAM or Winbond
                    g_mspi1State = MSPI12_OCTAL_FLASH;
                    g_mspi2State = MSPI12_PC; // EC1: AMOLED QSPI Display
                    g_iom0State = IOM_PC; // EC1: Touch ALS
                    g_iom1State = IOM_PC; // fly-wire to AP4EVB for sensor hub test
                    g_iom2State = IOM_SERIAL_PSRAM;
                    g_iom3State = IOM_SERIAL_PSRAM;
                    g_iom4State = IOM_SERIAL_PSRAM;
                    g_iom5State = IOM_SERIAL_PSRAM;
                    g_iom6State = IOM_SERIAL_PSRAM;
                    g_iom7State = IOM_SERIAL_PSRAM;
                    g_uartState = UART_HDR_FTDI;
                    g_sdio0State = SDIO_eMMC;
                    g_sdio0RstState = SDIO_RST_OPEN; // NC on EB
                    g_sdio1State = SDIO_EXT;
                    g_sdio1RstState = SDIO_RST_OPEN; // NC on EB
                    g_configTornadoBoard = false;
                    break;
                case 2:
                    am_util_stdio_printf("Mode 2 - SWD + VCOM\n");
                    g_swdState = SWD_AP5;
                    g_traceState = TRACE_PC;
                    g_mspi0State = MSPI03_PC;
                    g_mspi3State = MSPI03_PC;
                    g_mspi1State = MSPI12_PC;
                    g_mspi2State = MSPI12_PC;
                    g_iom0State = IOM_PC;
                    g_iom1State = IOM_PC;
                    g_iom2State = IOM_PC;
                    g_iom3State = IOM_PC;
                    g_iom4State = IOM_PC;
                    g_iom5State = IOM_PC;
                    g_iom6State = IOM_PC;
                    g_iom7State = IOM_PC;
                    g_uartState = UART_VCOM;
                    g_sdio0State = SDIO_PC;
                    g_sdio0RstState = SDIO_RST_PC; // NC on EB
                    g_sdio1State = SDIO_PC;
                    g_sdio1RstState = SDIO_RST_PC; // NC on EB
                    g_configTornadoBoard = false;
                    break;
                case 3:
                    am_util_stdio_printf("Mode 3 - MSPI + I2S + IOM + eMMC\n");
                    am_util_stdio_printf("Details: MSPI0/3 Hex PSRAM + MSPI1 Octal Flash + MSPI2 Quad PSRAM + IOX LB + IOM1-6 Serial PSRAM + IOM7 FRAM + eMMC0/1\n");
                    g_swdState = SWD_AP5;
                    g_traceState = TRACE_AP5;
                    g_mspi0State = MSPI03_HEX_PSRAM;
                    g_mspi3State = MSPI03_HEX_PSRAM;
                    g_mspi1State = MSPI12_OCTAL_FLASH;
                    g_mspi2State = MSPI12_QUAD_PSRAM;
                    g_iom0State = IOM_IOS_LOOPBACK;
                    g_iom1State = IOM_SERIAL_PSRAM;
                    g_iom2State = IOM_SERIAL_PSRAM;
                    g_iom3State = IOM_SERIAL_PSRAM;
                    g_iom4State = IOM_SERIAL_PSRAM;
                    g_iom5State = IOM_SERIAL_PSRAM;
                    g_iom6State = IOM_SERIAL_PSRAM;
                    g_iom7State = IOM_SERIAL_PSRAM;
                    g_uartState = UART_HDR_FTDI;
                    g_sdio0State = SDIO_eMMC;
                    g_sdio0RstState = SDIO_RST_eMMC; // NC on EB
                    g_sdio1State = SDIO_eMMC;
                    g_sdio1RstState = SDIO_RST_eMMC; // NC on EB
                    g_configTornadoBoard = false;
                    break;
                case 4:
                    am_util_stdio_printf("Mode 4 - MSPI + IOM + WiFi\n");
                    am_util_stdio_printf("Details: MSPI0/3 Hex PSRAM + MSPI1 NAND Flash + MSPI2 Octal Flash + IOM0-7 Serial PSRAM + SDIO0/1 WiFi\n");
                    g_swdState = SWD_AP5;
                    g_traceState = TRACE_AP5;
                    g_mspi0State = MSPI03_HEX_PSRAM;
                    g_mspi3State = MSPI03_HEX_PSRAM;
                    g_mspi1State = MSPI12_QUAD_NAND;
                    g_mspi2State = MSPI12_OCTAL_FLASH;
                    g_iom0State = IOM_FRAM;
                    g_iom1State = IOM_FRAM;
                    g_iom2State = IOM_FRAM;
                    g_iom3State = IOM_FRAM;
                    g_iom4State = IOM_FRAM;
                    g_iom5State = IOM_FRAM;
                    g_iom6State = IOM_FRAM;
                    g_iom7State = IOM_FRAM;
                    g_uartState = UART_VCOM;
                    g_sdio0State = SDIO_EXT;
                    g_sdio0RstState = SDIO_RST_EXT; // NC on EB
                    g_sdio1State = SDIO_EXT;
                    g_sdio1RstState = SDIO_RST_EXT; // NC on EB
                    g_configTornadoBoard = false;
                    break;
                case 5:
                    am_util_stdio_printf("Mode 5 - EB + Tornado Card\n");
                    am_util_stdio_printf("Details: MSPI0/MSPI1/MSPI2/MSPI3/IOM0/IOM3 to Tornado + IOM1 Open + IOM2,4-7 Serial PSRAM + SDIO0 eMMC0 + SDIO1 WiFi\n");
                    g_swdState = SWD_AP5;
                    g_traceState = TRACE_PC;
                    g_mspi0State = MSPI03_PC;
                    g_mspi3State = MSPI03_PC;
                    g_mspi1State = MSPI12_OCTAL_FLASH;
                    g_mspi2State = MSPI12_PC;
                    g_iom0State = IOM_PC;
                    g_iom1State = IOM_PC;
                    g_iom2State = IOM_SERIAL_PSRAM;
                    g_iom3State = IOM_PC;
                    g_iom4State = IOM_SERIAL_PSRAM;
                    g_iom5State = IOM_SERIAL_PSRAM;
                    g_iom6State = IOM_SERIAL_PSRAM;
                    g_iom7State = IOM_SERIAL_PSRAM;
                    g_uartState = UART_PC;
                    g_sdio0State = SDIO_eMMC;
                    g_sdio0RstState = SDIO_RST_eMMC; // NC on EB
                    g_sdio1State = SDIO_EXT;
                    g_sdio1RstState = SDIO_RST_EXT; // NC on EB
                    g_configTornadoBoard = true;
                    break;
                case 6:
                    am_util_stdio_printf("Mode 6 - EB + Tornado Card\n");
                    am_util_stdio_printf("Details: MSPI0 to EB + MSPI1/MSPI2/MSPI3/IOM0/IOM3 to Tornado + IOM1 Open + IOM2,4-7 Serial PSRAM + SDIO0 eMMC0 + SDIO1 WiFi\n");
                    g_swdState = SWD_AP5;
                    g_traceState = TRACE_PC;
                    g_mspi0State = MSPI03_HEX_PSRAM;
                    g_mspi3State = MSPI03_HEX_PSRAM;
                    g_mspi1State = MSPI12_OCTAL_FLASH;
                    g_mspi2State = MSPI12_PC;
                    g_iom0State = IOM_PC;
                    g_iom1State = IOM_PC;
                    g_iom2State = IOM_SERIAL_PSRAM;
                    g_iom3State = IOM_PC;
                    g_iom4State = IOM_SERIAL_PSRAM;
                    g_iom5State = IOM_SERIAL_PSRAM;
                    g_iom6State = IOM_SERIAL_PSRAM;
                    g_iom7State = IOM_SERIAL_PSRAM;
                    g_uartState = UART_PC;
                    g_sdio0State = SDIO_eMMC;
                    g_sdio0RstState = SDIO_RST_eMMC; // NC on EB
                    g_sdio1State = SDIO_EXT;
                    g_sdio1RstState = SDIO_RST_EXT; // NC on EB
                    g_configTornadoBoard = true;
                    break;
                case 7:
                    am_util_stdio_printf("Mode 7 - Tornado Card\n");
                    am_util_stdio_printf("Details: MSPI0/MSPI2/MSPI3/IOM0/EMMC to Tornado\n");
                    g_swdState = SWD_AP5;
                    g_traceState = TRACE_PC;
                    g_mspi0State = MSPI03_PC;
                    g_mspi3State = MSPI03_PC;
                    g_mspi1State = MSPI12_PC;
                    g_mspi2State = MSPI12_PC;
                    g_iom0State = IOM_PC;
                    g_iom1State = IOM_PC;
                    g_iom2State = IOM_PC;
                    g_iom3State = IOM_PC;
                    g_iom4State = IOM_PC;
                    g_iom5State = IOM_PC;
                    g_iom6State = IOM_PC;
                    g_iom7State = IOM_PC;
                    g_uartState = UART_VCOM;
                    g_sdio0State = SDIO_PC;
                    g_sdio0RstState = SDIO_RST_PC; // NC on EB
                    g_sdio1State = SDIO_PC;
                    g_sdio1RstState = SDIO_RST_PC; // NC on EB
                    g_configTornadoBoard = true;
                    break;
                case 8:
                    am_util_stdio_printf("Mode 8 - Tornado Card\n");
                    am_util_stdio_printf("Details: MSPI0/MSPI2/MSPI3/IOM0/EMMC to Tornado\n");
                    g_swdState = SWD_AP5;
                    g_traceState = TRACE_PC;
                    g_mspi0State = MSPI03_PC;
                    g_mspi3State = MSPI03_PC;
                    g_mspi1State = MSPI12_PC;
                    g_mspi2State = MSPI12_PC;
                    g_iom0State = IOM_PC;
                    g_iom1State = IOM_PC;
                    g_iom2State = IOM_PC;
                    g_iom3State = IOM_PC;
                    g_iom4State = IOM_PC;
                    g_iom5State = IOM_PC;
                    g_iom6State = IOM_PC;
                    g_iom7State = IOM_PC;
                    g_uartState = UART_HDR_FTDI;
                    g_sdio0State = SDIO_PC;
                    g_sdio0RstState = SDIO_RST_PC; // NC on EB
                    g_sdio1State = SDIO_PC;
                    g_sdio1RstState = SDIO_RST_PC; // NC on EB
                    g_configTornadoBoard = true;
                    break;
                case 9:
                    am_util_stdio_printf("Mode 9 - MSPI + IOM + WiFi\n");
                    am_util_stdio_printf("Details: MSPI0/3 Hex PSRAM + MSPI1 Quad Psram + MSPI2 Octal Flash + IOM0 FRAM + SDIO0/1 WiFi\n");
                    g_swdState = SWD_AP5;
                    g_traceState = TRACE_AP5;
                    g_mspi0State = MSPI03_HEX_PSRAM;
                    g_mspi3State = MSPI03_HEX_PSRAM;
                    g_mspi1State = MSPI12_QUAD_PSRAM;
                    g_mspi2State = MSPI12_OCTAL_FLASH;
                    g_iom0State = IOM_FRAM;
                    g_iom1State = IOM_PC;
                    g_iom2State = IOM_PC;
                    g_iom3State = IOM_PC;
                    g_iom4State = IOM_PC;
                    g_iom5State = IOM_PC;
                    g_iom6State = IOM_PC;
                    g_iom7State = IOM_PC;
                    g_uartState = UART_VCOM;
                    g_sdio0State = SDIO_EXT;
                    g_sdio0RstState = SDIO_RST_EXT; // NC on EB
                    g_sdio1State = SDIO_EXT;
                    g_sdio1RstState = SDIO_RST_EXT; // NC on EB
                    g_configTornadoBoard = false;
                    break;
                case 10:
                    am_util_stdio_printf("Mode 10 - Raw GPIO\n");
                    am_util_stdio_printf("Details: Minimum system, pinout all GPIOs\n");
                    g_swdState = SWD_AP5;
                    g_traceState = TRACE_PC;
                    g_mspi0State = MSPI03_PC;
                    g_mspi3State = MSPI03_PC;
                    g_mspi1State = MSPI12_PC;
                    g_mspi2State = MSPI12_PC;
                    g_iom0State = IOM_PC;
                    g_iom1State = IOM_PC;
                    g_iom2State = IOM_PC;
                    g_iom3State = IOM_PC;
                    g_iom4State = IOM_PC;
                    g_iom5State = IOM_PC;
                    g_iom6State = IOM_PC;
                    g_iom7State = IOM_PC;
                    g_uartState = UART_PC;
                    g_sdio0State = SDIO_PC;
                    g_sdio0RstState = SDIO_RST_PC; // NC on EB
                    g_sdio1State = SDIO_PC;
                    g_sdio1RstState = SDIO_RST_PC; // NC on EB
                    g_configTornadoBoard = false;
                    break;
                //
                // Placeholders for new modes
                //
#if 0 // uncomment cases as new modes are defined

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
                    am_util_stdio_printf("Undefined Mode - resetting\n");
                    g_mode = 0; // reset mode to 0 if not defined
                    g_bButtonPress = true;
                    g_configTornadoBoard = true;
                    break;
            };
            update_iox_state(); // update iox pins based on new state variable settings
            update_leds();
            update_tornado_config();
        }
    }
}
