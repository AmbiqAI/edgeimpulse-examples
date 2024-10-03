//*****************************************************************************
//
//! @file am_multi_boot_config.h
//!
//! @brief Global bootloader information.
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

#ifndef AM_MULTI_BOOT_CONFIG_H
#define AM_MULTI_BOOT_CONFIG_H

//*****************************************************************************
//
// Secure Boot.
//
//*****************************************************************************
#define MULTIBOOT_SECURE

//*****************************************************************************
//
// Run with flag page.
//
//*****************************************************************************
#define USE_FLAG_PAGE                       1

//*****************************************************************************
//
// Location of the flag page.
//
//*****************************************************************************
#define FLAG_PAGE_LOCATION                 0x00004000

//*****************************************************************************
//
// Max Size of Bootloader.
//
//*****************************************************************************
// The value here must match (at least) with the ROLength restriction imposed at
// bootloader linker configuration
#define MAX_BOOTLOADER_SIZE                0x00003000
// The value here must match (at least) with the RWLength restriction imposed at
// bootloader linker configuration
#define MAX_SRAM_USED                      0x00004000

//*****************************************************************************
//
// Default Override Pin mapped to Button1
//
//*****************************************************************************
#define DEFAULT_OVERRIDE_GPIO              AM_BSP_GPIO_BUTTON1

//*****************************************************************************
//
// I2C Address to use
//
//*****************************************************************************
#define I2C_SLAVE_ADDR                     0x10

//*****************************************************************************
// Definitions below are don't care for multiboot-core
//*****************************************************************************
//*****************************************************************************
//
// Multiboot mode supported.
//
//*****************************************************************************
//#define AM_MULTIBOOT_SUPPORT_IOS // enables support for boot over IOS (SPI/I2C)
#define AM_MULTIBOOT_SUPPORT_UART // enables support for boot over UART



//
// IOS interrupt pin to use
//
#define MULTIBOOT_IOS_INTERRUPT_PIN        4

// Time to wait for host to initiate download over IOS before switching to UART
#define WAIT_IOS_BOOT_MS                    200
#define WAIT_IOS_BOOT_SYSTICK               ((AM_HAL_CLKGEN_FREQ_MAX_HZ/1000)*WAIT_IOS_BOOT_MS)

#define MULTIBOOT_DETECT_BAUDRATE
#define MULTIBOOT_UART_BAUDRATE             115200

#endif // AM_MULTI_BOOT_CONFIG_H
