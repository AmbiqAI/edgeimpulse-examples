//*****************************************************************************
//
//! @file exactle_fit_amota_multi_boot.c
//!
//! @brief Bootloader program for AMOTA service.
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
#include "am_devices.h"
#include "am_util.h"
#include "am_bootloader.h"
#include "am_bl_storage.h"
#include "image_boot_handlers.h"

#include "exactle_fit_amota_multi_boot.h"
#include "exactle_fit_amota_multi_boot_config.h"

#ifdef BOOTLOADER_DEBUG
#include "am_util_stdio.h"
#include "am_util_delay.h"
// might need va_list someday, but not so far today.
#define DPRINTF(x) am_util_stdio_printf x
#else
#define DPRINTF(x)
#endif

//*****************************************************************************
//
// UART pin used for AutoBaud
//
//*****************************************************************************
#if defined(AM_PART_APOLLO)
#define UART_RX_PIN                     1
#elif defined(AM_PART_APOLLO2)
#define UART_RX_PIN                     2
#endif

// These have been defined here, as the new bootloader file no longer defines it
// It instead determines these dynamically. Ideally this file should do the same
#define APOLLO2_BOOTLOADER_FLASH_ADDRESS_MAX    (0x100000)

//*****************************************************************************
//
// Message buffers.
//
// Note: The RX buffer needs to be 32-bit aligned to be compatible with the
// flash helper functions, but we also need an 8-bit pointer to it for copying
// data from the IOS interface, which is only 8 bits wide.
//
//*****************************************************************************
uint8_t g_pui8TxBuffer[8];
uint32_t g_pui32RxBuffer[AM_HAL_FLASH_PAGE_SIZE / 4];
uint8_t *g_pui8RxBuffer = (uint8_t *) g_pui32RxBuffer;
uint32_t g_ui32BytesInBuffer = 0;

//*****************************************************************************
//
// Globals to keep track of the image write state.
//
//*****************************************************************************
uint32_t *g_pui32WriteAddress = 0;
uint32_t g_ui32BytesReceived = 0;

volatile bool g_bImageValid = false;

uint32_t g_ui32CRC = 0;

//*****************************************************************************
//
// Global variable to keep track of which interface we're using.
//
//*****************************************************************************
extern volatile bool g_bIOSActive;

//*****************************************************************************
//
// Image structure to hold data about the downloaded boot image.
//
//*****************************************************************************
am_bl_storage_image_t g_sImage =
{
    DEFAULT_LINK_ADDRESS,
    0,
    0,
    DEFAULT_OVERRIDE_GPIO,
    DEFAULT_OVERRIDE_POLARITY,
    0,
    0,
    0,
    BOOT_NO_NEW_IMAGE,
    0,
    0,
    (uint32_t*)0xFFFFFFFF,
    0,
    0,
    0
};

//*****************************************************************************
//
// Flag page information.
//
//*****************************************************************************
am_bl_storage_image_t *g_psBootImage = (am_bl_storage_image_t *) FLAG_PAGE_LOCATION;

void setup_ios_interface(void);
void setup_serial(int32_t i32Module, uint32_t ui32BaudRate);
void cleanup_ios_interface(void);

void boot_from_storage(am_bl_storage_image_t *psImage);
void new_image_check(am_bl_storage_image_t *psImage);

//*****************************************************************************
//
//! @brief Check the flash contents of a new image to make sure it's safe to load.
//!
//! @param psImage - Pointer to the image structure. (should be already updated)
//!
//! This function is used to determine whether the flash contents of a new image
//! is safe to load. (either stored in internal flash or external serial flash.)
//!
//! @return true if the image is safe to load.
//
//*****************************************************************************
bool
image_storage_check(am_bl_storage_image_t *psImage, uint32_t ui32StorageType)
{
    uint32_t ui32ResetVector, ui32StackPointer, ui32LinkAddress; //, ui32StorageAddress;

    ui32LinkAddress = (uint32_t) psImage->pui32LinkAddress;
    DPRINTF(("Entering %s 0x%08x\r\n", __func__, (uintptr_t)psImage));

    //
    // Make sure the link address is in flash.
    //
    if ( ui32LinkAddress > APOLLO2_BOOTLOADER_FLASH_ADDRESS_MAX )
    {
        DPRINTF(("Link address outside of flash. 0x%08x\r\n", ui32LinkAddress));
        return false;
    }

    //
    // Check to see if the image was encrypted. If it was, these tests won't
    // work. We'll need to just skip them.
    //
    if ( psImage->bEncrypted == false )
    {
        ui32StackPointer = psImage->pui32StorageAddressNewImage[0];
        ui32ResetVector = psImage->pui32StorageAddressNewImage[1];
    }
    else
    {
        ui32StackPointer = (uint32_t) psImage->pui32StackPointer;
        ui32ResetVector = (uint32_t) psImage->pui32ResetVector;
    }

    //
    // Make sure the stack is in SRAM.
    //
    if ( ui32StackPointer < 0x10000000 || ui32StackPointer > 0x1FFFFFFF )
    {
        DPRINTF(("Stack not in SRAM 0x%08x\r\n", ui32StackPointer));
        return false;
    }

    //
    // Make sure the reset vector points somewhere in the image.
    //
    if ( ui32ResetVector < ui32LinkAddress ||
         ui32ResetVector > ui32LinkAddress + psImage->ui32NumBytes)
    {
        DPRINTF(("Reset Vector not in image 0x%08x\r\n", ui32ResetVector));
        return false;
    }

    //
    // If the image isn't encrypted, run a CRC32.
    //
    if ( psImage->bEncrypted == false )
    {
        if ( ui32StorageType == BOOT_NEW_IMAGE_INTERNAL_FLASH )
        {
            //
            // Run a CRC on the image to make sure it matches the stored checksum
            // value.
            //
            if ( am_bootloader_fast_crc32(psImage->pui32StorageAddressNewImage, psImage->ui32NumBytes ) !=
                 psImage->ui32CRC)
            {
                DPRINTF(("Bad CRC 0x%08x\r\n", psImage->ui32CRC));
                return false;
            }
        }
        else if ( ui32StorageType == BOOT_NEW_IMAGE_EXTERNAL_FLASH )
        {
            //read data from external serial flash and perform CRC check here
            //this is done in boot_from_storage()
        }
    }

    //
    // If those tests pass, we're probably safe to run.
    //
    return true;
}

//*****************************************************************************
//
//! @brief Boot from the stored image.
//!
//! @param psImage - Pointer to the image structure.
//!
//! This function is used to boot from a valid stored image (either stored in
//! internal flash or external serial flash.)
//!
//
//*****************************************************************************
void boot_from_storage(am_bl_storage_image_t *psImage)
{
    if ( psImage->ui32Options == BOOT_NEW_IMAGE_INTERNAL_FLASH )
    {
        //
        // Verify image stored in the internal flash
        //
        if ( !image_storage_check(psImage, BOOT_NEW_IMAGE_INTERNAL_FLASH) )
        {
            // Invalid image
            return;
        }
        //
        // Load image to target link address in the internal flash
        //
        image_load_from_internal_flash(psImage->pui32LinkAddress, psImage->pui32StorageAddressNewImage, psImage->ui32NumBytes);

        //
        // Verify the flash operation result with a fast CRC check before we jump
        // into the new image and run
        //
        if ( am_bootloader_fast_crc32(psImage->pui32LinkAddress, psImage->ui32NumBytes ) !=
             psImage->ui32CRC)
        {
            DPRINTF(("Bad CRC 0x%08x\r\n", psImage->ui32CRC));
            return;
        }
    }
    else if ( psImage->ui32Options == BOOT_NEW_IMAGE_EXTERNAL_FLASH )
    {

#if defined(AM_PART_APOLLO2)
        //
        // Power on SPI
        //
        am_hal_iom_pwrctrl_enable(AM_BSP_FLASH_IOM);
#endif
        //
        // Enable serial interface
        // initialize spi interface with external flash
        //
        am_hal_iom_config(AM_BSP_FLASH_IOM, &g_sIOMConfig);

        //
        // configure pins for iom interface
        //
        configure_spiflash_pins();

        //
        // Initialize the spiflash driver with the IOM information for the second
        // flash device.
        //
        am_devices_spiflash_init(&g_sSpiFlash);

        //
        // Verify image stored in the external flash by checking CRC
        // Set up IOM1 SPI pins and turn on the IOM for this operation.
        //
        am_bsp_iom_spi_pins_enable(AM_BSP_FLASH_IOM);
        am_hal_iom_enable(AM_BSP_FLASH_IOM);

        //
        // Read from spi flash and calculate CRC32 using global buffer
        //
        uint32_t ui32CRC = 0;
        for ( uint16_t i = 0; i < (psImage->ui32NumBytes / AM_HAL_FLASH_PAGE_SIZE); i++ )
        {
            am_devices_spiflash_read((uint8_t*)g_ui32FlashLoadingBuffer,
                ((uint32_t)(psImage->pui32StorageAddressNewImage) + i*AM_HAL_FLASH_PAGE_SIZE),
                AM_HAL_FLASH_PAGE_SIZE);

            am_bootloader_partial_crc32((uint8_t*)g_ui32FlashLoadingBuffer, AM_HAL_FLASH_PAGE_SIZE, &ui32CRC);
        }

        uint32_t ui32Remainder = psImage->ui32NumBytes % AM_HAL_FLASH_PAGE_SIZE;
        if ( ui32Remainder )
        {
            am_devices_spiflash_read((uint8_t*)g_ui32FlashLoadingBuffer,
                ((uint32_t)(psImage->pui32StorageAddressNewImage) + psImage->ui32NumBytes - ui32Remainder), ui32Remainder);

            am_bootloader_partial_crc32(g_ui32FlashLoadingBuffer, ui32Remainder, &ui32CRC);
        }

        //
        // Disable IOM SPI pins and turn off the IOM for this operation.
        //
        am_bsp_iom_spi_pins_disable(AM_BSP_FLASH_IOM);
        am_hal_iom_disable(AM_BSP_FLASH_IOM);

#if defined(AM_PART_APOLLO2)
        am_hal_iom_power_off_save(AM_BSP_FLASH_IOM);
#endif

        if ( ui32CRC != psImage->ui32CRC )
        {
            // image stored invalid
            DPRINTF(("Invalid image in external flash."));
            return;
        }

        //
        // Load image to target link address in the internal flash
        //
        image_load_from_external_flash(psImage->pui32LinkAddress, (uint32_t)(psImage->pui32StorageAddressNewImage), psImage->ui32NumBytes);

        //
        // Verify the flash operation result
        //
        if ( am_bootloader_fast_crc32(psImage->pui32LinkAddress, psImage->ui32NumBytes ) !=
               psImage->ui32CRC)
        {
            DPRINTF(("Bad CRC 0x%08x\r\n", psImage->ui32CRC));
            return;
        }
    }

    //
    // Update the flash flag page
    //
    am_bl_storage_image_t FlagImage;
    FlagImage.pui32LinkAddress = psImage->pui32LinkAddress;
    FlagImage.ui32NumBytes = psImage->ui32NumBytes;
    FlagImage.ui32CRC = psImage->ui32CRC;
    FlagImage.ui32OverrideGPIO = psImage->ui32OverrideGPIO;
    FlagImage.ui32OverridePolarity = psImage->ui32OverridePolarity;
    FlagImage.pui32StackPointer = psImage->pui32StackPointer;
    FlagImage.pui32ResetVector = psImage->pui32ResetVector;
    FlagImage.bEncrypted = psImage->bEncrypted;
    FlagImage.ui32Options = BOOT_NO_NEW_IMAGE;
    FlagImage.pui32StorageAddressNewImage = (uint32_t*)0xFFFFFFFF;

    am_bl_storage_flag_page_update(&FlagImage, (uint32_t *)psImage);

}

//*****************************************************************************
//
//! @brief Checks a boot image to make sure it's safe to run.
//!
//! @param psImage - Pointer to the image structure.
//!
//! This function is used to determine whether a boot image is safe to run. It
//! verifies that the starting stack-pointer and reset vector entries for the
//! new image are reasonable. If these tests pass, it also runs a CRC-32 on the
//! image and checks the result against the expected CRC-32 value from the
//! image structure.
//!
//! The image structure can also specify a GPIO to manually override this test.
//! If the GPIO state matches the state specified in the structure, this test
//! will mark the image as "unsafe" regardless of the other conditions.
//!
//! @return true if the image is safe to run.
//
//*****************************************************************************
void
new_image_check(am_bl_storage_image_t *psImage)
{
    //
    // Check boot option to see whether there is a new image available to boot
    //
    if ( psImage->ui32Options != BOOT_NO_NEW_IMAGE )
    {
        //double check the value
        if ( (psImage->ui32Options == BOOT_NEW_IMAGE_INTERNAL_FLASH ) ||
             (psImage->ui32Options == BOOT_NEW_IMAGE_EXTERNAL_FLASH) )
        {
            //
            // There is a new image available to boot from
            //
            boot_from_storage(psImage);
        }
    }
}

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the (active LOW) interrupt pin so the host knows we don't have a
    // message to send yet.
    //
    am_hal_gpio_out_bit_set(INTERRUPT_PIN);
    am_hal_gpio_pin_config(INTERRUPT_PIN, AM_HAL_PIN_OUTPUT);

#ifdef BOOTLOADER_DEBUG
    // init debug session
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);
    am_bsp_pin_enable(ITM_SWO);
    am_hal_itm_enable();
    am_bsp_debug_printf_enable();
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Ambiq OTA Demo Boot\r\n");
#endif

    //
    // If we're using a flag page, we can run a full CRC check to verify the
    // integrity of our image. If not, we'll just check the override pin.
    //
    if ( USE_FLAG_PAGE )
    {
        //
        // If the user selected to use the last page of flash as the flag page, we
        // need to configure for that now.
        //
#if USE_LAST_PAGE_FOR_FLAG
        am_hal_mcuctrl_device_t device;

        //
        // Read the MCU registers to find our flash size.
        //
        am_hal_mcuctrl_device_info_get(&device);

        //
        // Set the boot image location based on the flash size.
        //
        if ( device.ui32FlashSize != 0 )
        {
            g_psBootImage = (am_bl_storage_image_t *)(device.ui32FlashSize - AM_HAL_FLASH_PAGE_SIZE);
        }
        else
        {
            g_psBootImage = (am_bl_storage_image_t *)(AM_HAL_FLASH_TOTAL_SIZE - AM_HAL_FLASH_PAGE_SIZE);
        }
#endif

        //
        // Check whether there is a new image stored in internal/external flash
        // If yes, load this image into its target execution range
        //
        // Override pin may bypass this operation.
        //
        if ( (g_psBootImage->ui32OverrideGPIO == 0xFFFFFFFF)        ||
             (g_psBootImage->ui32OverridePolarity == 0xFFFFFFFF)    ||
             (am_hal_bootloader_override_check((am_bootloader_image_t *)g_psBootImage) == false) )
        {
            new_image_check(g_psBootImage);
        }
        //
        // Check the flag page to see if there's a valid image ready. We do this
        // before any system initialization so we can minimize the number of
        // configuration options we need to undo before calling the application.
        //
        // In this case, we only have the interrupt pin to un-configure before we
        // can safely run the main application.
        //
        if ( am_bootloader_image_check((am_bootloader_image_t *)g_psBootImage) )
        {
            am_hal_gpio_pin_config(INTERRUPT_PIN, AM_HAL_PIN_DISABLE);
            am_bootloader_image_run((am_bootloader_image_t *)g_psBootImage);
        }
    }
    else
    {
        //
        // Check the override pin to make sure the host isn't requesting an
        // upgrade, and do a quick check to make sure an image actually exists
        // at the default application location.
        //
        if ( am_hal_bootloader_override_check((am_bootloader_image_t *)(&g_sImage)) == false &&
             *(g_sImage.pui32LinkAddress) != 0xFFFFFFFF)
        {
            //
            // If everything looks good, disable the interrupt pin and run.
            //
            am_hal_gpio_pin_config(INTERRUPT_PIN, AM_HAL_PIN_DISABLE);
            am_bootloader_image_run((am_bootloader_image_t *)(&g_sImage));
        }
    }


    //
    // If we get here, we're going to try to download a new image from a host
    // processor. Speed up the clocks and start turning on peripherals.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Enable interrupts so we can receive messages from the boot host.
    //
    am_hal_interrupt_master_enable();

    //
    // Loop forever.
    //
    while (1)
    {
    }
}
