//*****************************************************************************
//
//! @file multi_boot.c
//!
//! @brief Bootloader program accepting multiple host protocols.
//!
//! This is a bootloader program that supports flash programming over UART,
//! SPI, and I2C. The correct protocol is selected automatically at boot time.
//! The messaging is expected to follow little-endian format, which is native to
//! Apollo1/2.
//!
//! Default override pin corresponds to Button1. So, even if a valid image is
//! present in flash, bootloader can be forced to wait for new image from host.
//!
//! @verbatim
//! PIN fly lead connections assumed by multi_boot:
//!     HOST                                    SLAVE (multi_boot target)
//!     --------                                --------
//!     GPIO[2]  GPIO Interrupt (slave to host) GPIO[4]  GPIO interrupt
//!     GPIO[4]  OVERRIDE pin   (host to slave) GPIO[18] Override pin or n/c
//!     GPIO[5]  IOM0 SPI CLK/I2C SCL           GPIO[0]  IOS SPI SCK/I2C SCL
//!     GPIO[6]  IOM0 SPI MISO/I2C SDA          GPIO[1]  IOS SPI MISO/I2C SDA
//!     GPIO[7]  IOM0 SPI MOSI                  GPIO[2]  IOS SPI MOSI
//!     GPIO[11] IOM0 SPI nCE                   GPIO[3]  IOS SPI nCE
//!     GPIO[17] Slave reset (host to slave)    Reset Pin or n/c
//! Reset and Override pin connections from Host are optional
//! Keeping Button1 pressed on target has same effect as host driving override
//! @endverbatim

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
#include "am_multi_boot_config.h"
#include "am_multi_boot.h"

#if USE_EXTERNAL_FLASH == 1
#include "extflash.h"
#endif //USE_EXTERNAL_FLASH == 1

#ifdef AM_MULTIBOOT_SUPPORT_IOS
//*****************************************************************************
//
// Global variable to let the main loop know whether the IOS interface has been
// used yet.
//
//*****************************************************************************
volatile bool g_bIOSActive = false;

//*****************************************************************************
//
// IO Slave Register Access ISR.
//
//*****************************************************************************
void
am_ioslave_acc_isr(void)
{
    //
    // Make sure the main loop knows that the IOS is currently in use.
    //
    g_bIOSActive = true;

    am_multiboot_ios_acc_isr_handler();
}
#endif

#ifdef AM_MULTIBOOT_SUPPORT_UART
#ifdef MULTIBOOT_DETECT_BAUDRATE
volatile bool g_bUartConfigured = false;
//*****************************************************************************
//
// Setting up a GPIO ISR for the UART autobaud feature.
//
//*****************************************************************************
void
am_gpio_isr(void)
{
    uint32_t ui32BaudRate;

    ui32BaudRate = am_multiboot_uart_detect_baudrate(AM_BSP_GPIO_BOOTLOADER_UART_RX);

    //
    // Now that we have a baud rate, we can configure our UART.
    //
    am_multiboot_setup_serial(AM_BSP_UART_BOOTLOADER_INST, ui32BaudRate);

    //
    // Send a '0x55' to give the boot host some indication that we have the
    // correct baud rate and to let it know that our UART is ready for further
    // traffic.
    //
    am_hal_uart_char_transmit_polled(AM_BSP_UART_BOOTLOADER_INST, 0x55);
    g_bUartConfigured = true;
}
#endif
//*****************************************************************************
//
// UART ISR
//
//*****************************************************************************
void
#if (AM_BSP_UART_BOOTLOADER_INST == 0)
am_uart_isr(void)
#elif (AM_BSP_UART_BOOTLOADER_INST == 1)
am_uart1_isr(void)
#endif
{
    am_multiboot_uart_isr_handler(AM_BSP_UART_BOOTLOADER_INST);
}
#endif

// Need temporary buf equal to one flash sector size (larger of int or ext, if ext flash is supported)
// We accumulate data in this buffer and perform Writes only on page boundaries in
// multiple of page lentghs
#if (USE_EXTERNAL_FLASH == 1) && (AM_DEVICES_SPIFLASH_SECTOR_SIZE > AM_HAL_FLASH_PAGE_SIZE)
#define TEMP_BUFSIZE     AM_DEVICES_SPIFLASH_SECTOR_SIZE
#else
#define TEMP_BUFSIZE     AM_HAL_FLASH_PAGE_SIZE
#endif


uint32_t sramTempBuf[TEMP_BUFSIZE / 4];

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    am_bootloader_image_t *pBootImage;
    bool bOverride = false;
    bool bBootFromFlash = false;
#ifdef AM_MULTIBOOT_SUPPORT_OTA
    uint32_t otaPtrVal = *((uint32_t *)OTA_POINTER_LOCATION);
    uint32_t tempZero = 0;
#endif

#ifdef AM_MULTIBOOT_SUPPORT_IOS
    //
    // Set the (active LOW) interrupt pin so the host knows we don't have a
    // message to send yet.
    //
    am_hal_gpio_out_bit_set(MULTIBOOT_IOS_INTERRUPT_PIN);
    am_hal_gpio_pin_config(MULTIBOOT_IOS_INTERRUPT_PIN, AM_HAL_PIN_OUTPUT);
#endif

    bBootFromFlash = am_multiboot_check_boot_from_flash(&bOverride, &pBootImage);
    if (!bOverride)
    {
#ifdef AM_MULTIBOOT_SUPPORT_OTA
        // Check if OTA available
        if ((otaPtrVal != 0xFFFFFFFF) && (otaPtrVal != 0))
        {
            //
            // Configure the board for low power.
            //
            am_bsp_low_power_init();
            //
            // If we get here, we're going to try to download a new image from a host
            // processor. Speed up the clocks and start turning on peripherals.
            //
            am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);
            // clear the value for subsequent boots
            am_bootloader_write_flash_within_page(OTA_POINTER_LOCATION, &tempZero, 1);
            // function below does not return in case of success

#if USE_EXTERNAL_FLASH == 1
            if (am_multiboot_ota_handler((void *)otaPtrVal, sramTempBuf, sizeof(sramTempBuf) , &g_extFlash) == false)
            {
                if (bBootFromFlash)
                {
                    // We want to run the flash image with clean slate...
                    // So doing a POI here, and the image will be run in the next boot
                    //
                    // Perform a software reset.
                    //
                    am_hal_reset_poi();
                }
            }
#else
            if (am_multiboot_ota_handler((void *)otaPtrVal, sramTempBuf, sizeof(sramTempBuf) , NULL) == false)
            {
                if (bBootFromFlash)
                {
                    // We want to run the flash image with clean slate...
                    // So doing a POI here, and the image will be run in the next boot
                    //
                    // Perform a software reset.
                    //
                    am_hal_reset_poi();
                }
            }
#endif //USE_EXTERNAL_FLASH == 1
        }
#endif
        if (bBootFromFlash)
        {
#ifdef AM_MULTIBOOT_SUPPORT_IOS
            //
            // If everything looks good, disable the interrupt pin and run.
            //
            am_hal_gpio_pin_config(MULTIBOOT_IOS_INTERRUPT_PIN, AM_HAL_PIN_DISABLE);
#endif
            am_bootloader_image_run(pBootImage);
        }
    }
    //
    // If we get here, we're going to try to download a new image from a host
    // processor. Speed up the clocks and start turning on peripherals.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);
    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    //
    // Provide temp buffer for multiboot to operate on
    //
    am_multiboot_init(sramTempBuf, sizeof(sramTempBuf));

#ifdef AM_MULTIBOOT_SUPPORT_IOS
    //
    // Start systick to measure time for the IOS timeout.
    //
    am_hal_systick_load(0x00FFFFFF);
    am_hal_systick_start();

    //
    // Enable the IOS. Choose the correct protocol based on the state of pin 0.
    //
    am_multiboot_setup_ios_interface(MULTIBOOT_IOS_INTERRUPT_PIN);

    //
    // Wait for a few milliseconds to see if anyone will send us data.
    //
    while ( g_bIOSActive
#ifdef AM_MULTIBOOT_SUPPORT_UART
        || am_hal_systick_count() > (0xFFFFFF - WAIT_IOS_BOOT_SYSTICK)
#endif
        )
    {
        //
        // Delay to avoid polling peripheral registers so frequently.
        //
        am_util_delay_ms(1);
    }

    //
    // If we didn't get any IOS packets, we'll move on to the UART option.
    //
    am_multiboot_cleanup_ios_interface();
    am_hal_gpio_pin_config(MULTIBOOT_IOS_INTERRUPT_PIN, AM_HAL_PIN_DISABLE);
#endif

#ifdef AM_MULTIBOOT_SUPPORT_UART
#ifdef MULTIBOOT_DETECT_BAUDRATE
    //
    // Re-Start systick to measure time for autobaud and for the IOS timeout.
    //
    am_hal_systick_stop();
    am_hal_systick_load(0x00FFFFFF);
    am_hal_systick_start();

    //
    // Configure our RX pin as a GPIO input with a falling edge interrupt.
    //
    am_hal_gpio_pin_config(AM_BSP_GPIO_BOOTLOADER_UART_RX, AM_HAL_GPIO_INPUT);
    am_hal_gpio_int_polarity_bit_set(AM_BSP_GPIO_BOOTLOADER_UART_RX, AM_HAL_GPIO_FALLING);

    am_hal_gpio_int_clear(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BOOTLOADER_UART_RX));
    am_hal_gpio_int_enable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BOOTLOADER_UART_RX));
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);

    //
    // Enable interrupts so we can receive messages from the boot host.
    //
    am_hal_interrupt_master_enable();
    while(!g_bUartConfigured)
    {
        // Wait for Baud rate detection
        am_hal_sysctrl_sleep(false);
    }
    // ISR has already configured the UART by now
    am_hal_gpio_int_disable(AM_HAL_GPIO_BIT(AM_BSP_GPIO_BOOTLOADER_UART_RX));
    am_hal_interrupt_disable(AM_HAL_INTERRUPT_GPIO);
#else
    //
    // Now that we have a baud rate, we can configure our UART.
    //
    am_multiboot_setup_serial(AM_BSP_UART_BOOTLOADER_INST, MULTIBOOT_UART_BAUDRATE);
#endif

    //
    // Make sure the UART RX and TX pins are enabled.
    //
    am_hal_gpio_pin_config(AM_BSP_GPIO_BOOTLOADER_UART_TX, AM_BSP_GPIO_CFG_BOOTLOADER_UART_TX);
    am_hal_gpio_pin_config(AM_BSP_GPIO_BOOTLOADER_UART_RX, AM_BSP_GPIO_CFG_BOOTLOADER_UART_RX);
    //
    // Enable interrupts so we can receive messages from the boot host.
    //
    am_hal_interrupt_master_enable();
#endif

    //
    // Loop forever - should never reach here
    //
    while (1)
    {
    }
}
