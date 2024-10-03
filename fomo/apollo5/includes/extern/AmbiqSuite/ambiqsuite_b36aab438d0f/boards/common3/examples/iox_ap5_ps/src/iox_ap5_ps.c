//*****************************************************************************
//
//! @file iox_ap5_ps.c
//!
//! @brief IOX code for Apollo5 Presilicon Board (a.k.a. Turbo)
//!
//! Purpose: Processes button press to cycle mux setting through different
//!     modes.  The current mode is indicated by a bank of 4 LEDs.  This code
//!     can be tested on an Apollo3 EVB by uncommenting DEBUG_ON_AP3_EVB in
//!     iox_ap5_ps.h
//!
//! Printing takes place over the ITM at 1M Baud.  The SWO pin on this board
//! is GP45.  If using Apollo3 EVB to debug, SWO pin is 41.
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
#include "iox_ap5_ps.h"

#define DEBUG_OUTPUT_EN  // Uncomment to enable debug output

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
uint32_t g_mode = 0; // variable to track mode switch setting
volatile bool g_bButtonPress = true; // set true for initial mode config

// pin group configurations (default to PC settings)
mspi16_options_e g_mspi16State = MSPI16_PC;
sdio_options_e g_sdio0State = SDIO_PC;
sdio_options_e g_sdio1State = SDIO_UNUSED;
bool g_bSdioRstCtrl = true;
mspi8_options_e g_mspi8State = MSPI8_PC;
pdm_options_e g_pdm0State = PDM_PC;
pdm_options_e g_pdm1State = PDM_PC;
iom_options_e g_iom0State = IOM_PC;
iom_options_e g_iom1State = IOM_PC;
uart_options_e g_uartState = UART_HDR_FTDI;
disp_options_e g_dispState = DISP_PC;

// list of all mux sel GPIOs
uint32_t g_ui32MuxGpioList[] = {GPIO_PDM0_UART_MUX_SEL0, GPIO_PDM0_UART_MUX_SEL1,
    GPIO_PDM0_UART_MUX_SEL2, GPIO_PDM1_UART_MUX_SEL0, GPIO_PDM1_UART_MUX_SEL1,
    GPIO_DISP_MUX_SEL, GPIO_UART_SRC_SEL, GPIO_MSPIX8_MUX_SEL, GPIO_MSPIX16_MUX_SEL,
    GPIO_PSRAMX16_MUX_SEL, GPIO_IOM0_MUX_SEL0, GPIO_IOM0_MUX_SEL1, GPIO_IOM1_MUX_SEL0,
    GPIO_IOM1_MUX_SEL1, GPIO_SDIO0_MUX_SEL0, GPIO_SDIO0_MUX_SEL1, GPIO_SDIO1_MUX_SEL0,
    GPIO_SDIO0_RST_MUX_SEL0, GPIO_SDIO0_RST_MUX_SEL1, GPIO_SDIO1_RST_MUX_SEL0,
    GPIO_SDIO1_RST_MUX_SEL1, GPIO_IOM_MUX_SEL0, GPIO_IOM_MUX_SEL1, GPIO_IOM_MUX_SEL2,
    GPIO_IOM_MUX_SEL3, GPIO_IOM_MUX_SEL4, GPIO_IOM_MUX_SEL5, GPIO_IOM_MUX_SEL6,
    GPIO_IOM_MUX_SEL7, GPIO_SDIO_RSTn_I2S_MUX_SEL};


// ITM serial wire output
const am_hal_gpio_pincfg_t g_IOX_GPIO_ITM_SWO =
{
    .uFuncSel            = AM_HAL_PIN_45_SWO,
    .eDriveStrength      = AM_HAL_GPIO_PIN_DRIVESTRENGTH_2MA
};

// Mode button GPIO config
const am_hal_gpio_pincfg_t g_sModeButtonCfg =
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
    am_util_delay_ms(500);

    uint64_t ui64Status;

    am_hal_gpio_interrupt_status_get(false, &ui64Status);
    am_hal_gpio_interrupt_clear(ui64Status);

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

    am_hal_gpio_pinconfig(GPIO_LED_MODE_B3, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pinconfig(GPIO_LED_MODE_B2, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pinconfig(GPIO_LED_MODE_B1, g_AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_pinconfig(GPIO_LED_MODE_B0, g_AM_HAL_GPIO_OUTPUT);

    //
    // Init Muxes as outputs
    //
    uint32_t ui32MuxGpioCount = sizeof(g_ui32MuxGpioList) / sizeof(g_ui32MuxGpioList[0]);
    for (uint32_t i = 0; i < ui32MuxGpioCount; i++)
    {
        am_hal_gpio_pinconfig(g_ui32MuxGpioList[i], g_AM_HAL_GPIO_OUTPUT);
    }

    am_hal_gpio_pinconfig(GPIO_AP3_SCL, g_AM_HAL_GPIO_INPUT);
    am_hal_gpio_pinconfig(GPIO_AP3_SDA, g_AM_HAL_GPIO_INPUT);
    //
    // Initialize mode button as Input, interrupt on falling edge
    //
    am_hal_gpio_pinconfig(GPIO_CFG_SW, g_sModeButtonCfg);
    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_interrupt_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, GPIO_CFG_SW));
    am_hal_gpio_interrupt_enable(AM_HAL_GPIO_MASKBIT(pGpioIntMask, GPIO_CFG_SW));

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
// config_mspi16
//
//*****************************************************************************
void config_mspi16(mspi16_options_e eState)
{
    switch (eState)
    {
        case MSPI16_AP_PSRAM:
            write_mux_pin(GPIO_MSPIX16_MUX_SEL, 0);
            write_mux_pin(GPIO_PSRAMX16_MUX_SEL, 0);
            break;
        case MSPI16_WB_PSRAM:
            write_mux_pin(GPIO_MSPIX16_MUX_SEL, 0);
            write_mux_pin(GPIO_PSRAMX16_MUX_SEL, 1);
            break;
        case MSPI16_PC:
            write_mux_pin(GPIO_MSPIX16_MUX_SEL, 1);
            write_mux_pin(GPIO_PSRAMX16_MUX_SEL, 0); // this is a "don't care"
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
// config_sdio
//
// uint8_t sdio: SDIO # (0 or 1)
// sdio_options_e eState: SDIO state (SDIO_EMMC, SDIO_EXT, SDIO_PC, or SDIO_UNUSED*)
//                  *SDIO_UNUSED is valid for SDIO1 only
// bool rstCtrl: if true, GP61/62 will be routed to SDIOx_RSTn signals
//               if false, GP61/62 will be routed to ALS_TOUCH I2C signals
//
//*****************************************************************************
void config_sdio(uint8_t sdio, sdio_options_e eState, bool rstCtrl)
{
    if (sdio == 0) // config SDIO0
    {
        switch (eState)
        {
            case SDIO_eMMC:
                //write_mux_pin(GPIO_SDIO0_MUX_SEL1, 0); // this is a don't care
                write_mux_pin(GPIO_SDIO0_MUX_SEL0, 0);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 1);
                break;
            case SDIO_EXT:
                write_mux_pin(GPIO_SDIO0_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO0_MUX_SEL0, 1);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 0);
                break;
            case SDIO_PC:
                write_mux_pin(GPIO_SDIO0_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO0_MUX_SEL0, 1);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 1);
                break;
            default:
                break;
        }
        if (rstCtrl)
        {
            // GP61/62 will control SDIO RST lines
            write_mux_pin(GPIO_SDIO_RSTn_I2S_MUX_SEL, 0);
        }
        else
        {
            // GP61/62 will connect to Touch ALS signals
            write_mux_pin(GPIO_SDIO_RSTn_I2S_MUX_SEL, 1);
            write_mux_pin(GPIO_SDIO0_RST_MUX_SEL1, 0);
            write_mux_pin(GPIO_SDIO0_RST_MUX_SEL0, 0);
        }
    }
    else // config SDIO1
    {
        switch (eState)
        {
            case SDIO_eMMC:
                write_mux_pin(GPIO_SDIO1_MUX_SEL0, 0);
                write_mux_pin(GPIO_SDIO1_RST_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO1_RST_MUX_SEL0, 1);
                break;
            case SDIO_EXT:
                write_mux_pin(GPIO_SDIO1_MUX_SEL0, 1);
                write_mux_pin(GPIO_SDIO1_RST_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO1_RST_MUX_SEL0, 0);
                break;
            case SDIO_PC:
                write_mux_pin(GPIO_SDIO1_MUX_SEL0, 0);
                write_mux_pin(GPIO_SDIO1_RST_MUX_SEL1, 1);
                write_mux_pin(GPIO_SDIO1_RST_MUX_SEL0, 1);
                break;
            case SDIO_UNUSED: // SDIO1 only - when not using SDIO1
                // Default to reset open if not using SDIO1
                write_mux_pin(GPIO_SDIO1_RST_MUX_SEL1, 0);
                write_mux_pin(GPIO_SDIO1_RST_MUX_SEL0, 0);
                break;
            default:
                break;
        }
        if (rstCtrl)
        {
            // GP61/62 will control SDIO RST lines
            write_mux_pin(GPIO_SDIO_RSTn_I2S_MUX_SEL, 0);
        }
        else
        {
            // GP61/62 will connect to Touch ALS signals
            write_mux_pin(GPIO_SDIO_RSTn_I2S_MUX_SEL, 1);
            write_mux_pin(GPIO_SDIO1_RST_MUX_SEL1, 0);
            write_mux_pin(GPIO_SDIO1_RST_MUX_SEL0, 0);
        }
    }
}

//*****************************************************************************
//
// config_mspi8
//
//*****************************************************************************
void config_mspi8(mspi8_options_e eState)
{
    switch (eState)
    {
        case MSPI8_FLASH:
            write_mux_pin(GPIO_MSPIX8_MUX_SEL, 0);
            break;
        case MSPI8_PC:
            write_mux_pin(GPIO_MSPIX8_MUX_SEL, 1);
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
// config_pdm
//
//*****************************************************************************
void config_pdm(uint8_t pdm, pdm_options_e eState)
{
    if (pdm == 0) // config PDM0
    {
        switch (eState)
        {
            case PDM_OPEN:
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL2, 0); // this is a don't care
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL1, 0);
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL0, 0);
                break;
            case PDM_HDR:
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL2, 0); // this is a don't care
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL1, 0);
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL0, 1);
                break;
            case PDM_UART:
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL2, 0); // this is a don't care
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL1, 1);
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL0, 0);
                break;
            case PDM_PC:
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL2, 0);
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL1, 1);
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL0, 1);
                break;
            case PDM_SDIO1: // PDM0 only
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL2, 1);
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL1, 1);
                write_mux_pin(GPIO_PDM0_UART_MUX_SEL0, 1);
                break;
            default:
                break;
        }
    }
    else // config PDM1
    {
        switch (eState)
        {
            case PDM_OPEN:
                write_mux_pin(GPIO_PDM1_UART_MUX_SEL1, 0);
                write_mux_pin(GPIO_PDM1_UART_MUX_SEL0, 0);
                break;
            case PDM_HDR:
                write_mux_pin(GPIO_PDM1_UART_MUX_SEL1, 0);
                write_mux_pin(GPIO_PDM1_UART_MUX_SEL0, 1);
                break;
            case PDM_UART:
                write_mux_pin(GPIO_PDM1_UART_MUX_SEL1, 1);
                write_mux_pin(GPIO_PDM1_UART_MUX_SEL0, 0);
                break;
            case PDM_PC:
                write_mux_pin(GPIO_PDM1_UART_MUX_SEL1, 1);
                write_mux_pin(GPIO_PDM1_UART_MUX_SEL0, 1);
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
            case IOM_OPEN:
                write_mux_pin(GPIO_IOM0_MUX_SEL1, 0);
                write_mux_pin(GPIO_IOM0_MUX_SEL0, 0);
                break;
            case IOM_PSRAM:
                write_mux_pin(GPIO_IOM0_MUX_SEL1, 0);
                write_mux_pin(GPIO_IOM0_MUX_SEL0, 1);
                break;
            case IOM_I2S_HDR:
                write_mux_pin(GPIO_IOM0_MUX_SEL1, 1);
                write_mux_pin(GPIO_IOM0_MUX_SEL0, 0);
                break;
            case IOM_IOS_LB:
                write_mux_pin(GPIO_IOM0_MUX_SEL1, 1);
                write_mux_pin(GPIO_IOM0_MUX_SEL0, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL3, 0);
                write_mux_pin(GPIO_IOM_MUX_SEL2, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL1, 0);
                write_mux_pin(GPIO_IOM_MUX_SEL0, 1);
                break;
            case IOM_SDIO1:
                write_mux_pin(GPIO_IOM0_MUX_SEL1, 1);
                write_mux_pin(GPIO_IOM0_MUX_SEL0, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL3, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL2, 0);
                write_mux_pin(GPIO_IOM_MUX_SEL1, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL0, 0);
                break;
            case IOM_PC:
                write_mux_pin(GPIO_IOM0_MUX_SEL1, 1);
                write_mux_pin(GPIO_IOM0_MUX_SEL0, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL3, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL2, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL1, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL0, 1);
                break;
            default:
                break;
        }
    }
    else // config IOM1
    {
        switch (eState)
        {
            case IOM_OPEN:
                write_mux_pin(GPIO_IOM1_MUX_SEL1, 0);
                write_mux_pin(GPIO_IOM1_MUX_SEL0, 0);
                break;
            case IOM_PSRAM:
                write_mux_pin(GPIO_IOM1_MUX_SEL1, 0);
                write_mux_pin(GPIO_IOM1_MUX_SEL0, 1);
                break;
            case IOM_I2S_HDR:
                write_mux_pin(GPIO_IOM1_MUX_SEL1, 1);
                write_mux_pin(GPIO_IOM1_MUX_SEL0, 0);
                break;
            case IOM_IOS_LB:
                write_mux_pin(GPIO_IOM1_MUX_SEL1, 1);
                write_mux_pin(GPIO_IOM1_MUX_SEL0, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL7, 0);
                write_mux_pin(GPIO_IOM_MUX_SEL6, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL5, 0);
                write_mux_pin(GPIO_IOM_MUX_SEL4, 1);
                break;
            case IOM_SDIO1:
                write_mux_pin(GPIO_IOM1_MUX_SEL1, 1);
                write_mux_pin(GPIO_IOM1_MUX_SEL0, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL7, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL6, 0);
                write_mux_pin(GPIO_IOM_MUX_SEL5, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL4, 0);
                break;
            case IOM_PC:
                write_mux_pin(GPIO_IOM1_MUX_SEL1, 1);
                write_mux_pin(GPIO_IOM1_MUX_SEL0, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL7, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL6, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL5, 1);
                write_mux_pin(GPIO_IOM_MUX_SEL4, 1);
                break;
            default:
                break;
        }
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
            write_mux_pin(GPIO_UART_SRC_SEL, 0);
            break;
        case UART_VCOM:
            write_mux_pin(GPIO_UART_SRC_SEL, 1);
            break;
        default:
            break;
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
        case DISP_OB_DISPLAY:
            write_mux_pin(GPIO_DISP_MUX_SEL, 0);
            break;
        case DISP_PC:
            write_mux_pin(GPIO_DISP_MUX_SEL, 1);
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
    config_mspi16(g_mspi16State);
    config_sdio(0, g_sdio0State, g_bSdioRstCtrl);
    config_sdio(1, g_sdio1State, g_bSdioRstCtrl);
    config_mspi8(g_mspi8State);
    config_pdm(0, g_pdm0State);
    config_pdm(1, g_pdm1State);
    config_iom(0, g_iom0State);
    config_iom(1, g_iom1State);
    config_uart(g_uartState);
    config_disp(g_dispState);
}

//*****************************************************************************
//
// led_on(led_gpio): turn on the LED connected to GPIO # led_gpio
//
//*****************************************************************************
void led_on(uint32_t led_gpio)
{
#ifdef DEBUG_ON_AP3_EVB // EVB LEDs are opposite polarity
    am_hal_gpio_output_set(led_gpio);
#else
    am_hal_gpio_output_clear(led_gpio);
#endif
}

//*****************************************************************************
//
// led_off(led_gpio): turn off the LED connected to GPIO # led_gpio
//
//*****************************************************************************
void led_off(uint32_t led_gpio)
{
#ifdef DEBUG_ON_AP3_EVB // EVB LEDs are opposite polarity
    am_hal_gpio_output_clear(led_gpio);
#else
    am_hal_gpio_output_set(led_gpio);
#endif
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
    am_bsp_itm_printf_enable();
#ifdef DEBUG_ON_AP3_EVB
    // NOTE: You can test ITM output on GP45 using the Apollo3 EVB by
    // uncommenting this code and fly-wiring GP45 to GP41
    //am_hal_gpio_pinconfig(GPIO_AP3_SWO, g_IOX_GPIO_ITM_SWO); // Enable ITM output on GP45
    //am_hal_gpio_pinconfig(41, g_AM_HAL_GPIO_INPUT); // To view ITM on Apollo3 EVB, fly-wire GP45 to GP41
#else
    am_hal_gpio_pinconfig(GPIO_AP3_SWO, g_IOX_GPIO_ITM_SWO); // Enable ITM output on GP45
#endif

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Apollo5 Presilicon Board IOX\n");
    am_util_stdio_printf("Firmware version: %d\n\n", IOX_FW_REV);

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
        if (g_bButtonPress)
        {
            g_bButtonPress = false;
            g_mode = (g_mode + 1) & 0xf; // mode is limited to 3 bits
            if (g_mode == 0)
            {
                g_mode++; // don't allow mode 0 (LEDs would all be off)
            }
#ifdef DEBUG_OUTPUT_EN
            am_util_stdio_printf("Button pressed - new mode is %d\n", g_mode);
#endif
            switch (g_mode)
            {
                case 1: // GSW (smart watch use case for System Test)
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 1 - GSW Use Cases w/ Touch ALS\n");
#endif
                    g_mspi16State = MSPI16_AP_PSRAM;
                    g_sdio0State = SDIO_eMMC;
                    g_sdio1State = SDIO_UNUSED;
                    g_bSdioRstCtrl = false; // GP61/62 routed to Touch ALS signals
                    g_mspi8State = MSPI8_FLASH;
                    g_pdm0State = PDM_UART;
                    g_pdm1State = PDM_UART;
                    g_iom0State = IOM_I2S_HDR;
                    g_iom1State = IOM_PSRAM;
                    g_uartState = UART_HDR_FTDI;
                    g_dispState = DISP_OB_DISPLAY;
                    break;
                case 2: // Peripheral Card
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 2 - Peripheral Card\n");
#endif
                    g_mspi16State = MSPI16_PC;
                    g_sdio0State = SDIO_PC;
                    g_sdio1State = SDIO_UNUSED;
                    g_bSdioRstCtrl = true; // GP61/62 routed to SDIO RSTn signals
                    g_mspi8State = MSPI8_PC;
                    g_pdm0State = PDM_PC;
                    g_pdm1State = PDM_PC;
                    g_iom0State = IOM_PC;
                    g_iom1State = IOM_PC;
                    g_uartState = UART_HDR_FTDI;
                    g_dispState = DISP_PC;
                    break;
                case 3: // MSPI + eMMC + SDIO
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 3 - MSPI+eMMC+SDIO\n");
#endif
                    g_mspi16State = MSPI16_AP_PSRAM;
                    g_sdio0State = SDIO_eMMC;
                    g_sdio1State = SDIO_EXT;
                    g_bSdioRstCtrl = true; // GP61/62 routed to SDIO RSTn signals
                    g_mspi8State = MSPI8_FLASH;
                    g_pdm0State = PDM_SDIO1;
                    g_pdm1State = PDM_OPEN;
                    g_iom0State = IOM_SDIO1;
                    g_iom1State = IOM_SDIO1;
                    g_uartState = UART_HDR_FTDI;
                    g_dispState = DISP_OB_DISPLAY;
                    break;
                case 4: // Dual eMMC
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 4 - Dual eMMC\n");
#endif
                    g_mspi16State = MSPI16_AP_PSRAM;
                    g_sdio0State = SDIO_eMMC;
                    g_sdio1State = SDIO_eMMC;
                    g_bSdioRstCtrl = true; // GP61/62 routed to SDIO RSTn signals
                    g_mspi8State = MSPI8_FLASH;
                    g_pdm0State = PDM_SDIO1;
                    g_pdm1State = PDM_OPEN;
                    g_iom0State = IOM_SDIO1;
                    g_iom1State = IOM_SDIO1;
                    g_uartState = UART_HDR_FTDI;
                    g_dispState = DISP_PC;
                    break;
                case 5: // Display Board
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 5 - Display Board\n");
#endif
                    g_mspi16State = MSPI16_AP_PSRAM;
                    g_sdio0State = SDIO_eMMC;
                    g_sdio1State = SDIO_UNUSED;
                    g_bSdioRstCtrl = true; // GP61/62 routed to SDIO RSTn signals
                    g_mspi8State = MSPI8_FLASH;
                    g_pdm0State = PDM_HDR;
                    g_pdm1State = PDM_HDR;
                    g_iom0State = IOM_PSRAM;
                    g_iom1State = IOM_I2S_HDR;
                    g_uartState = UART_HDR_FTDI;
                    g_dispState = DISP_OB_DISPLAY;
                    break;
                case 6: // SDIO WiFi
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 6 - SDIO WiFi\n");
#endif
                    g_mspi16State = MSPI16_WB_PSRAM;
                    g_sdio0State = SDIO_EXT;
                    g_sdio1State = SDIO_EXT;
                    g_bSdioRstCtrl = true; // GP61/62 routed to SDIO RSTn signals
                    g_mspi8State = MSPI8_FLASH;
                    g_pdm0State = PDM_SDIO1;
                    g_pdm1State = PDM_OPEN;
                    g_iom0State = IOM_SDIO1;
                    g_iom1State = IOM_SDIO1;
                    g_uartState = UART_HDR_FTDI;
                    g_dispState = DISP_OB_DISPLAY;
                    break;
                case 7: // GSW (smart watch use case for System Test - with SDIO Reset control)
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 7 - GSW Use Cases w/ SDIO Reset Control\n");
#endif
                    g_mspi16State = MSPI16_AP_PSRAM;
                    g_sdio0State = SDIO_eMMC;
                    g_sdio1State = SDIO_UNUSED;
                    g_bSdioRstCtrl = true; // GP61/62 routed to SDIO RSTn signals
                    g_mspi8State = MSPI8_FLASH;
                    g_pdm0State = PDM_UART;
                    g_pdm1State = PDM_UART;
                    g_iom0State = IOM_I2S_HDR;
                    g_iom1State = IOM_PSRAM;
                    g_uartState = UART_HDR_FTDI;
                    g_dispState = DISP_OB_DISPLAY;
                    break;
                case 8: // Legacy IOs
#ifdef DEBUG_OUTPUT_EN
                    am_util_stdio_printf("Mode 8 - Legacy IOs Use Cases\n");
#endif
                    g_mspi16State = MSPI16_PC;
                    g_sdio0State = SDIO_UNUSED;
                    g_sdio1State = SDIO_UNUSED;
                    g_bSdioRstCtrl = false; // GP61/62 routed to SDIO RSTn signals
                    g_mspi8State = MSPI8_PC;
                    g_pdm0State = PDM_PC;
                    g_pdm1State = PDM_PC;
                    g_iom0State = IOM_PSRAM;
                    g_iom1State = IOM_IOS_LB;
                    g_uartState = UART_VCOM;
                    g_dispState = DISP_PC;
                    break;
                //
                // Placeholders for new modes
                //
#ifdef DEBUG_ON_AP3_EVB // cycle through all modes if debugging on Apollo3 EVB
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
                    g_bButtonPress = true;
                    break;
            };
            update_iox_state(); // update iox pins based on new state variable settings
            update_leds();
        }
    }
}
