//*****************************************************************************
//
//! @file ble_freertos_throughput.c
//!
//! @brief PacketCraft BLE - Ambiq Micro Throughput Example.
//!
//! Purpose: "This example is primarily used to test BLE data throughput.
//! By default, it broadcasts with the name 'ambiq_BLT_tester'.
//! If you want to test throughput, you need to install the 'Ambiq BLE Test.apk' application on your phone(it only supports Android currently), typically found in the 'tools\ble_throughput_app' directory of the SDK.
//! 1. Scan for devices: Open the application and enter the 'BLE Device Scan' interface to start scanning.
//! 2. Connecting to the device: After scanning and finding the corresponding 'Ambiq-BLE-test' device, click on the 'STOP' button in the menu bar to stop scanning,
//!    then click on the device name to establish the connection.
//! 3. Configure parameters: Once the device is successfully connected, you can configure MTU/PHY/Bonding from the options menu in the top right corner.
//! 4. Start testing: Depending on your needs, you can perform Downlink/Uplink/Bidirectional tests. Bidirectional testing involves transmitting Uplink data first, followed by Downlink transmission.
//!    The number of packets to be transmitted can be configured in the code.
//!    The default 'High Speed' mode refers to a connection interval of 15ms and a latency of 0. If unchecked, it switches to 'Slow Speed' mode with a connection interval of 125ms and a latency of 2.
//!    The 'Dynamic Interval' option refers to switching between 'High Speed' and 'Slow Speed' modes, which is only effective for Bidirectional testing.
//!    'AutoConnect' is used for automatic disconnect/connect testing, allowing you to set the duration before disconnecting after connection.

//! AM_DEBUG_PRINTF
//! WSF_TRACE_ENABLED=1
//!
//! If enabled, debug messages will be sent over ITM at 1M Baud.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

//*****************************************************************************
//
// This application has a large number of common include files. For
// convenience, we'll collect them all together in a single header and include
// that everywhere.
//
//*****************************************************************************
#include "ble_freertos_throughput.h"
#include "rtos.h"

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    //  Enable the I-Cache and D-Cache.
    //
    am_hal_cachectrl_icache_enable();
    am_hal_cachectrl_dcache_enable(true);

    //
    // Initialize the printf interface for ITM output
    //
    if (am_bsp_debug_printf_enable())
    {
        // Cannot print - so no point proceeding
        while(1);
    }

    //
    // Initialize plotting interface.
    //
    am_util_stdio_printf("FreeRTOS Throughput Example\n");

    //
    // Start the tasks.
    //
    run_tasks();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

