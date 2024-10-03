//*****************************************************************************
//
//  am_bsp.c
//! @file
//!
//! @brief Top level functions for performing board initialization.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo3_eb_bsp BSP for the Apollo3 Engineering Board
//! @ingroup BSP
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

#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Power tracking structures for IOM and UART
//
//*****************************************************************************
am_bsp_uart_pwrsave_t am_bsp_uart_pwrsave[AM_REG_UART_NUM_MODULES];

//*****************************************************************************
//
// LEDs
//
//*****************************************************************************
#ifdef AM_BSP_NUM_LEDS
am_devices_led_t am_bsp_psLEDs[AM_BSP_NUM_LEDS] =
{
#if 1   // The Engineering Board has no native LEDs
    {0xFFFFFFFF, 0xFFFFFFFF}
#else   // Use for apollo3_evb silicon (and pretty sure FPGA Red Board)
    {AM_BSP_GPIO_LED0, AM_DEVICES_LED_ON_HIGH | AM_DEVICES_LED_POL_DIRECT_DRIVE_M},
    {AM_BSP_GPIO_LED1, AM_DEVICES_LED_ON_HIGH | AM_DEVICES_LED_POL_DIRECT_DRIVE_M},
    {AM_BSP_GPIO_LED2, AM_DEVICES_LED_ON_HIGH | AM_DEVICES_LED_POL_DIRECT_DRIVE_M},
    {AM_BSP_GPIO_LED3, AM_DEVICES_LED_ON_HIGH | AM_DEVICES_LED_POL_DIRECT_DRIVE_M},
    {AM_BSP_GPIO_LED4, AM_DEVICES_LED_ON_HIGH | AM_DEVICES_LED_POL_DIRECT_DRIVE_M}
#endif
};
#endif

#ifdef AM_BSP_NUM_BUTTONS
//*****************************************************************************
//
// Buttons.
//
//*****************************************************************************
am_devices_button_t am_bsp_psButtons[AM_BSP_NUM_BUTTONS] =
{
    AM_DEVICES_BUTTON(AM_BSP_GPIO_BUTTON0, AM_DEVICES_BUTTON_NORMAL_HIGH),
    AM_DEVICES_BUTTON(AM_BSP_GPIO_BUTTON1, AM_DEVICES_BUTTON_NORMAL_HIGH),
    //AM_DEVICES_BUTTON(AM_BSP_GPIO_BUTTON2, AM_DEVICES_BUTTON_NORMAL_HIGH),
};
#endif

//*****************************************************************************
//
// Print interface tracking variable.
//
//*****************************************************************************
static uint32_t g_ui32PrintInterface = AM_BSP_PRINT_INFC_NONE;

//*****************************************************************************
//
// Default UART configuration settings.
//
//*****************************************************************************
static void *g_sCOMUART;

static const am_hal_uart_config_t g_sBspUartConfig =
{
    //
    // Standard UART settings: 115200-8-N-1
    //
    .ui32BaudRate = 115200,
    .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
    .ui32Parity = AM_HAL_UART_PARITY_NONE,
    .ui32StopBits = AM_HAL_UART_ONE_STOP_BIT,
    .ui32FlowControl = AM_HAL_UART_FLOW_CTRL_NONE,

    //
    // Set TX and RX FIFOs to interrupt at half-full.
    //
    .ui32FifoLevels = (AM_HAL_UART_TX_FIFO_1_2 |
                       AM_HAL_UART_RX_FIFO_1_2),

    //
    // The default interface will just use polling instead of buffers.
    //
    .pui8TxBuffer = 0,
    .ui32TxBufferSize = 0,
    .pui8RxBuffer = 0,
    .ui32RxBufferSize = 0,
};

#ifndef AM_BSP_DISABLE_BUFFERED_UART
//*****************************************************************************
//
// Default UART configuration settings if using buffers.
//
//*****************************************************************************
#define AM_BSP_UART_BUFFER_SIZE     1024
static uint8_t pui8UartTxBuffer[AM_BSP_UART_BUFFER_SIZE];
static uint8_t pui8UartRxBuffer[AM_BSP_UART_BUFFER_SIZE];

static am_hal_uart_config_t g_sBspUartBufferedConfig =
{
    //
    // Standard UART settings: 115200-8-N-1
    //
    .ui32BaudRate = 115200,
    .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
    .ui32Parity = AM_HAL_UART_PARITY_NONE,
    .ui32StopBits = AM_HAL_UART_ONE_STOP_BIT,
    .ui32FlowControl = AM_HAL_UART_FLOW_CTRL_NONE,

    //
    // Set TX and RX FIFOs to interrupt at half-full.
    //
    .ui32FifoLevels = (AM_HAL_UART_TX_FIFO_1_2 |
                       AM_HAL_UART_RX_FIFO_1_2),

    //
    // The default interface will just use polling instead of buffers.
    //
    .pui8TxBuffer = pui8UartTxBuffer,
    .ui32TxBufferSize = sizeof(pui8UartTxBuffer),
    .pui8RxBuffer = pui8UartRxBuffer,
    .ui32RxBufferSize = sizeof(pui8UartRxBuffer),
};
#endif // AM_BSP_DISABLE_BUFFERED_UART

//*****************************************************************************
//
//! @brief Prepare the MCU for low power operation.
//!
//! This function enables several power-saving features of the MCU, and
//! disables some of the less-frequently used peripherals. It also sets the
//! system clock to 24 MHz.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_low_power_init(void)
{
    // #### INTERNAL BEGIN ####
#if 0 // Later production versions of SBL no longer needs this call.
    // However, early production parts, such as what is on the EVB, do need it.
    //
    // Make sure SWO/ITM/TPIU is disabled.
    // SBL may not get it completely shut down.
    //
    am_bsp_itm_printf_disable();
#endif
    // #### INTERNAL END ####
#if 0 // For the FPGA, skip all this for now.
    //
    // Initialize for low power in the power control block
    //
    am_hal_pwrctrl_low_power_init();

    // #### INTERNAL BEGIN ####
#if 0
    // CSV-349, https://ambiqmicro.atlassian.net/browse/CSV-349
    // As of 6/18/19, the validation team discovered that this switch of the
    // Apollo3 RTC to LFRC would lock up some A1 and B0 parts. Until the issue
    // is root-caused and a solid workaround determined, we must skip it.

    //
    // Run the RTC off the LFRC.
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC);

    //
    // Stop the XTAL.
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_XTAL_STOP, 0);
#endif
    // #### INTERNAL END ####
    //
    // Disable the RTC.
    //
    am_hal_rtc_osc_disable();
#endif
} // am_bsp_low_power_init()

//*****************************************************************************
//
//! @brief Enable the TPIU and ITM for debug printf messages.
//!
//! This function enables TPIU registers for debug printf messages and enables
//! ITM GPIO pin to SWO mode. This function should be called after reset and
//! after waking up from deep sleep.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_debug_printf_enable(void)
{
    if (g_ui32PrintInterface == AM_BSP_PRINT_INFC_SWO)
    {
        am_bsp_itm_printf_enable();
    }
    else if (g_ui32PrintInterface == AM_BSP_PRINT_INFC_UART0)
    {
        am_bsp_uart_printf_enable();
    }
#ifndef AM_BSP_DISABLE_BUFFERED_UART
    else if (g_ui32PrintInterface == AM_BSP_PRINT_INFC_BUFFERED_UART0)
    {
        am_bsp_buffered_uart_printf_enable();
    }
#endif // AM_BSP_DISABLE_BUFFERED_UART
} // am_bsp_debug_printf_enable()

//*****************************************************************************
//
//! @brief Enable the TPIU and ITM for debug printf messages.
//!
//! This function disables TPIU registers for debug printf messages and
//! enables ITM GPIO pin to GPIO mode and prepares the MCU to go to deep sleep.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_debug_printf_disable(void)
{
    if (g_ui32PrintInterface == AM_BSP_PRINT_INFC_SWO)
    {
        am_bsp_itm_printf_disable();
    }
    else if (g_ui32PrintInterface == AM_BSP_PRINT_INFC_UART0)
    {
        am_bsp_uart_printf_disable();
    }
} // am_bsp_debug_printf_disable()

//*****************************************************************************
//
// @brief Enable printing over ITM.
//
//*****************************************************************************
void
am_bsp_itm_printf_enable(void)
{
    am_hal_tpiu_config_t TPIUcfg;

    //
    // Set the global print interface.
    //
    g_ui32PrintInterface = AM_BSP_PRINT_INFC_SWO;

    //
    // Enable the ITM interface and the SWO pin.
    //
    am_hal_itm_enable();
//  am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, g_AM_BSP_GPIO_ITM_SWO);

    //
    // Enable the ITM and TPIU
    // Set the BAUD clock for 1M
    //
    // #### INTERNAL BEGIN ####
#if APOLLO3P_FPGA
    //
    // Stingray FPGA MCUCTRL TPIU DIV2 clock runs at 24MHz, all others run at
    //  1MHz. So we need to modify parameters to force 1Mbit output.
    //
    TPIUcfg.ui32SetItmBaud       = 0;   // Force use of the following cfg parms
    TPIUcfg.ui32ClockPrescaler   = 24;  // 24MHz / 24 = 1Mbit
    TPIUcfg.ui32ParallelPortSize = 1;   // 1 bit port size
    TPIUcfg.ui32PinProtocol      =      // UART protocol
         _VAL2FLD( TPI_SPPR_TXMODE, TPI_SPPR_TXMODE_UART);
    TPIUcfg.ui32TraceClkIn       =      // Use DIV2 clock.
            _VAL2FLD(MCUCTRL_TPIUCTRL_CLKSEL, MCUCTRL_TPIUCTRL_CLKSEL_HFRCDIV2) |
            _VAL2FLD(MCUCTRL_TPIUCTRL_ENABLE, MCUCTRL_TPIUCTRL_ENABLE_EN);
    am_hal_tpiu_enable(&TPIUcfg);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, g_AM_BSP_GPIO_ITM_SWO);
#elif MCU_VALIDATION
#if 1
    //
    // Force a custom configuration using the 24MHz clock.
    // Since the FPGA runs at 1/2 speed, we must take that into account.
    //
    TPIUcfg.ui32SetItmBaud = 0; // Force a custom configuration
    TPIUcfg.ui32TraceClkIn =
            _VAL2FLD(MCUCTRL_TPIUCTRL_CLKSEL, MCUCTRL_TPIUCTRL_CLKSEL_HFRCDIV2) |
            _VAL2FLD(MCUCTRL_TPIUCTRL_ENABLE, MCUCTRL_TPIUCTRL_ENABLE_EN);
    TPIUcfg.ui32PinProtocol = _VAL2FLD( TPI_SPPR_TXMODE, TPI_SPPR_TXMODE_UART);
    TPIUcfg.ui32ParallelPortSize = TPI_CSPSR_CWIDTH_1BIT;
//  TPIUcfg.ui32ClockPrescaler = ((24000000 / 1) / 115200) - 1;   // Assume FPGA is running at full speed
    TPIUcfg.ui32ClockPrescaler = ((24000000 / 2) / 115200) - 1;   // Assume FPGA is running at 1/2 speed
#else
    //
    // Use this setting for silicon.
    //
    TPIUcfg.ui32SetItmBaud = AM_HAL_TPIU_BAUD_115200;
#endif
#else
    // #### INTERNAL END ####
    TPIUcfg.ui32SetItmBaud = AM_HAL_TPIU_BAUD_1M;
    // #### INTERNAL BEGIN ####
#endif
    // #### INTERNAL END ####
    am_hal_tpiu_enable(&TPIUcfg);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, g_AM_BSP_GPIO_ITM_SWO);
    // #### INTERNAL BEGIN ####
#if MCU_VALIDATION
    am_util_delay_ms(2000); // wait for armada to catch up
    am_hal_itm_stimulus_reg_word_write(0, 0xf8f8f8f8);  // tell lemans to pay attention
#endif // MCU_VALIDATION
    // #### INTERNAL END ####

    //
    // Attach the ITM to the STDIO driver.
    //
    am_util_stdio_printf_init(am_hal_itm_print);
} // am_bsp_itm_printf_enable()

//*****************************************************************************
//
// @brief Disable printing over ITM.
//
//*****************************************************************************
void
am_bsp_itm_printf_disable(void)
{
    //
    // Disable the ITM/TPIU
    //
    am_hal_itm_disable();

    //
    // Detach the ITM interface from the STDIO driver.
    //
    am_util_stdio_printf_init(0);

    //
    // Disconnect the SWO pin
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, g_AM_HAL_GPIO_DISABLE);
} // am_bsp_itm_printf_disable()

//*****************************************************************************
//
//! @brief Set up the IOM pins based on mode and module.
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_iom_pins_enable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode)
{
    uint32_t ui32Combined;

    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_IOM_NUM_MODULES )
    {
        //
        // FPGA supports only IOM0 and 1.
        //
        return;
    }

    ui32Combined = ((ui32Module << 2) | eIOMMode);

    switch ( ui32Combined )
    {
        case ((0 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCK,  g_AM_BSP_GPIO_IOM0_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MISO, g_AM_BSP_GPIO_IOM0_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MOSI, g_AM_BSP_GPIO_IOM0_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS,   g_AM_BSP_GPIO_IOM0_CS);
            break;

        case ((1 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCK,  g_AM_BSP_GPIO_IOM1_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MISO, g_AM_BSP_GPIO_IOM1_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MOSI, g_AM_BSP_GPIO_IOM1_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_CS,   g_AM_BSP_GPIO_IOM1_CS);
            break;

        case ((2 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCK,  g_AM_BSP_GPIO_IOM2_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MISO, g_AM_BSP_GPIO_IOM2_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MOSI, g_AM_BSP_GPIO_IOM2_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS,   g_AM_BSP_GPIO_IOM2_CS);
            break;

        case ((3 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCK,  g_AM_BSP_GPIO_IOM3_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MISO, g_AM_BSP_GPIO_IOM3_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MOSI, g_AM_BSP_GPIO_IOM3_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_CS,   g_AM_BSP_GPIO_IOM3_CS);
            break;

        case ((4 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCK,  g_AM_BSP_GPIO_IOM4_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MISO, g_AM_BSP_GPIO_IOM4_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MOSI, g_AM_BSP_GPIO_IOM4_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_CS,   g_AM_BSP_GPIO_IOM4_CS);
            break;

        case ((5 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCK,  g_AM_BSP_GPIO_IOM5_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MISO, g_AM_BSP_GPIO_IOM5_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MOSI, g_AM_BSP_GPIO_IOM5_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_CS,   g_AM_BSP_GPIO_IOM5_CS);
            break;

        case ((0 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCL,  g_AM_BSP_GPIO_IOM0_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SDA,  g_AM_BSP_GPIO_IOM0_SDA);
            break;

        case ((1 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCL,  g_AM_BSP_GPIO_IOM1_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SDA,  g_AM_BSP_GPIO_IOM1_SDA);
            break;

        case ((2 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCL,  g_AM_BSP_GPIO_IOM2_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SDA,  g_AM_BSP_GPIO_IOM2_SDA);
            break;

        case ((3 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCL,  g_AM_BSP_GPIO_IOM3_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SDA,  g_AM_BSP_GPIO_IOM3_SDA);
            break;

        case ((4 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCL,  g_AM_BSP_GPIO_IOM4_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SDA,  g_AM_BSP_GPIO_IOM4_SDA);
            break;

        case ((5 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCL,  g_AM_BSP_GPIO_IOM5_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SDA,  g_AM_BSP_GPIO_IOM5_SDA);
            break;

        default:
            break;
    }
} // am_bsp_iom_pins_enable()

//*****************************************************************************
//
//! @brief Disable the IOM pins based on mode and module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_iom_pins_disable(uint32_t ui32Module, am_hal_iom_mode_e eIOMMode)
{
    uint32_t ui32Combined;

    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_IOM_NUM_MODULES )
    {
        //
        // FPGA supports only IOM0 and 1.
        //
        return;
    }

    ui32Combined = ((ui32Module << 2) | eIOMMode);

    switch ( ui32Combined )
    {
        case ((0 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCK,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MISO, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MOSI, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS,   g_AM_HAL_GPIO_DISABLE);
            break;

        case ((1 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCK,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MISO, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MOSI, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_CS,   g_AM_HAL_GPIO_DISABLE);
            break;

        case ((2 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCK,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MISO, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MOSI, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS,   g_AM_HAL_GPIO_DISABLE);
            break;

        case ((3 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCK,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MISO, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MOSI, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_CS,   g_AM_HAL_GPIO_DISABLE);
            break;

        case ((4 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCK,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MISO, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MOSI, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_CS,   g_AM_HAL_GPIO_DISABLE);
            break;

        case ((5 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCK,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MISO, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MOSI, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_CS,   g_AM_HAL_GPIO_DISABLE);
            break;

        case ((0 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCL,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SDA,  g_AM_HAL_GPIO_DISABLE);
            break;

        case ((1 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCL,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SDA,  g_AM_HAL_GPIO_DISABLE);
            break;

        case ((2 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCL,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SDA,  g_AM_HAL_GPIO_DISABLE);
            break;

        case ((3 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCL,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SDA,  g_AM_HAL_GPIO_DISABLE);
            break;

        case ((4 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCL,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SDA,  g_AM_HAL_GPIO_DISABLE);
            break;

        case ((5 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCL,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SDA,  g_AM_HAL_GPIO_DISABLE);
            break;
        default:
            break;
    }
} // am_bsp_iom_pins_disable()

//*****************************************************************************
//
//! @brief Set up the MSPI pins based on the external flash device type.
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_mspi_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_BSP_GPIO_MSPI0_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_BSP_GPIO_MSPI0_SCK);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_BSP_GPIO_MSPI0_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_BSP_GPIO_MSPI0_SCK);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_BSP_GPIO_MSPI0_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  g_AM_BSP_GPIO_MSPI0_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  g_AM_BSP_GPIO_MSPI0_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  g_AM_BSP_GPIO_MSPI0_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  g_AM_BSP_GPIO_MSPI0_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_BSP_GPIO_MSPI0_SCK);
                    break;
                default:
                    break;
            }
            break;

        case 1:
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_BSP_GPIO_MSPI1_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_BSP_GPIO_MSPI1_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_BSP_GPIO_MSPI1_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_BSP_GPIO_MSPI1_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  g_AM_BSP_GPIO_MSPI1_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  g_AM_BSP_GPIO_MSPI1_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  g_AM_BSP_GPIO_MSPI1_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    break;
                default:
                    break;
            }
            break;

        case 2:
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                    break;
                default:
                    break;
            }
            break;

        default:
            break;
    }
} // am_bsp_mspi_pins_enable()

//*****************************************************************************
//
//! @brief Disable the MSPI pins based on the external flash device type.
//!
//! This function configures up to 10-pins for MSPI serial, dual, quad,
//! dual-quad, and octal operation.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_mspi_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_HAL_GPIO_DISABLE);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_HAL_GPIO_DISABLE);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_HAL_GPIO_DISABLE);
                    break;
                default:
                    break;
            }
            break;
        case 1:
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_HAL_GPIO_DISABLE);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_HAL_GPIO_DISABLE);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_HAL_GPIO_DISABLE);
                    break;
                default:
                    break;
            }
            break;
        case 2:
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_HAL_GPIO_DISABLE);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_HAL_GPIO_DISABLE);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_HAL_GPIO_DISABLE);
                    break;
                default:
                    break;
            }
            break;
        default:
        break;
    }
} // am_bsp_mspi_pins_disable()

//*****************************************************************************
//
//! @brief Set up the IOS pins based on mode and module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_ios_pins_enable(uint32_t ui32Module, uint32_t ui32IOSMode)
{
    uint32_t ui32Combined;

    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_IOSLAVE_NUM_MODULES )
    {
        return;
    }

    ui32Combined = ((ui32Module << 2) | ui32IOSMode);

    switch ( ui32Combined )
    {
        case ((0 << 2) | AM_HAL_IOS_USE_SPI):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCK,  g_AM_BSP_GPIO_IOS_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MISO, g_AM_BSP_GPIO_IOS_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MOSI, g_AM_BSP_GPIO_IOS_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_CE,   g_AM_BSP_GPIO_IOS_CE);
            break;

        case ((0 << 2) | AM_HAL_IOS_USE_I2C):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCL,  g_AM_BSP_GPIO_IOS_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SDA,  g_AM_BSP_GPIO_IOS_SDA);
            break;
        default:
            break;
    }
} // am_bsp_ios_pins_enable()

//*****************************************************************************
//
//! @brief Disable the IOS pins based on mode and module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_ios_pins_disable(uint32_t ui32Module, uint32_t ui32IOSMode)
{
    uint32_t ui32Combined;

    //
    // Validate parameters
    //
    if ( ui32Module >= AM_REG_IOSLAVE_NUM_MODULES )
    {
        return;
    }

    ui32Combined = ((ui32Module << 2) | ui32IOSMode);

    switch ( ui32Combined )
    {
        case ((0 << 2) | AM_HAL_IOS_USE_SPI):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCK,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MISO, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MOSI, g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_CE,   g_AM_HAL_GPIO_DISABLE);
            break;

        case ((0 << 2) | AM_HAL_IOS_USE_I2C):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCL,  g_AM_HAL_GPIO_DISABLE);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SDA,  g_AM_HAL_GPIO_DISABLE);
            break;
        default:
            break;
    }
} // am_bsp_ios_pins_disable()

//*****************************************************************************
//
//! @brief UART-based string print function.
//!
//! This function is used for printing a string via the UART, which for some
//! MCU devices may be multi-module.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_uart_string_print(char *pcString)
{
    uint32_t ui32StrLen = 0;
    uint32_t ui32BytesWritten = 0;

    //
    // Measure the length of the string.
    //
    while (pcString[ui32StrLen] != 0)
    {
        ui32StrLen++;
    }

    //
    // Print the string via the UART.
    //
    const am_hal_uart_transfer_t sUartWrite =
    {
        .ui32Direction = AM_HAL_UART_WRITE,
        .pui8Data = (uint8_t *) pcString,
        .ui32NumBytes = ui32StrLen,
        .ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER,
        .pui32BytesTransferred = &ui32BytesWritten,
    };

    am_hal_uart_transfer(g_sCOMUART, &sUartWrite);

    // #### INTERNAL BEGIN ####
    // CAJ: We probably need to do a different kind of error check here. The
    // transfer as written above will block forever, so the strings should
    // never ever be different lengths.
    // #### INTERNAL END ####
    if (ui32BytesWritten != ui32StrLen)
    {
        //
        // Couldn't send the whole string!!
        //
        while(1);
    }
} // am_bsp_uart_string_print()

//*****************************************************************************
//
// Pass-through function to let applications access the COM UART.
//
//*****************************************************************************
uint32_t
am_bsp_com_uart_transfer(const am_hal_uart_transfer_t *psTransfer)
{
    return am_hal_uart_transfer(g_sCOMUART, psTransfer);
} // am_bsp_com_uart_transfer()

//*****************************************************************************
//
// Initialize and configure the UART
//
//*****************************************************************************
void
am_bsp_uart_printf_enable(void)
{
    //
    // Save the information that we're using the UART for printing.
    //
    g_ui32PrintInterface = AM_BSP_PRINT_INFC_UART0;

    //
    // Initialize, power up, and configure the communication UART. Use the
    // custom configuration if it was provided. Otherwise, just use the default
    // configuration.
    //
    am_hal_uart_initialize(AM_BSP_UART_PRINT_INST, &g_sCOMUART);
    am_hal_uart_power_control(g_sCOMUART, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_uart_configure(g_sCOMUART, &g_sBspUartConfig);

    //
    // Enable the UART pins.
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX);

    //
    // Register the BSP print function to the STDIO driver.
    //
    am_util_stdio_printf_init(am_bsp_uart_string_print);
} // am_bsp_uart_printf_enable()

//*****************************************************************************
//
// Disable the UART
//
//*****************************************************************************
void
am_bsp_uart_printf_disable(void)
{
    //
    // Make sure the UART has finished sending everything it's going to send.
    //
    am_hal_uart_tx_flush(g_sCOMUART);

    //
    // Detach the UART from the stdio driver.
    //
    am_util_stdio_printf_init(0);

    //
    // Power down the UART, and surrender the handle.
    //
    am_hal_uart_power_control(g_sCOMUART, AM_HAL_SYSCTRL_DEEPSLEEP, false);
    am_hal_uart_deinitialize(g_sCOMUART);

    //
    // Disable the UART pins.
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_HAL_GPIO_DISABLE);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_HAL_GPIO_DISABLE);

} // am_bsp_uart_printf_disable()

#ifndef AM_BSP_DISABLE_BUFFERED_UART
//*****************************************************************************
//
// Initialize and configure the UART
//
//*****************************************************************************
void
am_bsp_buffered_uart_printf_enable(void)
{
    //
    // Save the information that we're using the UART for printing.
    //
    g_ui32PrintInterface = AM_BSP_PRINT_INFC_UART0;

    //
    // Initialize, power up, and configure the communication UART. Use the
    // custom configuration if it was provided. Otherwise, just use the default
    // configuration.
    //
    am_hal_uart_initialize(AM_BSP_UART_PRINT_INST, &g_sCOMUART);
    am_hal_uart_power_control(g_sCOMUART, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_uart_configure(g_sCOMUART, &g_sBspUartBufferedConfig);

    //
    // Enable the UART pins.
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX);

    //
    // Register the BSP print function to the STDIO driver.
    //
    am_util_stdio_printf_init(am_bsp_uart_string_print);

    //
    // Enable the interrupts for the UART.
    //
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + AM_BSP_UART_PRINT_INST));
} // am_bsp_buffered_uart_printf_enable()

//*****************************************************************************
//
// Interrupt routine for the buffered UART interface.
//
//*****************************************************************************
void
am_bsp_buffered_uart_service(void)
{
    uint32_t ui32Status, ui32Idle;
    am_hal_uart_interrupt_status_get(g_sCOMUART, &ui32Status, true);
    am_hal_uart_interrupt_clear(g_sCOMUART, ui32Status);
    am_hal_uart_interrupt_service(g_sCOMUART, ui32Status, &ui32Idle);
} // am_bsp_buffered_uart_service()
#endif // AM_BSP_DISABLE_BUFFERED_UART

// #### INTERNAL BEGIN ####
#if 0
//*****************************************************************************
//
// Internal routine to enable the SIMOBUCK.
//
//*****************************************************************************
//
// 1/2/18 REH
// These settings enable bucks, which are normally done via customer setup
// of customer INFO0 space. The following settings are internal only.
//
#define AM_REG_MCUCTRL_SIMOBUCK3_O                   0x00000358
#define AM_REG_MCUCTRL_SIMOBUCK3_SIMOBUCKBIASPDNB_S  31
#define AM_REG_MCUCTRL_SIMOBUCK3_SIMOBUCKBIASPDNB_M  0x80000000

#define AM_REG_PWRCTRL_MISC_SIMOBUCKEN_S             0
#define AM_REG_PWRCTRL_MISC_SIMOBUCKEN_M             0x00000001

void
am_bsp_internal_bucks_enable(void)
{
    if ( !am_hal_pwrctrl_simobuck_enabled_check() )
    {
        //
        // Simobuck is not enabled.  Enable manually.
        //
        AM_BFW(MCUCTRL, SIMOBUCK3, SIMOBUCKBIASPDNB, 1);
        AM_BFW(PWRCTRL, MISC, SIMOBUCKEN, 1);
    }
} // am_bsp_internal_bucks_enable()
#endif // 0
// #### INTERNAL END ####
// #### INTERNAL BEGIN ####
// TODO: Should this be MCU_VALIDATION?
#if 1  //MCU_VALIDATION
//*****************************************************************************
//
//! @brief Put the test errors to the TPIU and ITM.
//!
//! This function outputs the number of test errors to the TPIU/ITM.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_ckerr(int32_t i32ErrorCount)
{
    if (0 == i32ErrorCount)
    {
//      am_util_stdio_printf("\n** TEST PASSED **\n");
        am_hal_itm_stimulus_reg_word_write(1, 0x0000600d);  // indicates test pass.
    }
    else
    {
//      am_util_stdio_printf("\n** TEST FAILED, ERRORCOUNT = %10d **\n", i32ErrorCount);
        am_hal_itm_stimulus_reg_word_write(1, ((i32ErrorCount << 16) | 0x1BAD));    // indicates failure with error count.
    }

    am_hal_itm_stimulus_reg_word_write(0, 0x1);             // indicates test result ready for ITM.
} // am_bsp_ckerr()
#endif // MCU_VALIDATION
// #### INTERNAL END ####


//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
