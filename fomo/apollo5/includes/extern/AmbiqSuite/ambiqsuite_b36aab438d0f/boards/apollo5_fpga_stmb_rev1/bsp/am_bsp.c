//*****************************************************************************
//
//  am_bsp.c
//! @file
//!
//! @brief Top level functions for performing board initialization.
//!
//! @addtogroup BSP Board Support Package (BSP)
//! @addtogroup apollo5_fpga_stmb_rev1 BSP
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
// Configuration.
//
//*****************************************************************************
#define AM_BSP_ITM_FREQUENCY     AM_HAL_TPIU_BAUD_1M

// #### INTERNAL BEGIN ####
#if MCU_VALIDATION
#define AM_BSP_ITM_FREQUENCY     AM_HAL_TPIU_BAUD_115200
#endif
// #### INTERNAL END ####

//*****************************************************************************
//
// macros for the ETMs on board.
//
//*****************************************************************************
#define AM_BSP_PDM_ETM          AM_BSP_GRP0_ETM
#define AM_BSP_IOS_ETM          AM_BSP_GRP1_ETM

//*****************************************************************************
//
// Buttons.
//
//*****************************************************************************
#if AM_BSP_NUM_BUTTONS
am_devices_button_t am_bsp_psButtons[AM_BSP_NUM_BUTTONS] =
{
    AM_DEVICES_BUTTON(AM_BSP_GPIO_PUSH1, AM_DEVICES_BUTTON_NORMAL_HIGH),
    AM_DEVICES_BUTTON(AM_BSP_GPIO_PUSH2, AM_DEVICES_BUTTON_NORMAL_HIGH),
};
#endif

#if AM_BSP_NUM_RPI_BUTTONS
am_devices_button_t am_bsp_rpi_psButtons[AM_BSP_NUM_RPI_BUTTONS] =
{
    AM_DEVICES_BUTTON(AM_BSP_GPIO_RPI_PUSH1, AM_DEVICES_BUTTON_NORMAL_HIGH),
    AM_DEVICES_BUTTON(AM_BSP_GPIO_RPI_PUSH2, AM_DEVICES_BUTTON_NORMAL_HIGH),
};
#endif

#if defined (DISP_CTRL_IP)
//*****************************************************************************
//
// Display configurations.
//
//*****************************************************************************
am_devices_display_hw_config_t g_sDispCfg =
{
    //display driver IC type
    .eIC = DISP_IC_RM69330,

    //display te pin
    .ui16TEpin = DISPLAY_TE_PIN,

    //display resolutions
    .ui16ResX = 454,
    .ui16ResY = 454,

    .eInterface = DISP_IF_QSPI,
    .eDsiFreq  = AM_HAL_DSI_FREQ_TRIM_X20,
    .eDbiWidth = AM_HAL_DSI_DBI_WIDTH_16,
    .ui8NumLanes = 1,

    .eTEType = DISP_TE_DC,
    .ui16Offset = 14,
    .bFlip = false
};
#endif // DISP_CTRL_IP

//*****************************************************************************
//
//! @brief DSI VDD18 power switch.
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_external_vdd18_switch(bool bEnable)
{

} // am_bsp_external_vdd18_switch()

//*****************************************************************************
//
// Set display reset pins.
//
//*****************************************************************************
static inline void
am_bsp_disp_reset_pin_output_set(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;
    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;

    if ( g_sDispCfg.eInterface != DISP_IF_JDI )
    {
        am_hal_gpio_state_write(AM_BSP_GPIO_DISP_DEVICE_RST, eGpioWrType);
    }
} // am_bsp_disp_reset_pin_output_set()

//*****************************************************************************
//
// Set display reset pins.
//
//*****************************************************************************
void
am_bsp_disp_reset_pins_set(void)
{
    am_bsp_disp_reset_pin_output_set(true);
} // am_bsp_disp_reset_pins_set()

//*****************************************************************************
//
// Set display reset pins.
//
//*****************************************************************************
void
am_bsp_disp_reset_pins_clear(void)
{
    am_bsp_disp_reset_pin_output_set(false);
} // am_bsp_disp_reset_pins_clear()

#if defined (DISP_CTRL_IP)
//*****************************************************************************
//
// Set up the display pins.
//
//*****************************************************************************
void
am_bsp_disp_pins_enable(void)
{
#warning "If run SPI4,DSPI or QSPI,please connect IFSEL1 to GPIO22,IFSEL2 to GPIO24,RST to GPIO8,CE to GPIO7 ,CLK to GPIO25 and EN to VDD."
#warning "If run JDI interface,please short VCOM with VB."
    if ( g_sDispCfg.eInterface != DISP_IF_JDI )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSI_TE,  g_AM_BSP_GPIO_DISP_DSI_TE);
        am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL1, g_AM_BSP_GPIO_DISP_IFSEL1);
        am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL2, g_AM_BSP_GPIO_DISP_IFSEL2);
        am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DEVICE_RST, g_AM_BSP_GPIO_DISP_DEVICE_RST);
        am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DEVICE_EN, g_AM_BSP_GPIO_DISP_DEVICE_EN);

        am_hal_gpio_state_write(AM_BSP_GPIO_DISP_DEVICE_EN, AM_HAL_GPIO_OUTPUT_SET);
    }

    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_QSPI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_CS,  g_AM_BSP_GPIO_DISP_QSPI_CS);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D0,  g_AM_BSP_GPIO_DISP_QSPI_D0);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D1,  g_AM_BSP_GPIO_DISP_QSPI_D1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D2,  g_AM_BSP_GPIO_DISP_QSPI_D2);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D3,  g_AM_BSP_GPIO_DISP_QSPI_D3);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_SCK, g_AM_BSP_GPIO_DISP_QSPI_SCK);
            //
            // Set IM2 = 1, IM1 = 0 select to QSPI mode
            //
            am_hal_gpio_state_write(AM_BSP_GPIO_DISP_IFSEL2, AM_HAL_GPIO_OUTPUT_SET);
            am_hal_gpio_state_write(AM_BSP_GPIO_DISP_IFSEL1, AM_HAL_GPIO_OUTPUT_CLEAR);
            break;
        case DISP_IF_DSPI:
        case DISP_IF_SPI4:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_CS,  g_AM_BSP_GPIO_DISP_SPI_CS);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SD,  g_AM_BSP_GPIO_DISP_SPI_SD);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_DCX, g_AM_BSP_GPIO_DISP_SPI_DCX);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SCK, g_AM_BSP_GPIO_DISP_SPI_SCK);
            //am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SDI, g_AM_BSP_GPIO_DISP_SPI_SDI);
            //
            // Set IM2 = 0, IM1 = 1 select to SPI4/DSPI mode
            //
            am_hal_gpio_state_write(AM_BSP_GPIO_DISP_IFSEL2, AM_HAL_GPIO_OUTPUT_CLEAR);
            am_hal_gpio_state_write(AM_BSP_GPIO_DISP_IFSEL1, AM_HAL_GPIO_OUTPUT_SET);
            break;
        case DISP_IF_DSI:
            break;
        case DISP_IF_JDI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GEN,  g_AM_BSP_GPIO_DISP_GEN);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_INTB, g_AM_BSP_GPIO_DISP_INTB);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_R1,   g_AM_BSP_GPIO_DISP_R1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_R2,   g_AM_BSP_GPIO_DISP_R2);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_G1,   g_AM_BSP_GPIO_DISP_G1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_G2,   g_AM_BSP_GPIO_DISP_G2);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_B1,   g_AM_BSP_GPIO_DISP_B1);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_B2,   g_AM_BSP_GPIO_DISP_B2);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GSP,  g_AM_BSP_GPIO_DISP_GSP);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BSP,  g_AM_BSP_GPIO_DISP_BSP);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GCK,  g_AM_BSP_GPIO_DISP_GCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BCK,  g_AM_BSP_GPIO_DISP_BCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VA,   g_AM_BSP_GPIO_DISP_VA);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VCOM, g_AM_BSP_GPIO_DISP_VCOM);
            break;
        default:
            break;
    }
} // am_bsp_disp_pins_enable()

//*****************************************************************************
//
// Deinit the display pins.
//
//*****************************************************************************
void
am_bsp_disp_pins_disable(void)
{
    if ( g_sDispCfg.eInterface != DISP_IF_JDI )
    {
        am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DSI_TE,     am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL1,     am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_IFSEL2,     am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DEVICE_RST, am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DEVICE_EN,  am_hal_gpio_pincfg_disabled);
    }
    switch (g_sDispCfg.eInterface)
    {
        case DISP_IF_QSPI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_CS,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D0,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D1,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D2,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_D3,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_QSPI_SCK, am_hal_gpio_pincfg_disabled);
            break;
        case DISP_IF_DSPI:
        case DISP_IF_SPI4:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_CS,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SD,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_DCX, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SCK, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_SPI_SDI, am_hal_gpio_pincfg_disabled);
            break;
        case DISP_IF_DSI:
            break;
        case DISP_IF_JDI:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GEN,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_INTB, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_R1,   am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_R2,   am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_G1,   am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_G2,   am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_B1,   am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_B2,   am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GSP,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BSP,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_GCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_BCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VA,   am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_VCOM, am_hal_gpio_pincfg_disabled);
            break;
        default:
            break;
    }
} // am_bsp_disp_pins_disable()

#endif // DISP_CTRL_IP

//*****************************************************************************
//
// Display device enable
//
//*****************************************************************************
void
am_bsp_external_display_device_enable(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;
    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_DISP_DEVICE_EN, g_AM_BSP_GPIO_DISP_DEVICE_EN);
    am_hal_gpio_state_write(AM_BSP_GPIO_DISP_DEVICE_EN, eGpioWrType);
} // am_bsp_external_display_device_enable()

//*****************************************************************************
//
// Accelerometer device enable
//
//*****************************************************************************
void
am_bsp_external_accel_device_enable(bool bEnable)
{
    am_hal_gpio_write_type_e eGpioWrType;
    eGpioWrType = bEnable ? AM_HAL_GPIO_OUTPUT_SET : AM_HAL_GPIO_OUTPUT_CLEAR;
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ACCEL_DEVICE_EN, g_AM_BSP_GPIO_ACCEL_DEVICE_EN);
    am_hal_gpio_state_write(AM_BSP_GPIO_ACCEL_DEVICE_EN, eGpioWrType);
} // am_bsp_external_accel_device_enable()

//*****************************************************************************
//
// Print interface tracking variable.
//
//*****************************************************************************
typedef enum
{
    AM_BSP_PRINT_IF_NONE,
    AM_BSP_PRINT_IF_SWO,
    AM_BSP_PRINT_IF_UART,
    AM_BSP_PRINT_IF_BUFFERED_UART,
    AM_BSP_PRINT_IF_MEMORY,
}
am_bsp_print_interface_e;

static am_bsp_print_interface_e g_ePrintInterface = AM_BSP_PRINT_IF_NONE;

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
    .eDataBits = AM_HAL_UART_DATA_BITS_8,
    .eParity = AM_HAL_UART_PARITY_NONE,
    .eStopBits = AM_HAL_UART_ONE_STOP_BIT,
    .eFlowControl = AM_HAL_UART_FLOW_CTRL_NONE,

    //
    // Set TX and RX FIFOs to interrupt at half-full.
    //
    .eTXFifoLevel = AM_HAL_UART_FIFO_LEVEL_16,
    .eRXFifoLevel = AM_HAL_UART_FIFO_LEVEL_16,
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
#endif // AM_BSP_DISABLE_BUFFERED_UART

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// State variables for memory-based printf.
//
//*****************************************************************************
am_bsp_memory_printf_state_t *g_psPrintfState = 0;
// #### INTERNAL END ####
//*****************************************************************************
//
// External power on.
//
//*****************************************************************************
void am_bsp_external_pwr_on(void)
{

} // am_bsp_external_pwr_on()

//*****************************************************************************
//
// Prepare the MCU for low power operation.
//
//*****************************************************************************
void
am_bsp_low_power_init(void)
{
    //
    // Disable the ITM interface.
    //
    //am_bsp_itm_printf_disable();

    //
    // Initialize for low power in the power control block
    //
    am_hal_pwrctrl_low_power_init();

#if AM_BSP_ENABLE_SIMOBUCK
    //
    // Enable SIMOBUCK for this board
    //
    am_hal_pwrctrl_control(AM_HAL_PWRCTRL_CONTROL_SIMOBUCK_INIT, 0);
#endif

    //
    // Disable the RTC.
    //
    am_hal_rtc_osc_disable();

} // am_bsp_low_power_init()

//*****************************************************************************
//
// Enable the TPIU and ITM for debug printf messages.
//
//*****************************************************************************
int32_t
am_bsp_debug_printf_enable(void)
{
    switch (g_ePrintInterface)
    {
        case AM_BSP_PRINT_IF_NONE:
        //
        // Fall on through to default to SWO
        //
        case AM_BSP_PRINT_IF_SWO:
            return am_bsp_itm_printf_enable();

        case AM_BSP_PRINT_IF_UART:
            return am_bsp_uart_printf_enable();

        case AM_BSP_PRINT_IF_BUFFERED_UART:
            return am_bsp_buffered_uart_printf_enable();

// #### INTERNAL BEGIN ####
        case AM_BSP_PRINT_IF_MEMORY:
            return am_bsp_memory_printf_enable(g_psPrintfState);
// #### INTERNAL END ####
        default:
            break;
    }

    return -1;

} // am_bsp_debug_printf_enable()

//*****************************************************************************
//
// Disable the TPIU and ITM for debug printf messages.
//
//*****************************************************************************
void
am_bsp_debug_printf_disable(void)
{
    switch (g_ePrintInterface)
    {
        case AM_BSP_PRINT_IF_SWO:
            am_bsp_itm_printf_disable();
            break;

        case AM_BSP_PRINT_IF_UART:
            am_bsp_uart_printf_disable();
            break;

        case AM_BSP_PRINT_IF_BUFFERED_UART:
            am_bsp_buffered_uart_printf_disable();
            break;

// #### INTERNAL BEGIN ####
        case AM_BSP_PRINT_IF_MEMORY:
            am_bsp_memory_printf_disable();
            break;
// #### INTERNAL END ####

        default:
            break;
    }
} // am_bsp_debug_printf_disable()

//*****************************************************************************
//
// Enable printing over ITM.
//
//*****************************************************************************
int32_t
am_bsp_itm_printf_enable(void)
{
    uint32_t ui32dcuVal;

    //
    // Need to make sure that SWO is enabled
    //
// #### INTERNAL BEGIN ####
    if (MCUCTRL->BOOTLOADER_b.SECBOOTFEATURE == MCUCTRL_BOOTLOADER_SECBOOTFEATURE_ENABLED)
// #### INTERNAL END ####
    {
        if ((PWRCTRL->DEVPWRSTATUS_b.PWRSTCRYPTO == 1) && (CRYPTO->HOSTCCISIDLE_b.HOSTCCISIDLE == 1))
        {
// #### INTERNAL BEGIN ####
            if (MCUCTRL->BOOTLOADER & (MCUCTRL_BOOTLOADER_SBLLOCK_Msk | MCUCTRL_BOOTLOADER_SBRLOCK_Msk))
            {
                am_hal_dcu_set_scope(true);
            }
// #### INTERNAL END ####
            am_hal_dcu_get(&ui32dcuVal);

            //
            // Enable SWO
            //
            if ( ((ui32dcuVal & (AM_HAL_DCU_CPUTRC_TPIU_SWO | AM_HAL_DCU_CPUDBG_NON_INVASIVE)) != (AM_HAL_DCU_CPUTRC_TPIU_SWO | AM_HAL_DCU_CPUDBG_NON_INVASIVE)) &&
                 (am_hal_dcu_update(true, (AM_HAL_DCU_CPUTRC_TPIU_SWO | AM_HAL_DCU_CPUDBG_NON_INVASIVE)) != AM_HAL_STATUS_SUCCESS) )
            {
                //
                // Cannot enable SWO
                //
                return -1;
            }
        }
        else
        {
            //
            // If DCU is not accessible, we cannot determine if ITM can be safely enabled.
            //
            return -1;
        }
    }

    //
    // Set the global print interface.
    //
    g_ePrintInterface = AM_BSP_PRINT_IF_SWO;

    //
    // Enable the ITM interface and the SWO pin.
    //
#ifdef AM_PART_APOLLO5A
    if ( am_hal_tpiu_enable(AM_HAL_TPIU_BAUD_1M) != AM_HAL_STATUS_SUCCESS )
    {
        //
        // Error occurred during the TPIU enable.
        //
        while(1);
    }

    if ( am_hal_itm_enable() != AM_HAL_STATUS_SUCCESS )
    {
        //
        // Error occurred during the ITM enable.
        //
        while(1);
    }
#else   // Need to update the Bronco HAL ITM and TPIU functions.
    am_hal_tpiu_enable(AM_BSP_ITM_FREQUENCY);
    am_hal_itm_enable();
#endif

    am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, g_AM_BSP_GPIO_ITM_SWO);

    //
    // Attach the ITM to the STDIO driver.
    //
    am_util_stdio_printf_init(am_hal_itm_print);

    return 0;

} // am_bsp_itm_printf_enable()

//*****************************************************************************
//
// Disable printing over ITM.
//
//*****************************************************************************
void
am_bsp_itm_printf_disable(void)
{
    if (g_ePrintInterface != AM_BSP_PRINT_IF_SWO)
    {
        return;
    }

    //
    // Disable the ITM/TPIU
    //
#ifdef AM_PART_APOLLO5A
    if ( am_hal_itm_disable() != AM_HAL_STATUS_SUCCESS )
    {
        //
        // Error occurred during the ITM disable.
        //
        while(1);
    }

    if ( am_hal_tpiu_disable() != AM_HAL_STATUS_SUCCESS )
    {
        //
        // Error occurred during the TPIU disable.
        //
        while(1);
    }
#else   // Need to update the Bronco HAL ITM and TPIU functions.
    am_hal_itm_disable();
    am_hal_tpiu_disable();
#endif

    //
    // Detach the ITM interface from the STDIO driver.
    //
    am_util_stdio_printf_init(0);

    //
    // Disconnect the SWO pin
    //
    am_hal_gpio_pinconfig(AM_BSP_GPIO_ITM_SWO, am_hal_gpio_pincfg_disabled);
    g_ePrintInterface = AM_BSP_PRINT_IF_NONE;
} // am_bsp_itm_printf_disable()

//*****************************************************************************
//
// Reset SDIO device via GPIO
//
//*****************************************************************************
void
am_bsp_sdio_reset(uint32_t ui32Module)
{
    switch (ui32Module)
    {
        case 0:
            am_hal_gpio_output_clear(AM_BSP_GPIO_SDIO0_RST);
            am_util_delay_ms(10);
            am_hal_gpio_output_set(AM_BSP_GPIO_SDIO0_RST);
            break;

        case 1:
            am_hal_gpio_output_clear(AM_BSP_GPIO_SDIO1_RST);
            am_util_delay_ms(10);
            am_hal_gpio_output_set(AM_BSP_GPIO_SDIO1_RST);
            break;

        default:
            break;
    }
} // am_bsp_sdio_reset()

//*****************************************************************************
//
// Set up the SDIO's pins
//
//*****************************************************************************
void
am_bsp_sdio_pins_enable(uint32_t ui32Module, uint8_t ui8BusWidth)
{
    switch (ui32Module)
    {
        case 0:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_CMD,  g_AM_BSP_GPIO_SDIO0_CMD);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_CLK,  g_AM_BSP_GPIO_SDIO0_CLK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_RST,  g_AM_BSP_GPIO_SDIO0_RST);

            switch (ui8BusWidth)
            {
                case 8:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT7, g_AM_BSP_GPIO_SDIO0_DAT7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT6, g_AM_BSP_GPIO_SDIO0_DAT6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT5, g_AM_BSP_GPIO_SDIO0_DAT5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT4, g_AM_BSP_GPIO_SDIO0_DAT4);
                case 4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT3, g_AM_BSP_GPIO_SDIO0_DAT3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT2, g_AM_BSP_GPIO_SDIO0_DAT2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT1, g_AM_BSP_GPIO_SDIO0_DAT1);
                case 1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT0, g_AM_BSP_GPIO_SDIO0_DAT0);
                    break;
            }
            break;

        case 1:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_CMD,  g_AM_BSP_GPIO_SDIO1_CMD);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_CLK,  g_AM_BSP_GPIO_SDIO1_CLK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_RST,  g_AM_BSP_GPIO_SDIO1_RST);

            switch (ui8BusWidth)
            {
                case 8:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT7, g_AM_BSP_GPIO_SDIO1_DAT7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT6, g_AM_BSP_GPIO_SDIO1_DAT6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT5, g_AM_BSP_GPIO_SDIO1_DAT5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT4, g_AM_BSP_GPIO_SDIO1_DAT4);
                case 4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT3, g_AM_BSP_GPIO_SDIO1_DAT3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT2, g_AM_BSP_GPIO_SDIO1_DAT2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT1, g_AM_BSP_GPIO_SDIO1_DAT1);
                case 1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT0, g_AM_BSP_GPIO_SDIO1_DAT0);
                    break;
            }
            break;

        default:
            break;
    }

} // am_bsp_sdio_pins_enable()

//*****************************************************************************
//
// Disable the SDIO's pins
//
//*****************************************************************************
void
am_bsp_sdio_pins_disable(uint32_t ui32Module, uint8_t ui8BusWidth)
{
    switch (ui32Module)
    {
        case 0:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_CMD,  am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_CLK,  am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_RST,  am_hal_gpio_pincfg_default);

            switch (ui8BusWidth)
            {
                case 8:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT7, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT6, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT5, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT4, am_hal_gpio_pincfg_default);
                case 4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT3, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT2, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT1, am_hal_gpio_pincfg_default);
                case 1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO0_DAT0, am_hal_gpio_pincfg_default);
                    break;
            }
            break;

        case 1:
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_CMD,  am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_CLK,  am_hal_gpio_pincfg_default);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_RST,  am_hal_gpio_pincfg_default);

            switch (ui8BusWidth)
            {
                case 8:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT7, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT6, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT5, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT4, am_hal_gpio_pincfg_default);
                case 4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT3, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT2, am_hal_gpio_pincfg_default);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT1, am_hal_gpio_pincfg_default);
                case 1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_SDIO1_DAT0, am_hal_gpio_pincfg_default);
                    break;
            }
            break;

        default:
            break;
    }
} // am_bsp_sdio_pins_disable()

//*****************************************************************************
//
// Initialize and configure the UART
//
//*****************************************************************************
int32_t
am_bsp_uart_printf_enable(void)
{
    //
    // Initialize, power up, and configure the communication UART. Use the
    // custom configuration if it was provided. Otherwise, just use the default
    // configuration.
    //
    if (am_hal_uart_initialize(AM_BSP_UART_PRINT_INST, &g_sCOMUART) ||
        am_hal_uart_power_control(g_sCOMUART, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_uart_configure(g_sCOMUART, &g_sBspUartConfig) ||

        //
        // Enable the UART pins.
        //
        am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX) ||
        am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX) )
    {
        return -1;
    }

    //
    // Save the information that we're using the UART for printing.
    //
    g_ePrintInterface = AM_BSP_PRINT_IF_UART;

    //
    // Register the BSP print function to the STDIO driver.
    //
    am_util_stdio_printf_init(am_bsp_uart_string_print);

    return 0;

} // am_bsp_uart_printf_enable()

//*****************************************************************************
//
// Disable the UART
//
//*****************************************************************************
void
am_bsp_uart_printf_disable(void)
{
    if (g_ePrintInterface != AM_BSP_PRINT_IF_UART)
    {
        return;
    }

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
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, am_hal_gpio_pincfg_disabled);
    g_ePrintInterface = AM_BSP_PRINT_IF_NONE;
} // am_bsp_uart_printf_disable()

//*****************************************************************************
//
// UART-based string print function.
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
        .eType = AM_HAL_UART_BLOCKING_WRITE,
        .pui8Data = (uint8_t *) pcString,
        .ui32NumBytes = ui32StrLen,
        .ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER,
        .pui32BytesTransferred = &ui32BytesWritten,
    };

    am_hal_uart_transfer(g_sCOMUART, &sUartWrite);

    // #### INTERNAL BEGIN ####
    //
    // CAJ: We probably need to do a different kind of error check here. The
    // transfer as written above will block forever, so the strings should
    // never ever be different lengths.
    //
    // #### INTERNAL END ####
    if (ui32BytesWritten != ui32StrLen)
    {
        //
        // Couldn't send the whole string!!
        //
        while (1);
    }
} // am_bsp_uart_string_print()

#ifndef AM_BSP_DISABLE_BUFFERED_UART
//*****************************************************************************
//
// Initialize and configure the UART
//
//*****************************************************************************
int32_t
am_bsp_buffered_uart_printf_enable(void)
{
    //
    // Initialize, power up, and configure the communication UART. Use the
    // custom configuration if it was provided. Otherwise, just use the default
    // configuration.
    //
    if (am_hal_uart_initialize(AM_BSP_UART_PRINT_INST, &g_sCOMUART) ||
        am_hal_uart_power_control(g_sCOMUART, AM_HAL_SYSCTRL_WAKE, false) ||
        am_hal_uart_configure(g_sCOMUART, &g_sBspUartConfig) ||
        am_hal_uart_buffer_configure(g_sCOMUART, pui8UartTxBuffer, sizeof(pui8UartTxBuffer), pui8UartRxBuffer, sizeof(pui8UartRxBuffer)) ||

        //
        // Enable the UART pins.
        //
        am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX) ||
        am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX) )
    {
        return -1;
    }

    //
    // Register the BSP print function to the STDIO driver.
    //
    am_util_stdio_printf_init(am_bsp_uart_string_print);

    //
    // Enable the interrupts for the UART.
    //
    NVIC_SetPriority((IRQn_Type)(UART0_IRQn + AM_BSP_UART_PRINT_INST), AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + AM_BSP_UART_PRINT_INST));

    //
    // Set the global print interface.
    //
    g_ePrintInterface = AM_BSP_PRINT_IF_BUFFERED_UART;

    return 0;

} // am_bsp_buffered_uart_printf_enable()

//*****************************************************************************
//
// Disable the UART
//
//*****************************************************************************
void
am_bsp_buffered_uart_printf_disable(void)
{
    if (g_ePrintInterface != AM_BSP_PRINT_IF_BUFFERED_UART)
    {
        return;
    }

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
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, am_hal_gpio_pincfg_disabled);

    //
    // Disable the interrupts for the UART.
    //
    NVIC_DisableIRQ((IRQn_Type)(UART0_IRQn + AM_BSP_UART_PRINT_INST));
    g_ePrintInterface = AM_BSP_PRINT_IF_NONE;
} // am_bsp_buffered_uart_printf_disable()

//*****************************************************************************
//
// Interrupt routine for the buffered UART interface.
//
//*****************************************************************************
void
am_bsp_buffered_uart_service(void)
{
    uint32_t ui32Status;
    am_hal_uart_interrupt_status_get(g_sCOMUART, &ui32Status, true);
    am_hal_uart_interrupt_clear(g_sCOMUART, ui32Status);
    am_hal_uart_interrupt_service(g_sCOMUART, ui32Status);
} // am_bsp_buffered_uart_service()
#endif // AM_BSP_DISABLE_BUFFERED_UART

// #### INTERNAL BEGIN ####
//*****************************************************************************
//
// Print to memory
//
//*****************************************************************************
void
am_bsp_memory_string_print(char *pcString)
{
    uint32_t ui32StrLen = 0;
    uint8_t *pui8Dest = g_psPrintfState->pui8Buffer;
    uint32_t ui32Index = g_psPrintfState->ui32Index;
    uint32_t ui32Size = g_psPrintfState->ui32Size;

    //
    // Measure the length of the string.
    //
    while (pcString[ui32StrLen] != 0)
    {
        ui32StrLen++;
    }

    AM_CRITICAL_BEGIN;

    //
    // Just copy the string into memory.
    //
    uint32_t i = 0;
    while (i < ui32StrLen)
    {
        pui8Dest[ui32Index] = pcString[i];
        ui32Index = (ui32Index + 1) % ui32Size;
        i++;
    }

    g_psPrintfState->ui32Index = ui32Index;

    AM_CRITICAL_END;
} // am_bsp_memory_string_print()

//*****************************************************************************
//
// Enable print to memory
//
//*****************************************************************************
int32_t
am_bsp_memory_printf_enable(am_bsp_memory_printf_state_t *psPrintfState)
{
    //
    // Set the global print interface.
    //
    g_ePrintInterface = AM_BSP_PRINT_IF_MEMORY;

    g_psPrintfState = psPrintfState;
    am_util_stdio_printf_init(am_bsp_memory_string_print);

    return 0;

} // am_bsp_memory_printf_enable()

//*****************************************************************************
//
// Disable print to memory
//
//*****************************************************************************
void
am_bsp_memory_printf_disable(void)
{
    if (g_ePrintInterface != AM_BSP_PRINT_IF_MEMORY)
    {
        return;
    }

    am_util_stdio_printf_init(0);
    g_ePrintInterface = AM_BSP_PRINT_IF_NONE;
} // am_bsp_memory_printf_disable()

// #### INTERNAL END ####
//*****************************************************************************
//
// Set up the IOM pins based on mode and module.
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
        return;
    }

    ui32Combined = ((ui32Module << 2) | eIOMMode);

    switch ( ui32Combined )
    {
#if defined(AM_BSP_GPIO_IOM0_SCK)
        case ((0 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCK,  g_AM_BSP_GPIO_IOM0_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MISO, g_AM_BSP_GPIO_IOM0_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MOSI, g_AM_BSP_GPIO_IOM0_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS,   g_AM_BSP_GPIO_IOM0_CS);
            break;
#endif /* AM_BSP_GPIO_IOM0_SCK */

#if defined(AM_BSP_GPIO_IOM1_SCK)
        case ((1 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCK,  g_AM_BSP_GPIO_IOM1_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MISO, g_AM_BSP_GPIO_IOM1_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MOSI, g_AM_BSP_GPIO_IOM1_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_CS,   g_AM_BSP_GPIO_IOM1_CS);
            break;
#endif /* AM_BSP_GPIO_IOM1_SCK */

#if defined(AM_BSP_GPIO_IOM2_SCK)
        case ((2 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCK,  g_AM_BSP_GPIO_IOM2_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MISO, g_AM_BSP_GPIO_IOM2_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MOSI, g_AM_BSP_GPIO_IOM2_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS,   g_AM_BSP_GPIO_IOM2_CS);
            break;
#endif /* AM_BSP_GPIO_IOM2_SCK */

#if defined(AM_BSP_GPIO_IOM3_SCK)
        case ((3 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCK,  g_AM_BSP_GPIO_IOM3_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MISO, g_AM_BSP_GPIO_IOM3_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MOSI, g_AM_BSP_GPIO_IOM3_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_CS,   g_AM_BSP_GPIO_IOM3_CS);
            break;
#endif /* AM_BSP_GPIO_IOM3_SCK */

#if defined(AM_BSP_GPIO_IOM4_SCK)
        case ((4 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCK,  g_AM_BSP_GPIO_IOM4_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MISO, g_AM_BSP_GPIO_IOM4_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MOSI, g_AM_BSP_GPIO_IOM4_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_CS,   g_AM_BSP_GPIO_IOM4_CS);
            break;
#endif /* AM_BSP_GPIO_IOM4_SCK */

#if defined(AM_BSP_GPIO_IOM5_SCK)
        case ((5 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCK,  g_AM_BSP_GPIO_IOM5_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MISO, g_AM_BSP_GPIO_IOM5_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MOSI, g_AM_BSP_GPIO_IOM5_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_CS,   g_AM_BSP_GPIO_IOM5_CS);
            break;
#endif /* AM_BSP_GPIO_IOM5_SCK */

#if defined(AM_BSP_GPIO_IOM6_SCK)
        case ((6 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCK,  g_AM_BSP_GPIO_IOM6_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MISO, g_AM_BSP_GPIO_IOM6_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MOSI, g_AM_BSP_GPIO_IOM6_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_CS,   g_AM_BSP_GPIO_IOM6_CS);
            break;
#endif /* AM_BSP_GPIO_IOM6_SCK */

#if defined(AM_BSP_GPIO_IOM7_SCK)
        case ((7 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCK,  g_AM_BSP_GPIO_IOM7_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MISO, g_AM_BSP_GPIO_IOM7_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MOSI, g_AM_BSP_GPIO_IOM7_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_CS,   g_AM_BSP_GPIO_IOM7_CS);
            break;
#endif /* AM_BSP_GPIO_IOM7_SCK */

#if defined(AM_BSP_GPIO_IOM0_SCL)
        case ((0 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCL,  g_AM_BSP_GPIO_IOM0_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SDA,  g_AM_BSP_GPIO_IOM0_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM0_SCL */

#if defined(AM_BSP_GPIO_IOM1_SCL)
        case ((1 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCL,  g_AM_BSP_GPIO_IOM1_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SDA,  g_AM_BSP_GPIO_IOM1_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM1_SCL */

#if defined(AM_BSP_GPIO_IOM2_SCL)
        case ((2 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCL,  g_AM_BSP_GPIO_IOM2_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SDA,  g_AM_BSP_GPIO_IOM2_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM2_SCL */

#if defined(AM_BSP_GPIO_IOM3_SCL)
        case ((3 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCL,  g_AM_BSP_GPIO_IOM3_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SDA,  g_AM_BSP_GPIO_IOM3_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM3_SCL */

#if defined(AM_BSP_GPIO_IOM4_SCL)
        case ((4 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCL,  g_AM_BSP_GPIO_IOM4_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SDA,  g_AM_BSP_GPIO_IOM4_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM4_SCL */

#if defined(AM_BSP_GPIO_IOM5_SCL)
        case ((5 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCL,  g_AM_BSP_GPIO_IOM5_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SDA,  g_AM_BSP_GPIO_IOM5_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM5_SCL */

#if defined(AM_BSP_GPIO_IOM6_SCL)
        case ((6 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCL,  g_AM_BSP_GPIO_IOM6_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SDA,  g_AM_BSP_GPIO_IOM6_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM6_SCL */

#if defined(AM_BSP_GPIO_IOM7_SCL)
        case ((7 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCL,  g_AM_BSP_GPIO_IOM7_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SDA,  g_AM_BSP_GPIO_IOM7_SDA);
            break;
#endif /* AM_BSP_GPIO_IOM7_SCL */

        default:
            break;
    }
} // am_bsp_iom_pins_enable()

//*****************************************************************************
//
// Disable the IOM pins based on mode and module.
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
        return;
    }

    ui32Combined = ((ui32Module << 2) | eIOMMode);

    switch ( ui32Combined )
    {
#if defined(AM_BSP_GPIO_IOM0_SCK)
        case ((0 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM0_SCK */

#if defined(AM_BSP_GPIO_IOM1_SCK)
        case ((1 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM1_SCK */

#if defined(AM_BSP_GPIO_IOM2_SCK)
        case ((2 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM2_SCK */

#if defined(AM_BSP_GPIO_IOM3_SCK)
        case ((3 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM3_SCK */

#if defined(AM_BSP_GPIO_IOM4_SCK)
        case ((4 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM4_SCK */

#if defined(AM_BSP_GPIO_IOM5_SCK)
        case ((5 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM5_SCK */

#if defined(AM_BSP_GPIO_IOM6_SCK)
        case ((6 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM6_SCK */

#if defined(AM_BSP_GPIO_IOM7_SCK)
        case ((7 << 2) | AM_HAL_IOM_SPI_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_CS,   am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM7_SCK */

#if defined(AM_BSP_GPIO_IOM0_SCL)
        case ((0 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM0_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM0_SCL */

#if defined(AM_BSP_GPIO_IOM1_SCL)
        case ((1 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM1_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM1_SCL */

#if defined(AM_BSP_GPIO_IOM2_SCL)
        case ((2 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM2_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM2_SCL */

#if defined(AM_BSP_GPIO_IOM3_SCL)
        case ((3 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM3_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM3_SCL */

#if defined(AM_BSP_GPIO_IOM4_SCL)
        case ((4 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM4_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM4_SCL */

#if defined(AM_BSP_GPIO_IOM5_SCL)
        case ((5 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM5_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM5_SCL */

#if defined(AM_BSP_GPIO_IOM6_SCL)
        case ((6 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM6_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM6_SCL */

#if defined(AM_BSP_GPIO_IOM7_SCL)
        case ((7 << 2) | AM_HAL_IOM_I2C_MODE):
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOM7_SDA,  am_hal_gpio_pincfg_disabled);
            break;
#endif /* AM_BSP_GPIO_IOM7_SCL */

        default:
            break;
    }
} // am_bsp_iom_pins_disable()

//*****************************************************************************
//
// Set up the IOS pins based on mode and module.
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
#if defined(AM_BSP_GPIO_IOS_SCK)
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCK,  g_AM_BSP_GPIO_IOS_SCK);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MISO, g_AM_BSP_GPIO_IOS_MISO);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MOSI, g_AM_BSP_GPIO_IOS_MOSI);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_CE,   g_AM_BSP_GPIO_IOS_CE);
#endif /* AM_BSP_GPIO_IOS_SCK */
            break;

        case ((0 << 2) | AM_HAL_IOS_USE_I2C):
#if defined(AM_BSP_GPIO_IOS_SCL)
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCL,  g_AM_BSP_GPIO_IOS_SCL);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SDA,  g_AM_BSP_GPIO_IOS_SDA);
#endif /* AM_BSP_GPIO_IOS_SCL */
            break;

        default:
            break;
    }
} // am_bsp_ios_pins_enable()

//*****************************************************************************
//
// Disable the IOS pins based on mode and module.
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
#if defined(AM_BSP_GPIO_IOS_SCK)
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCK,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MISO, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_MOSI, am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_CE,   am_hal_gpio_pincfg_disabled);
#endif /* AM_BSP_GPIO_IOS_SCK */
            break;

        case ((0 << 2) | AM_HAL_IOS_USE_I2C):
#if defined(AM_BSP_GPIO_IOS_SCL)
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SCL,  am_hal_gpio_pincfg_disabled);
            am_hal_gpio_pinconfig(AM_BSP_GPIO_IOS_SDA,  am_hal_gpio_pincfg_disabled);
#endif /* AM_BSP_GPIO_IOS_SCL */
            break;

        default:
            break;
    }
} // am_bsp_ios_pins_disable()

//*****************************************************************************
//
// Set up the MSPI pins based on the external flash device type with clock
// on data pin 4.
//
//*****************************************************************************
void
am_bsp_mspi_clkond4_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_BSP_GPIO_MSPI0_CE0);
                    //
                    // setting clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4, g_AM_BSP_GPIO_MSPI0_D4_CLK);
                    break;

                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE1, g_AM_BSP_GPIO_MSPI0_CE1);
                    //
                    // setting clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4, g_AM_BSP_GPIO_MSPI0_D4_CLK);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_BSP_GPIO_MSPI1_CE0);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_BSP_GPIO_MSPI1_D4_CLK);
                    break;

                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, g_AM_BSP_GPIO_MSPI1_CE1);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_BSP_GPIO_MSPI1_D4_CLK);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI1_SCK */
            break;
        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4, g_AM_BSP_GPIO_MSPI2_D4_CLK);
                    break;

                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    //
                    // above configuration could not support clock on data 4.
                    //
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, g_AM_BSP_GPIO_MSPI2_CE1);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4, g_AM_BSP_GPIO_MSPI2_D4_CLK);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI2_SCK */
            break;
        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, g_AM_BSP_GPIO_MSPI3_CE0);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4, g_AM_BSP_GPIO_MSPI3_D4_CLK);
                    break;

                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    //
                    // above configuration could not support clock on data 4.
                    //
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE1, g_AM_BSP_GPIO_MSPI3_CE1);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4, g_AM_BSP_GPIO_MSPI3_D4_CLK);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
} // am_bsp_mspi_clkond4_pins_enable()

//*****************************************************************************
//
// Disable the MSPI pins based on the external flash device type with clock
// on data pin 4.
//
//*****************************************************************************
void
am_bsp_mspi_clkond4_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, am_hal_gpio_pincfg_disabled);
                    //
                    // setting clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4, am_hal_gpio_pincfg_disabled);
                    break;

                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE1, am_hal_gpio_pincfg_disabled);
                    //
                    // setting clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4, am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;

        case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, am_hal_gpio_pincfg_disabled);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  am_hal_gpio_pincfg_disabled);
                    break;

                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, am_hal_gpio_pincfg_disabled);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }

#endif /* AM_BSP_GPIO_MSPI1_SCK */
            break;

        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
        switch ( eMSPIDevice )
        {
            case AM_HAL_MSPI_FLASH_QUAD_CE0:
            case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
            case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
            case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, am_hal_gpio_pincfg_disabled);
                //
                // clock on data pin 4.
                //
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4, am_hal_gpio_pincfg_disabled);
                break;

            case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
            case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                //
                // above configuration could not support clock on data 4.
                //
                break;
            case AM_HAL_MSPI_FLASH_QUAD_CE1:
            case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
            case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
            case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
                //
                // clock on data pin 4.
                //
                am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4, am_hal_gpio_pincfg_disabled);
                break;
            default:
                break;
        }
#endif /* AM_BSP_GPIO_MSPI2_SCK */
        break;

        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, am_hal_gpio_pincfg_disabled);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4, am_hal_gpio_pincfg_disabled);
                    break;

                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    //
                    // above configuration could not support clock on data 4.
                    //
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE1, am_hal_gpio_pincfg_disabled);
                    //
                    // clock on data pin 4.
                    //
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4, am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
} // am_bsp_mspi_clkond4_pins_disable()

//*****************************************************************************
//
// Set up the MSPI pins based on the external flash device type.
//
//*****************************************************************************
void
am_bsp_mspi_pins_enable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_BSP_GPIO_MSPI0_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_BSP_GPIO_MSPI0_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, g_AM_BSP_GPIO_MSPI0_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_BSP_GPIO_MSPI0_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_BSP_GPIO_MSPI0_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, g_AM_BSP_GPIO_MSPI0_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
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
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, g_AM_BSP_GPIO_MSPI0_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
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
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, g_AM_BSP_GPIO_MSPI0_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE1, g_AM_BSP_GPIO_MSPI0_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  g_AM_BSP_GPIO_MSPI0_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  g_AM_BSP_GPIO_MSPI0_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  g_AM_BSP_GPIO_MSPI0_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  g_AM_BSP_GPIO_MSPI0_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_BSP_GPIO_MSPI0_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, g_AM_BSP_GPIO_MSPI0_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, g_AM_BSP_GPIO_MSPI0_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  g_AM_BSP_GPIO_MSPI0_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  g_AM_BSP_GPIO_MSPI0_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  g_AM_BSP_GPIO_MSPI0_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  g_AM_BSP_GPIO_MSPI0_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D8,  g_AM_BSP_GPIO_MSPI0_D8);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D9,  g_AM_BSP_GPIO_MSPI0_D9);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D10,  g_AM_BSP_GPIO_MSPI0_D10);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D11,  g_AM_BSP_GPIO_MSPI0_D11);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D12,  g_AM_BSP_GPIO_MSPI0_D12);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D13,  g_AM_BSP_GPIO_MSPI0_D13);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D14,  g_AM_BSP_GPIO_MSPI0_D14);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D15,  g_AM_BSP_GPIO_MSPI0_D15);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_BSP_GPIO_MSPI0_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, g_AM_BSP_GPIO_MSPI0_DQSDM);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQS1DM1, g_AM_BSP_GPIO_MSPI0_DQS1DM1);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE1, g_AM_BSP_GPIO_MSPI0_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  g_AM_BSP_GPIO_MSPI0_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  g_AM_BSP_GPIO_MSPI0_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  g_AM_BSP_GPIO_MSPI0_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  g_AM_BSP_GPIO_MSPI0_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  g_AM_BSP_GPIO_MSPI0_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  g_AM_BSP_GPIO_MSPI0_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  g_AM_BSP_GPIO_MSPI0_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  g_AM_BSP_GPIO_MSPI0_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D8,  g_AM_BSP_GPIO_MSPI0_D8);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D9,  g_AM_BSP_GPIO_MSPI0_D9);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D10,  g_AM_BSP_GPIO_MSPI0_D10);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D11,  g_AM_BSP_GPIO_MSPI0_D11);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D12,  g_AM_BSP_GPIO_MSPI0_D12);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D13,  g_AM_BSP_GPIO_MSPI0_D13);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D14,  g_AM_BSP_GPIO_MSPI0_D14);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D15,  g_AM_BSP_GPIO_MSPI0_D15);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, g_AM_BSP_GPIO_MSPI0_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, g_AM_BSP_GPIO_MSPI0_DQSDM);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQS1DM1, g_AM_BSP_GPIO_MSPI0_DQS1DM1);
                    break;

                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_BSP_GPIO_MSPI1_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, g_AM_BSP_GPIO_MSPI1_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, g_AM_BSP_GPIO_MSPI1_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, g_AM_BSP_GPIO_MSPI1_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, g_AM_BSP_GPIO_MSPI1_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, g_AM_BSP_GPIO_MSPI1_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, g_AM_BSP_GPIO_MSPI1_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, g_AM_BSP_GPIO_MSPI1_DQSDM);
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
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, g_AM_BSP_GPIO_MSPI1_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, g_AM_BSP_GPIO_MSPI1_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_BSP_GPIO_MSPI1_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  g_AM_BSP_GPIO_MSPI1_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  g_AM_BSP_GPIO_MSPI1_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  g_AM_BSP_GPIO_MSPI1_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, g_AM_BSP_GPIO_MSPI1_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, g_AM_BSP_GPIO_MSPI1_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_BSP_GPIO_MSPI1_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  g_AM_BSP_GPIO_MSPI1_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  g_AM_BSP_GPIO_MSPI1_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  g_AM_BSP_GPIO_MSPI1_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, g_AM_BSP_GPIO_MSPI1_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, g_AM_BSP_GPIO_MSPI1_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  g_AM_BSP_GPIO_MSPI1_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  g_AM_BSP_GPIO_MSPI1_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  g_AM_BSP_GPIO_MSPI1_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  g_AM_BSP_GPIO_MSPI1_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  g_AM_BSP_GPIO_MSPI1_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  g_AM_BSP_GPIO_MSPI1_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  g_AM_BSP_GPIO_MSPI1_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  g_AM_BSP_GPIO_MSPI1_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, g_AM_BSP_GPIO_MSPI1_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, g_AM_BSP_GPIO_MSPI1_DQSDM);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI1_SCK */
            break;
        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, g_AM_BSP_GPIO_MSPI2_DQSDM);
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  g_AM_BSP_GPIO_MSPI2_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  g_AM_BSP_GPIO_MSPI2_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  g_AM_BSP_GPIO_MSPI2_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  g_AM_BSP_GPIO_MSPI2_D7);
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, g_AM_BSP_GPIO_MSPI2_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                    break;

                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, g_AM_BSP_GPIO_MSPI2_DQSDM);
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  g_AM_BSP_GPIO_MSPI2_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  g_AM_BSP_GPIO_MSPI2_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  g_AM_BSP_GPIO_MSPI2_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  g_AM_BSP_GPIO_MSPI2_D7);
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  g_AM_BSP_GPIO_MSPI2_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  g_AM_BSP_GPIO_MSPI2_D3);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, g_AM_BSP_GPIO_MSPI2_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  g_AM_BSP_GPIO_MSPI2_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  g_AM_BSP_GPIO_MSPI2_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, g_AM_BSP_GPIO_MSPI2_SCK);
                    break;

                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI2_SCK */
            break;
        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, g_AM_BSP_GPIO_MSPI3_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, g_AM_BSP_GPIO_MSPI3_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, g_AM_BSP_GPIO_MSPI3_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, g_AM_BSP_GPIO_MSPI3_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, g_AM_BSP_GPIO_MSPI3_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, g_AM_BSP_GPIO_MSPI3_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, g_AM_BSP_GPIO_MSPI3_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  g_AM_BSP_GPIO_MSPI3_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  g_AM_BSP_GPIO_MSPI3_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  g_AM_BSP_GPIO_MSPI3_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  g_AM_BSP_GPIO_MSPI3_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, g_AM_BSP_GPIO_MSPI3_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, g_AM_BSP_GPIO_MSPI3_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, g_AM_BSP_GPIO_MSPI3_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  g_AM_BSP_GPIO_MSPI3_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  g_AM_BSP_GPIO_MSPI3_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  g_AM_BSP_GPIO_MSPI3_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  g_AM_BSP_GPIO_MSPI3_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, g_AM_BSP_GPIO_MSPI3_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, g_AM_BSP_GPIO_MSPI3_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE1, g_AM_BSP_GPIO_MSPI3_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  g_AM_BSP_GPIO_MSPI3_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  g_AM_BSP_GPIO_MSPI3_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  g_AM_BSP_GPIO_MSPI3_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  g_AM_BSP_GPIO_MSPI3_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, g_AM_BSP_GPIO_MSPI3_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, g_AM_BSP_GPIO_MSPI3_DQSDM);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, g_AM_BSP_GPIO_MSPI3_CE0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  g_AM_BSP_GPIO_MSPI3_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  g_AM_BSP_GPIO_MSPI3_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  g_AM_BSP_GPIO_MSPI3_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  g_AM_BSP_GPIO_MSPI3_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D8,  g_AM_BSP_GPIO_MSPI3_D8);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D9,  g_AM_BSP_GPIO_MSPI3_D9);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D10,  g_AM_BSP_GPIO_MSPI3_D10);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D11,  g_AM_BSP_GPIO_MSPI3_D11);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D12,  g_AM_BSP_GPIO_MSPI3_D12);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D13,  g_AM_BSP_GPIO_MSPI3_D13);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D14,  g_AM_BSP_GPIO_MSPI3_D14);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D15,  g_AM_BSP_GPIO_MSPI3_D15);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, g_AM_BSP_GPIO_MSPI3_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, g_AM_BSP_GPIO_MSPI3_DQSDM);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQS1DM1, g_AM_BSP_GPIO_MSPI3_DQS1DM1);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE1, g_AM_BSP_GPIO_MSPI3_CE1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  g_AM_BSP_GPIO_MSPI3_D0);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  g_AM_BSP_GPIO_MSPI3_D1);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  g_AM_BSP_GPIO_MSPI3_D2);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  g_AM_BSP_GPIO_MSPI3_D3);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  g_AM_BSP_GPIO_MSPI3_D4);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  g_AM_BSP_GPIO_MSPI3_D5);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  g_AM_BSP_GPIO_MSPI3_D6);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  g_AM_BSP_GPIO_MSPI3_D7);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D8,  g_AM_BSP_GPIO_MSPI3_D8);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D9,  g_AM_BSP_GPIO_MSPI3_D9);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D10,  g_AM_BSP_GPIO_MSPI3_D10);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D11,  g_AM_BSP_GPIO_MSPI3_D11);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D12,  g_AM_BSP_GPIO_MSPI3_D12);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D13,  g_AM_BSP_GPIO_MSPI3_D13);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D14,  g_AM_BSP_GPIO_MSPI3_D14);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D15,  g_AM_BSP_GPIO_MSPI3_D15);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, g_AM_BSP_GPIO_MSPI3_SCK);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, g_AM_BSP_GPIO_MSPI3_DQSDM);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQS1DM1, g_AM_BSP_GPIO_MSPI3_DQS1DM1);
                    break;

                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
} // am_bsp_mspi_pins_enable()


//*****************************************************************************
//
// Disable the MSPI pins based on the external flash device type.
//
//*****************************************************************************
void
am_bsp_mspi_pins_disable(uint32_t ui32Module, am_hal_mspi_device_e eMSPIDevice)
{
    switch (ui32Module)
    {
        case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D8,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D9,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D10,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D11,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D12,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D13,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D14,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D15,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQS1DM1, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D8,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D9,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D10,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D11,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D12,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D13,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D14,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_D15,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQSDM, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI0_DQS1DM1, am_hal_gpio_pincfg_disabled);
                    break;

                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI0_SCK */
            break;
        case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI1_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI1_SCK */
            break;
        case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                    break;

                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_DQSDM, am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D7,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D3,  am_hal_gpio_pincfg_disabled);
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI2_SCK, am_hal_gpio_pincfg_disabled);
                    break;

                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI2_SCK */
            break;
        case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
            switch ( eMSPIDevice )
            {
                case AM_HAL_MSPI_FLASH_SERIAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_SERIAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE0:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_QUAD_CE1:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
                case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_CE1:
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE0, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D8,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D9,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D10,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D11,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D12,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D13,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D14,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D15,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQS1DM1, am_hal_gpio_pincfg_disabled);
                    break;
                case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_CE1, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D0,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D1,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D2,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D3,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D4,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D5,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D6,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D7,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D8,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D9,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D10,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D11,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D12,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D13,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D14,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_D15,  am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_SCK, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQSDM, am_hal_gpio_pincfg_disabled);
                    am_hal_gpio_pinconfig(AM_BSP_GPIO_MSPI3_DQS1DM1, am_hal_gpio_pincfg_disabled);
                    break;

                default:
                    break;
            }
#endif /* AM_BSP_GPIO_MSPI3_SCK */
            break;
        default:
            break;
    }
} // am_bsp_mspi_pins_disable()

//*****************************************************************************
//
//! @brief MSPI CE Pin Config Get
//!
//! This function returns the pinnum and pincfg for the CE of MSPI requested.
//
//*****************************************************************************
void
am_bsp_mspi_ce_pincfg_get(uint32_t ui32Module,
                          am_hal_mspi_device_e eMSPIDevice,
                          uint32_t * pPinnum,
                          am_hal_gpio_pincfg_t * pPincfg)
{
    switch ( eMSPIDevice )
    {
        case AM_HAL_MSPI_FLASH_SERIAL_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE0:
        case AM_HAL_MSPI_FLASH_QUAD_CE0_1_1_4:
        case AM_HAL_MSPI_FLASH_QUAD_CE0_1_4_4:
        case AM_HAL_MSPI_FLASH_OCTAL_CE0:
        case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE0:
        case AM_HAL_MSPI_FLASH_HEX_DDR_CE0:
            switch (ui32Module)
            {
                case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
                    *pPinnum = AM_BSP_GPIO_MSPI0_CE0;
                    *pPincfg = g_AM_BSP_GPIO_MSPI0_CE0;
#endif
                    break;
                case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
                    *pPinnum = AM_BSP_GPIO_MSPI1_CE0;
                    *pPincfg = g_AM_BSP_GPIO_MSPI1_CE0;
#endif
                    break;
                case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
                    *pPinnum = AM_BSP_GPIO_MSPI2_CE0;
                    *pPincfg = g_AM_BSP_GPIO_MSPI2_CE0;
#endif
                    break;
                case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
                    *pPinnum = AM_BSP_GPIO_MSPI3_CE0;
                    *pPincfg = g_AM_BSP_GPIO_MSPI3_CE0;
#endif
                    break;
                default:
                    break;
            }
            break;
        case AM_HAL_MSPI_FLASH_SERIAL_CE1:
        case AM_HAL_MSPI_FLASH_QUAD_CE1:
        case AM_HAL_MSPI_FLASH_QUAD_CE1_1_1_4:
        case AM_HAL_MSPI_FLASH_QUAD_CE1_1_4_4:
        case AM_HAL_MSPI_FLASH_OCTAL_CE1:
        case AM_HAL_MSPI_FLASH_OCTAL_DDR_CE1:
        case AM_HAL_MSPI_FLASH_HEX_DDR_CE1:
            switch (ui32Module)
            {
                case 0:
#if (AM_BSP_GPIO_MSPI0_SCK)
                    *pPinnum = AM_BSP_GPIO_MSPI0_CE1;
                    *pPincfg = g_AM_BSP_GPIO_MSPI0_CE1;
#endif
                    break;
                case 1:
#if (AM_BSP_GPIO_MSPI1_SCK)
                    *pPinnum = AM_BSP_GPIO_MSPI1_CE1;
                    *pPincfg = g_AM_BSP_GPIO_MSPI1_CE1;
#endif
                    break;
                case 2:
#if (AM_BSP_GPIO_MSPI2_SCK)
                    *pPinnum = AM_BSP_GPIO_MSPI2_CE1;
                    *pPincfg = g_AM_BSP_GPIO_MSPI2_CE1;
#endif
                    break;
                case 3:
#if (AM_BSP_GPIO_MSPI3_SCK)
                    *pPinnum = AM_BSP_GPIO_MSPI3_CE1;
                    *pPincfg = g_AM_BSP_GPIO_MSPI3_CE1;
#endif
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
} // am_bsp_mspi_ce_pincfg_get()

//*****************************************************************************
//
// Enable the ETM pins.
//
//*****************************************************************************
void
am_bsp_etm_pins_enable(am_bsp_etm_interface_e etm_interface)
{
    if ( etm_interface == AM_BSP_PDM_ETM )
    {
        am_hal_gpio_pinconfig(AM_HAL_PIN_51_SWTRACE0, g_AM_BSP_GPIO_GRP0_ETM_TRACE0);
        am_hal_gpio_pinconfig(AM_HAL_PIN_52_SWTRACE1, g_AM_BSP_GPIO_GRP0_ETM_TRACE1);
        am_hal_gpio_pinconfig(AM_HAL_PIN_53_SWTRACE2, g_AM_BSP_GPIO_GRP0_ETM_TRACE2);
        am_hal_gpio_pinconfig(AM_HAL_PIN_54_SWTRACE3, g_AM_BSP_GPIO_GRP0_ETM_TRACE3);
        am_hal_gpio_pinconfig(AM_HAL_PIN_50_SWTRACECLK, g_AM_BSP_GPIO_GRP0_ETM_TRACECLK);
        am_hal_gpio_pinconfig(AM_HAL_PIN_55_SWTRACECTL, g_AM_BSP_GPIO_GRP0_ETM_TRACECTL);
    }
    else
    {
        am_hal_gpio_pinconfig(AM_HAL_PIN_1_SWTRACE0, g_AM_BSP_GPIO_GRP1_ETM_TRACE0);
        am_hal_gpio_pinconfig(AM_HAL_PIN_2_SWTRACE1, g_AM_BSP_GPIO_GRP1_ETM_TRACE1);
        am_hal_gpio_pinconfig(AM_HAL_PIN_3_SWTRACE2, g_AM_BSP_GPIO_GRP1_ETM_TRACE2);
        am_hal_gpio_pinconfig(AM_HAL_PIN_4_SWTRACE3, g_AM_BSP_GPIO_GRP1_ETM_TRACE3);
        am_hal_gpio_pinconfig(AM_HAL_PIN_0_SWTRACECLK, g_AM_BSP_GPIO_GRP1_ETM_TRACECLK);
    }
}// am_bsp_etm_pins_enable()

//*****************************************************************************
//
// Disable the ETM pins.
//
//*****************************************************************************
void
am_bsp_etm_pins_disable(am_bsp_etm_interface_e etm_interface)
{
    if ( etm_interface == AM_BSP_GRP0_ETM )
    {
        am_hal_gpio_pinconfig(AM_HAL_PIN_51_SWTRACE0, am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_HAL_PIN_52_SWTRACE1, am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_HAL_PIN_53_SWTRACE2, am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_HAL_PIN_54_SWTRACE3, am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_HAL_PIN_50_SWTRACECLK, am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_HAL_PIN_55_SWTRACECTL, am_hal_gpio_pincfg_disabled);
    }
    else
    {
        am_hal_gpio_pinconfig(AM_HAL_PIN_1_SWTRACE0, am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_HAL_PIN_2_SWTRACE1, am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_HAL_PIN_3_SWTRACE2, am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_HAL_PIN_4_SWTRACE3, am_hal_gpio_pincfg_disabled);
        am_hal_gpio_pinconfig(AM_HAL_PIN_0_SWTRACECLK, am_hal_gpio_pincfg_disabled);
    }
}// am_bsp_etm_pins_disable()

//TODO/ have to remap to new pins for tbird if need to use this feature
#if 0
//*****************************************************************************
//
//! @brief Enable the RPi UART pins.
//!
//! This function configures 2-pins for RPi UART  Signals
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_rpi_uart_pins_enable(void)
{
    am_hal_gpio_pinconfig(AM_HAL_PIN_30_UART0TX, g_AM_BSP_GPIO_RPI_UART_TX);
    am_hal_gpio_pinconfig(AM_HAL_PIN_32_UART0RX, g_AM_BSP_GPIO_RPI_UART_RX);
}// am_bsp_rpi_uart_pins_enable()

//*****************************************************************************
//
//! @brief Disable the RPi UART pins.
//!
//! This function disbales 2-pins for RPi UART  Signals
//!
//! @return None.
//
//*****************************************************************************
void
am_bsp_rpi_uart_pins_disable(void)
{
    am_hal_gpio_pinconfig(AM_HAL_PIN_30_UART0TX, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_HAL_PIN_32_UART0RX, am_hal_gpio_pincfg_disabled);
}// am_bsp_rpi_uart_pins_disable()
#endif
//*****************************************************************************
//
// Enable the PDM0 pins.
//
//*****************************************************************************
void
am_bsp_pdm0_pins_enable(void)
{
    am_hal_gpio_pinconfig(AM_HAL_PIN_51_PDM0_DATA, g_AM_BSP_GPIO_PDM0_DATA);
    am_hal_gpio_pinconfig(AM_HAL_PIN_50_PDM0_CLK, g_AM_BSP_GPIO_PDM0_CLK);
}// am_bsp_pdm0_pins_enable()

//*****************************************************************************
//
// Disable the PDM0 pins.
//
//*****************************************************************************
void
am_bsp_pdm0_pins_disable(void)
{
    am_hal_gpio_pinconfig(AM_HAL_PIN_51_PDM0_DATA, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_HAL_PIN_50_PDM0_CLK, am_hal_gpio_pincfg_disabled);
}// am_bsp_pdm0_pins_disable()

//*****************************************************************************
//
// Enable the I2S0 pins.
//
//*****************************************************************************
void
am_bsp_i2s0_pins_enable(void)
{
    am_hal_gpio_pinconfig(AM_HAL_PIN_12_I2S0_DATA, g_AM_BSP_GPIO_I2S0_DATA);
    am_hal_gpio_pinconfig(AM_HAL_PIN_11_I2S0_CLK, g_AM_BSP_GPIO_I2S0_CLK);
    am_hal_gpio_pinconfig(AM_HAL_PIN_13_I2S0_WS, g_AM_BSP_GPIO_I2S0_WS);
}// am_bsp_i2s0_pins_enable()

//*****************************************************************************
//
// Disable the I2S0 pins.
//
//*****************************************************************************
void
am_bsp_i2s0_pins_disable(void)
{
    am_hal_gpio_pinconfig(AM_HAL_PIN_12_I2S0_DATA, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_HAL_PIN_11_I2S0_CLK, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_HAL_PIN_13_I2S0_WS, am_hal_gpio_pincfg_disabled);
}// am_bsp_i2s0_pins_disable()

//*****************************************************************************
//
// Enable the IOM PSRAM pins.
//
//*****************************************************************************
void
am_bsp_iom_psram_pins_enable(void)
{
    am_hal_gpio_pinconfig(AM_HAL_PIN_35_M4MOSI, g_AM_BSP_GPIO_IOM_PSRAM_MOSI);
    am_hal_gpio_pinconfig(AM_HAL_PIN_36_M4MISO, g_AM_BSP_GPIO_IOM_PSRAM_MISO);
    am_hal_gpio_pinconfig(AM_HAL_PIN_34_M4SCK, g_AM_BSP_GPIO_IOM_PSRAM_SCK);
    am_hal_gpio_pinconfig(AM_HAL_PIN_33_NCE33, g_AM_BSP_GPIO_IOM_PSRAM_CS);
}// am_bsp_iom_psram_pins_enable(void)


//*****************************************************************************
//
// Disable the IOM PSRAM pins.
//
//*****************************************************************************
void
am_bsp_iom_psram_pins_disable(void)
{
    am_hal_gpio_pinconfig(AM_HAL_PIN_35_M4MOSI, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_HAL_PIN_36_M4MISO, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_HAL_PIN_34_M4SCK, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_HAL_PIN_33_NCE33, am_hal_gpio_pincfg_disabled);
}// am_bsp_iom_psram_pins_disable()