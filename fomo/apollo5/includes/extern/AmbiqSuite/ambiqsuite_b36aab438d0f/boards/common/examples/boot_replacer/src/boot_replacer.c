//*****************************************************************************
//
//! @file boot_replacer.c
//!
//! @brief Bootloader program accepting multiple host protocols.
//!
//! This is a bootloader program that supports flash programming over UART,
//! SPI, and I2C. The correct protocol is selected automatically at boot time.
//!
//! SWO is configured in 1M baud, 8-n-1 mode.
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
#include "am_bootloader.h"

#include "boot_image.h"

uint32_t *g_pui32Destination = 0x0;

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    uint32_t i;
    bool bImagesMatch;

    //
    // Set the clock frequency.
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
    // Don't program an image if it will overwrite this one.
    //
    if (BOOT_IMAGE_LENGTH > AM_HAL_FLASH_PAGE_SIZE)
    {
        while(1);
    }

    //
    // Check to see if we've already programmed this image in. (This should
    // prevent an infinite loop between a bootloader with an inactive override
    // mechanism.
    //
    bImagesMatch = true;

    for (i = 0; i < BOOT_IMAGE_LENGTH; i++)
    {
        if (g_pui32BootImage[i] != g_pui32Destination[i])
        {
            bImagesMatch = false;
            break;
        }
    }

    //
    // If the images we should stop the program here, so we don't end up in an
    // endless loop of reprogramming ourselves.
    //
    while (bImagesMatch == true);

    //
    // Erase page zero. All of our current bootloaders fit into page zero.
    //
    am_hal_flash_page_erase(AM_HAL_FLASH_PROGRAM_KEY, 0, 0);

    //
    // Program the entire image in one go.
    //
    am_hal_flash_program_main(AM_HAL_FLASH_PROGRAM_KEY, g_pui32BootImage, 0x0,
                              BOOT_IMAGE_LENGTH);

    //
    // Trigger a RESET.
    //
    am_hal_reset_poi();

    //
    // Loop forever.
    //
    while(1)
    {
    }
}
