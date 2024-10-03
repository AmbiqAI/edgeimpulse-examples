//*****************************************************************************
//
//! @file flash_write_advanced.c
//!
//! @brief Flash write example with advanced features and infospace
//!
//! Purpose: This example shows how to modify the internal Flash using HAL flash helper
//! functions.
//!
//! This example works on instance 1 of the Flash, i.e. the portion of the
//! Flash above 512KB. It is intended to execute from instance 0.
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

#include "flash_write_main_block.h"
#include "flash_write_info_block.h"

//##### INTERNAL BEGIN #####
// CAUTION!!!! - RECOVERY Key should not be released in SDK
// TODO: We still need to figure out how to make it available to customers
#define FLASH_RECOVERY_KEY           0x5B3E75BC
//##### INTERNAL END #####

//##### INTERNAL BEGIN #####
#ifndef SRAM_MAX_ADDR_TEST
//##### INTERNAL END #####
uint32_t g_ui32Source[4096]; // 2 8KB pages worth.
//##### INTERNAL BEGIN #####
#endif
//##### INTERNAL END #####

//*****************************************************************************
//
// Main function.
//
//*****************************************************************************
int
main(void)
{
    int32_t i32ErrorFlag = 0;

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
    // Initialize the printf interface for ITM/SWO output.
    //
    am_bsp_itm_printf_enable();

    //
    // Clear the terminal and print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Flash Write Example\n");

    //
    // Erase the whole block of FLASH instance 1 (512KB to 1MB).
    //
    if ( main_block_erase() )
    {
        i32ErrorFlag++;
    }

    //
    // Program a few words in a page in the main block of instance 1.
    // We'll arbitrarily pick 0x00041000 = 260*1024 = (256+4)*1024.
    //
    if ( main_block_pattern_write() )
    {
        i32ErrorFlag++;
    }

    //
    // Erase the page just programmed.
    //
    if ( main_block_page_erase() )
    {
        i32ErrorFlag++;
    }

    //
    // Erase the INFO block.
    //
    if ( info_block_erase() )
    {
        i32ErrorFlag++;
    }

    //
    // Program a few words in the INFO block.
    //
    if ( info_block_pattern_write() )
    {
        i32ErrorFlag++;
    }

    //
    // Erase the INFO block again before the next two steps.
    //
    if ( info_block_erase() )
    {
        i32ErrorFlag++;
    }

    //
    // Write a board name into the INFO block at 128 bytes before the end.
    //
    if ( info_block_write_board_name("CUSTOMER: MY BOARD") )
    {
        i32ErrorFlag++;
    }

    //
    // Write 6 byte BLE Address into the INFO block at 136 bytes before the end.
    //
    if ( info_block_write_ble_address(0x0000123456ABCDEF) )
    {
        i32ErrorFlag++;
    }

    //
    // Disable Flash INFO space erase operations.
    //
    if ( info_block_disable_erase() )
    {
        i32ErrorFlag++;
    }

    //
    // Disable Flash INFO space program operations by quadrant.
    //
    if ( info_block_disable_program(0xA, 0xA) )
    {
        i32ErrorFlag++;
    }

    //
    // Enable flash wipe function in Flash INFO space.
    //
    if ( info_block_enable_flash_wipe() )
    {
        i32ErrorFlag++;
    }

    //
    // Report success or any failures.
    //
    if (i32ErrorFlag)
    {
        am_util_stdio_printf("ERROR: FLASH Write example failure %d\n",
                             i32ErrorFlag);
    }
    else
    {
        am_util_stdio_printf("FLASH Write example successful \n");
// ##### INTERNAL BEGIN #####
#ifdef AM_AUTOMATED_TESTING
        am_util_stdio_printf("Example Test: PASS\n");
#endif
// ##### INTERNAL END #####
    }

    //
    // Verify that ITM has finished printing.
    //
    am_hal_itm_not_busy();


    //
    // Wait at least 20 seconds.
    //
    am_hal_flash_delay(20 * (AM_HAL_CLKGEN_FREQ_MAX_HZ / 3));


// ##### INTERNAL BEGIN #####
    //
    // Unconditionally recover a part that could be bricked.
    // We will never return from this call.
    //
    am_hal_flash_recovery(FLASH_RECOVERY_KEY);
// ##### INTERNAL END #####


    //
    // Provide return code back to the system.
    //
    return i32ErrorFlag;
}
