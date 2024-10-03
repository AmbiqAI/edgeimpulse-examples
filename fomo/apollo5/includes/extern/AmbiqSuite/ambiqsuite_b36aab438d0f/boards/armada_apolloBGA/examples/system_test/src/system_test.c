//*****************************************************************************
//
//! @file system_test.c
//!
//! @brief Configures the adxl362 to sample at 400Hz and set its watermark to
//! 130 triplets. When the adxl362 FIFO hits its watermark, it's int2 line
//! rises causing the Apollo MCU to interrupt and begin draining the FIFO while
//! sleeping and periodically waking to empty the internal IOM.
//!
//! Note: No ITM debug printing takes place in this example.
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
#include "am_devices.h"
#include "am_util.h"

#include "dsps_image.h"

//*****************************************************************************
//
// Forward declarations.
//
//*****************************************************************************
void test_complete(bool bPass);
void am_iomaster1_isr(void);
void am_gpio_isr(void);
void am_uart_isr(void);
void adxl_data_handler(void);
void adxl_read_start(void);
void start_adxl(void);
void configure_adxl_pins(void);
void configure_radio_pins(void);
void configure_radio_uart(void);
void radio_uart_enable(void);
void radio_uart_disable(void);
void send_radio_samples(void);
void boot_radio(void);

//*****************************************************************************
//
// Test options.
//
//*****************************************************************************
#define NUM_TRIALS                          300
#define NUM_SAMPLES                         99

//*****************************************************************************
//
// Macro for retrieving the axis number from a 16-bit ADXL sample.
//
//*****************************************************************************
#define ADXL362_AXIS(x)                     (((x) & 0x0000C000) >> 14)

//*****************************************************************************
//
// Buffer for holding ADXL samples.
//
//*****************************************************************************
#define SAMPLE_BUFFER_SIZE                              NUM_SAMPLES
static am_devices_adxl362_sample(SAMPLE_BUFFER_SIZE)    g_psSampleBuffer;

//*****************************************************************************
//
// UART buffers for the radio.
//
//*****************************************************************************
uint8_t g_pui8TxArray[1024];
uint8_t g_pui8RxArray[1024];

//*****************************************************************************
//
// Flags to alert the main application of various interrupt conditions.
//
//*****************************************************************************
volatile bool g_bADXLFifoFull = false;
volatile bool g_bIOMIdle = true;
volatile bool g_bUARTIdle = true;

//*****************************************************************************
//
// Counters to see how much data we've pulled from the ADXL
//
//*****************************************************************************
volatile uint32_t g_ui32DataCount = 0;

//*****************************************************************************
//
// Configuration structure for the UART
//
//*****************************************************************************
am_hal_uart_config_t g_sUartConfig =
{
    .ui32BaudRate = 112000,
    .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
    .bTwoStopBits = false,
    .ui32Parity   = AM_HAL_UART_PARITY_NONE,
    .ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE
};

//*****************************************************************************
//
// Configuration structure for the IO Master.
//
//*****************************************************************************
am_hal_iom_config_t g_sIOMConfig =
{
    .ui32InterfaceMode = AM_HAL_IOM_SPIMODE,
    .ui32ClockFrequency = AM_HAL_IOM_1MHZ,
    .bSPHA = 0,
    .bSPOL = 0,
    .ui8WriteThreshold = 4,
    .ui8ReadThreshold = 60,
};

//*****************************************************************************
//
// Configuration structure for the ADXL
//
//*****************************************************************************
am_devices_adxl362_t g_sADXL =
{
    .ui32IOMModule = 1,
    .ui32ChipSelect = 2,
    .ui32Samples = NUM_SAMPLES,
    .ui32SampleRate = AM_DEVICES_ADXL362_400HZ,
    .ui32Range = AM_DEVICES_ADXL362_2G,
    .bHalfBandwidth = false,
    .bSyncMode = false,
};

//*****************************************************************************
//
// Indicate that the test is complete.
//
//*****************************************************************************
void
test_complete(bool bPass)
{
    if ( bPass )
    {
        //
        // Turn off LED2 to show a pass.
        //
        am_hal_gpio_out_bit_set(AM_BSP_GPIO_LED2);
    }

    //
    // Turn off LED 1 to show that the test is finished.
    //
    am_hal_gpio_out_bit_set(AM_BSP_GPIO_LED1);

    //
    // Halt.
    //
    while(1);
}

//*****************************************************************************
//
// Interrupt handler for IOM1
//
//*****************************************************************************
void
am_iomaster1_isr(void)
{
    uint32_t ui32IntStatus;

    //
    // Read and clear the interrupt status.
    //
    ui32IntStatus = am_hal_iom_int_status_get(1, false);
    am_hal_iom_int_clear(1, ui32IntStatus);

    //
    // Service FIFO interrupts as necessary, and call IOM callbacks as
    // transfers are completed.
    //
    am_hal_iom_int_service(1, ui32IntStatus);
}

//*****************************************************************************
//
// Interrupt handler for the GPIO pins.
//
//*****************************************************************************
void
am_gpio_isr(void)
{
    uint64_t ui64Status;

    //
    // Read and clear the GPIO interrupt status.
    //
    ui64Status = am_hal_gpio_int_status_get(false);
    am_hal_gpio_int_clear(ui64Status);

    //
    // Check to make sure that this is the correct interrupt, and set a flag to
    // alert the base-level application that the ADXL has data.
    //
    if ( ui64Status & AM_HAL_GPIO_BIT(34) )
    {
        g_bADXLFifoFull = true;
    }
}

//*****************************************************************************
//
// Interrupt handler for the UART
//
//*****************************************************************************
void
am_uart_isr(void)
{
    uint32_t ui32Status;

    //
    // Read the masked interrupt status from the UART.
    //
    ui32Status = am_hal_uart_int_status_get(AM_BSP_UART_PRINT_INST, true);

    //
    // Service the buffered UART.
    //
    am_hal_uart_service_buffered(AM_BSP_UART_PRINT_INST, ui32Status);

    //
    // Clear the UART interrupts.
    //
    am_hal_uart_int_clear(AM_BSP_UART_PRINT_INST, ui32Status);

    //
    // If the transmit fifo is empty, wait until the UART isn't busy anymore,
    // and then shut it off.
    //
    if ( am_hal_uart_flags_get(AM_BSP_UART_PRINT_INST ) &
         AM_REG_UART_FR_TXFE_XMTFIFO_EMPTY)
    {
        while(am_hal_uart_flags_get(AM_BSP_UART_PRINT_INST) &
              AM_REG_UART_FR_BUSY_BUSY);

        radio_uart_disable();
        g_bUARTIdle = true;
    }
}


//*****************************************************************************
//
// Function to handle incoming data from the ADXL.
//
//*****************************************************************************
void
adxl_data_handler(void)
{
    uint32_t i, ui32ExpectedAxis;

    //
    // The IOM transfer is complete, so we can disable the interface.
    //
    g_bIOMIdle = true;
    am_hal_iom_disable(1);

    //
    // Loop through the just-received ADXL samples to see if the axis markers
    // are in the right order.
    //
    ui32ExpectedAxis = 0;
    for (i = 0; i < NUM_SAMPLES; i++ )
    {
        //
        // Check the axis marker.
        //
        if ( ADXL362_AXIS(g_psSampleBuffer.samples[i]) != ui32ExpectedAxis )
        {
            //
            // The axis didn't match, so mark the test case as a failure.
            //
            test_complete(false);
        }
        else
        {
            //
            // Passed. Move on to the next sample and the next axis.
            //
            ui32ExpectedAxis = (ui32ExpectedAxis + 1) % 3;
        }
    }

    send_radio_samples();

    //
    // If we process the pre-determined number of ADXL samples without any axis
    // alignment issues, mark the test case as a PASS.
    //
    g_ui32DataCount++;
//    if ( g_ui32DataCount >= NUM_TRIALS )
//    {
//        test_complete(true);
//    }
}

//*****************************************************************************
//
// Turn on the IOM, read a set of measurements from the ADXL, and then turn off
// the IOM again.
//
//*****************************************************************************
void
adxl_read_start(void)
{
    //
    // Enable the IOM so we can talk to the ADXL.
    //
    g_bIOMIdle = false;
    am_hal_iom_enable(1);

    //
    // Start a SPI read command to retrieve the samples from the ADXL
    //
    am_hal_iom_int_clear(1, AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR);
    am_hal_iom_int_enable(1, AM_HAL_IOM_INT_CMDCMP | AM_HAL_IOM_INT_THR);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_IOMASTER1);

    //
    // We expect to get a full batch of 130 triplets (or 390 samples). Call the
    // adxl_data_handler when the samples finally arrive.
    //
    am_devices_adxl362_sample_get(&g_sADXL,
                                  NUM_SAMPLES, g_psSampleBuffer.words,
                                  adxl_data_handler);
}

//*****************************************************************************
//
// Perform all of the tasks necessary to initialize the ADXL and put it in
// measurement mode.
//
//*****************************************************************************
void
start_adxl(void)
{
    //
    // Enable the IOM so we can talk to the ADXL.
    //
    am_hal_iom_enable(1);

    //
    // Set ADXL registers to the default settings from the driver (asynchronous
    // mode) and set "measurement mode" to get the ADXL to start taking
    // samples.
    //
    am_devices_adxl362_config(&g_sADXL);

    am_devices_adxl362_measurement_mode_set(&g_sADXL);

    //
    // Wait until the data has actually gone out over the SPI lines.
    //
    am_hal_iom_poll_complete(1);

    //
    // Disable the IOM to save power. We won't need it again until the ADXL
    // actually has data.
    //
    am_hal_iom_disable(1);
}

//*****************************************************************************
//
// Configure GPIOs for communicating with the ADXL
//
//*****************************************************************************
void
configure_adxl_pins(void)
{
    //
    // Set up IOM1 SPI pins.
    //
    am_hal_gpio_pin_config(8, AM_HAL_PIN_8_M1SCK | AM_HAL_GPIO_HIGH_DRIVE);
    am_hal_gpio_pin_config(9, AM_HAL_PIN_9_M1MISO);
    am_hal_gpio_pin_config(10, AM_HAL_PIN_10_M1MOSI);
    am_hal_gpio_pin_config(37, AM_HAL_PIN_37_M1nCE2);

    //
    // Set up ADXL interrupt pins.
    //
    am_hal_gpio_pin_config(34, AM_HAL_PIN_INPUT);

    //
    // Enable a GPIO interrupt for positive edges on ADXL's INT1 pin.
    //
    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(34));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(34));

    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);
}

//*****************************************************************************
//
// Configure the necessary pins for communication with the radio.
//
//*****************************************************************************
void
configure_radio_pins(void)
{
    //
    // Enable the necessary pins for talking to the radio.
    //
    am_hal_gpio_pin_config(0, AM_HAL_PIN_0_UARTTX);
    am_hal_gpio_pin_config(1, AM_HAL_PIN_1_UARTRX);
    //am_hal_gpio_pin_config(3, AM_HAL_PIN_3_PSOURCE);   // DIALOG_POWER
    am_hal_gpio_pin_config(17, AM_HAL_PIN_OUTPUT);     // DIALOG_RESET
}

//*****************************************************************************
//
// Configure the necessary pins for communication with the radio.
//
//*****************************************************************************
void
configure_radio_uart(void)
{
    //
    // Enable the clock to the UART.
    //
    am_hal_uart_clock_enable(AM_BSP_UART_PRINT_INST);

    //
    // Disable and configure the UART.
    //
    am_hal_uart_disable(AM_BSP_UART_PRINT_INST);
    am_hal_uart_config(AM_BSP_UART_PRINT_INST, &g_sUartConfig);


    //
    // Enable the UART.
    //
    am_hal_uart_enable(AM_BSP_UART_PRINT_INST);
}

//*****************************************************************************
//
// Configure the necessary pins for communication with the radio.
//
//*****************************************************************************
void
radio_uart_enable(void)
{
    //
    // Enable the clock to the UART.
    //
    am_hal_uart_clock_enable(AM_BSP_UART_PRINT_INST);
    am_hal_uart_config(AM_BSP_UART_PRINT_INST, &g_sUartConfig);
    am_hal_uart_enable(AM_BSP_UART_PRINT_INST);
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART);
    am_hal_uart_int_enable(AM_BSP_UART_PRINT_INST, AM_HAL_UART_INT_TX);
}

//*****************************************************************************
//
// Configure the necessary pins for communication with the radio.
//
//*****************************************************************************
void
radio_uart_disable(void)
{
    //
    // Disable the clock to the UART.
    //
    am_hal_uart_disable(AM_BSP_UART_PRINT_INST);
    am_hal_uart_clock_disable(AM_BSP_UART_PRINT_INST);
}

//*****************************************************************************
//
// Send the current set of radio samples out over UART.
//
//*****************************************************************************
void
send_radio_samples(void)
{
    //
    // Tell the base-level that the UART is now in use.
    //
    g_bUARTIdle = false;

    //
    // Make sure the radio is on.
    //
    radio_uart_enable();

    //
    // Send over a set of adxl samples.
    //
    am_util_stdio_printf("A:%d,%d,%d\r",
                         AM_DEVICES_ADXL362_VALUE(g_psSampleBuffer.samples[0]),
                         AM_DEVICES_ADXL362_VALUE(g_psSampleBuffer.samples[0]),
                         AM_DEVICES_ADXL362_VALUE(g_psSampleBuffer.samples[0]));
}

//*****************************************************************************
//
// Run the dialog bootloader.
//
//*****************************************************************************
void
boot_radio(void)
{
    uint32_t ui32NumTries, ui32Status;

    //
    // Turn on the radio, and hold it in RESET.
    //
    am_hal_gpio_out_bit_set(3);

    //
    // Try to boot the radio. If it doesn't work the first few times, give up.
    //
    for ( ui32NumTries = 0; ui32NumTries < 10; ui32NumTries++ )
    {
        //
        // Pulse the reset line.
        //
        am_hal_gpio_out_bit_set(17);
        am_util_delay_us(100);
        am_hal_gpio_out_bit_clear(17);

        //
        // Run the boot algorithm.
        //
        ui32Status = am_devices_da14580_uart_boot(g_pui8DspsImage,
                                                  DSPS_IMAGE_LENGTH,
                                                  AM_BSP_UART_PRINT_INST);

        //
        // If we succeeded, we can stop.
        //
        if ( ui32Status == 0 )
        {
            return;
        }
    }

    //
    // If we weren't able to boot the radio, mark this test run as a failure.
    //
    test_complete(false);
}

//*****************************************************************************
//
// Main.
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the system clock to 24 MHz.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_24MHZ);


    //
    // Enable internal buck converters and turn off the voltage comparator.
    //
    am_hal_pwrctrl_bucks_enable();
    am_hal_vcomp_disable();

    //
    // SRAM bank power setting.
    //
    am_hal_mcuctrl_sram_power_set(AM_HAL_MCUCTRL_SRAM_POWER_DOWN_1 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_2 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_3 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_4 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6,
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_1 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_2 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_3 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_4 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6);

    //
    // Flash bank power set.
    //
    am_hal_mcuctrl_flash_power_set(AM_HAL_MCUCTRL_FLASH_POWER_DOWN_1);

    //
    // Disable the bandgap.
    //
    am_hal_mcuctrl_bandgap_disable();

    //
    // Power down the XT OSC (autmatic once no peripherals are using it), and
    // enable autocalibration of the HFRC.
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_LFRC);
    am_hal_clkgen_hfrc_adjust_enable(AM_REG_CLKGEN_HFADJ_HFWARMUP_2SEC,
                                     AM_REG_CLKGEN_HFADJ_HFADJCK_4SEC);

    //
    // Enable an LED so we can see whether we're awake or asleep
    //
    //am_hal_gpio_pin_config(AM_BSP_GPIO_LED3, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(AM_BSP_GPIO_LED3);

    //
    // Configure two more LEDs to indicate pass/fail for the test case.
    //
    //am_hal_gpio_pin_config(AM_BSP_GPIO_LED1, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(AM_BSP_GPIO_LED1);
    //am_hal_gpio_pin_config(AM_BSP_GPIO_LED2, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(AM_BSP_GPIO_LED2);

    //
    // Configure peripherals for talking with the ADXL.
    //
    am_hal_iom_config(1, &g_sIOMConfig);
    configure_adxl_pins();
    am_hal_iom_disable(1);

    //
    // Configure peripherals for talking with the radio.
    //
    configure_radio_pins();
    configure_radio_uart();

    //
    // Boot the radio, and prepare for buffered UART operation.
    //
    boot_radio();
    am_hal_uart_init_buffered(AM_BSP_UART_PRINT_INST, g_pui8RxArray, 1024,
                              g_pui8TxArray, 1024);
    am_util_stdio_printf_init((am_util_stdio_print_char_t)
                              am_hal_uart_string_transmit_buffered);

    radio_uart_disable();

    //
    // Configure the ADXL, and start the data-taking process.
    //
    start_adxl();

    //
    // Loop forever, waiting for events.
    //
    while(1)
    {
        //
        // Disable interrupts temporarily. We need to make sure that the global
        // flags aren't changing while we are checking them.
        //
        am_hal_interrupt_master_disable();

        if ( g_bADXLFifoFull && g_bIOMIdle )
        {
            //
            // Check to see if the ADXL's interrupt line is high. If it is, we
            // need to start a SPI transfer to pull out any available data.
            //
            // We will go to sleep after starting the IOM operation and
            // wake up when it is finished.
            //
            adxl_read_start();
            g_bADXLFifoFull = false;
        }

        //
        // Turn off the indicator LED before we go to sleep mode.
        //
        am_hal_gpio_out_bit_set(AM_BSP_GPIO_LED3);

        //
        // Pick a sleep mode. If the IOM is active, we will need to wake up
        // again very soon, so we will use normal sleep mode. Otherwise, we
        // should use deep sleep.
        //
        if ( g_bIOMIdle && g_bUARTIdle )
        {
            //
            // Disable the UART and IOM pins for deep sleep mode.
            //
            am_hal_gpio_pin_config(0, AM_HAL_PIN_DISABLE);
            am_hal_gpio_pin_config(1, AM_HAL_PIN_DISABLE);
            am_hal_gpio_pin_config(8, AM_HAL_PIN_DISABLE);
            am_hal_gpio_pin_config(9, AM_HAL_PIN_DISABLE);
            am_hal_gpio_pin_config(10, AM_HAL_PIN_DISABLE);
            am_hal_gpio_pin_config(37, AM_HAL_PIN_DISABLE);

            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);

            am_hal_gpio_pin_config(0, AM_HAL_PIN_0_UARTTX);
            am_hal_gpio_pin_config(1, AM_HAL_PIN_1_UARTRX);
            am_hal_gpio_pin_config(8, AM_HAL_PIN_8_M1SCK | AM_HAL_GPIO_HIGH_DRIVE);
            am_hal_gpio_pin_config(9, AM_HAL_PIN_9_M1MISO);
            am_hal_gpio_pin_config(10, AM_HAL_PIN_10_M1MOSI);
            am_hal_gpio_pin_config(37, AM_HAL_PIN_37_M1nCE2);
        }
        else
        {
            am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_NORMAL);
        }

        //
        // Turn the indicator LED on as we exit sleep.
        //
        am_hal_gpio_out_bit_clear(AM_BSP_GPIO_LED3);

        //
        // Re-enable interrupts so we can handle any new events that have come
        // in.
        //
        am_hal_interrupt_master_enable();
    }
}
