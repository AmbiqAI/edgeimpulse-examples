//*****************************************************************************
//
//! @file usb_ble_bridge.c
//!
//! @brief Converts USB HCI commands to SPI.
//!
//! @addtogroup ble_examples BLE Examples
//!
//! @defgroup usb_ble_bridge BLE USB HCI Example
//! @ingroup ble_examples
//! @{
//!
//! This exapmle can be used as a way to communicate between a host chip using
//! UART HCI and the BLE module inside Apollo3.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "am_devices_cooper.h"
#include "am_util.h"

#include "tusb.h"
#include "dtm_api.h"

//*****************************************************************************
//
// External variables declaration.
//
//*****************************************************************************
extern am_devices_cooper_buffer(1024) g_psWriteData;

//*****************************************************************************
//
// External function declaration.
//
//*****************************************************************************
extern volatile uint32_t g_ui32SerialRxIndex;
extern uint16_t serial_rx_hci_state_machine(uint8_t *pBuf, uint16_t len);

//*****************************************************************************
//
// RX Callback for the USB CDC.
//
//*****************************************************************************
void tud_cdc_rx_cb(uint8_t itf)
{
    uint32_t size = tud_cdc_n_available(itf);
    uint8_t * pData = (uint8_t *) &(g_psWriteData.bytes[g_ui32SerialRxIndex]);

    tud_cdc_n_read(itf, pData, size);
    (void)serial_rx_hci_state_machine(pData, size);
    g_ui32SerialRxIndex += size;
}

//*****************************************************************************
//
// USB CDC Task.
//
//*****************************************************************************
void cdc_task(void)
{

}

//*****************************************************************************
//
// Initialize the USB.
//
//*****************************************************************************
void serial_interface_init(void)
{
    am_util_stdio_printf("Apollo4 USB to SPI Bridge\n");

    tusb_init();
}

//*****************************************************************************
//
// Deinitialize the USB.
//
//*****************************************************************************
void serial_interface_deinit(void)
{

}
//*****************************************************************************
//
// Read USB data.
//
//*****************************************************************************
void serial_data_read(uint8_t* pui8Data, uint32_t* ui32Length)
{

}

//*****************************************************************************
//
// Write USB data.
//
//*****************************************************************************
void serial_data_write(uint8_t* pui8Data, uint32_t ui32Length)
{
    tud_cdc_write(pui8Data, ui32Length);
    tud_cdc_write_flush();
}

//*****************************************************************************
//
// Enable IRQ.
//
//*****************************************************************************
void serial_irq_enable(void)
{

}

//*****************************************************************************
//
// Disable IRQ.
//
//*****************************************************************************
void serial_irq_disable(void)
{

}

//*****************************************************************************
//
// Serial task to adapt different interface, do nothing for UART.
//
//*****************************************************************************
void serial_task(void)
{
    tud_task();
}

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int main(void)
{
    dtm_init();
    dtm_process();
}

//*****************************************************************************
//
// End Doxygen group.
//! @}
//
//*****************************************************************************
