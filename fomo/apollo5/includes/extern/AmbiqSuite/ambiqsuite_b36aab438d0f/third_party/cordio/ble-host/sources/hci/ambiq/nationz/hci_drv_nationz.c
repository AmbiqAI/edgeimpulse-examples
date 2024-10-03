//*****************************************************************************
//
//! @file hci_drv.c
//!
//! @brief HCI driver interface.
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
#include <stdbool.h>

#include "wsf_types.h"
#include "bstream.h"
#include "wsf_msg.h"
#include "wsf_cs.h"
#include "hci_drv.h"
#include "hci_drv_apollo.h"
#include "hci_tr_apollo.h"
#include "hci_core.h"

//#include "hci_vs_nationz.h"

#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_bsp.h"
#include "am_devices_nationz.h"
#include "hci_drv_nationz.h"

#include "hci_apollo_config.h"

#include <string.h>

uint8_t g_BLEMacAddress[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};

//
// Take over the interrupt handler for whichever IOM we're using.
// #### INTERNAL BEGIN ####
// CAJ: This should probably be an option, not a requirement.
// #### INTERNAL END ####
//
#define HciDrvSpiISR                                                          \
    HciDrvSpiISR2(HCI_DRV_NATIONZ_IOM)
#define HciDrvSpiISR2(n)                                                      \
    HciDrvSpiISR3(n)
#define HciDrvSpiISR3(n)                                                      \
    am_iomaster ## n ## _isr

//*****************************************************************************
//
// Static function prototypes.
//
//*****************************************************************************
static void hciDrvReadCallback(uint8_t *pui8Data, uint32_t ui32Length);

//*****************************************************************************
//
// Boot the radio.
//
//*****************************************************************************
void
HciDrvRadioBoot(uint32_t ui32Module)
{
    //
    // Enable the pins and IOM for the Nationz chip.
    //
    am_devices_nationz_setup();

    //
    // Apply any necessary patches here...
    //

    //
    // Tell the Nationz part that patching is complete.
    //
    am_devices_nationz_patches_complete();

    //
    // Make sure our read callback is appropriately registered.
    //
    am_devices_nationz_set_read_callback(hciDrvReadCallback);

    return;
}

void
HciDrvRadioShutdown(void)
{
}

//*****************************************************************************
//
// Send an HCI message.
//
//*****************************************************************************
uint16_t
hciDrvWrite(uint8_t type, uint16_t len, uint8_t *pData)
{
    am_devices_nationz_write(type, len, pData);

    return len;
}

//*****************************************************************************
//
// Nationz SPI ISR
//
// This function should be called in response to all SPI interrupts during
// operation of the Nationz radio.
//
//*****************************************************************************
void
HciDrvSpiISR(void)
{
    uint32_t ui32Status;

    //
    // Check to see which interrupt caused us to enter the ISR.
    //
    ui32Status = am_hal_iom_int_status_get(HCI_DRV_NATIONZ_IOM, true);

    //
    // Fill or empty the FIFO, and either continue the current operation or
    // start the next one in the queue. If there was a callback, it will be
    // called here.
    //
    am_hal_iom_queue_service(HCI_DRV_NATIONZ_IOM, ui32Status);

    //
    // Clear the interrupts before leaving the ISR.
    //
    am_hal_iom_int_clear(HCI_DRV_NATIONZ_IOM, ui32Status);
}

//*****************************************************************************
//
// GPIO ISR for Nationz data ready.
//
//*****************************************************************************
void
HciDataReadyISR(void)
{
    //
    // Start the HCI read action on the bus. When the data comes back, it will
    // come back through the read callback we set up during boot.
    //
    am_devices_nationz_read();
}

//*****************************************************************************
//
// Function for handling HCI read data from the Nationz chip.
//
//*****************************************************************************
static void
hciDrvReadCallback(uint8_t *pui8Data, uint32_t ui32Length)
{
    //
    // Pass the newly received data along to the Cordio stack.
    //
    hciTrSerialRxIncoming(pui8Data, ui32Length);
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetRfPowerLevelEx
 *
 *  \brief  Vendor-specific command for settting Radio transmit power level
 *          for Nationz.
 *
 *  \param  txPowerlevel    valid range from 0 to 15 in decimal.
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
bool_t HciVsNZ_SetRfPowerLevelEx(txPowerLevel_t txPowerlevel)
{
  uint8_t set_tx_power[3];

  set_tx_power[0] = set_tx_power[1] = 0;
  set_tx_power[2] = txPowerlevel;

  if(txPowerlevel < TX_POWER_LEVEL_INVALID) {
    HciVendorSpecificCmd(0xFC3B, sizeof(set_tx_power), set_tx_power);
    return true;
  }
  else {
    return false;
  }

}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetBDAddr
 *
 *  \brief  Vendor-specific command for settting BlueTooth Device Address
 *          for Nationz.
 *
 *  \param  addr[6]  BD Address
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
bool_t HciVsNZ_SetBDAddr(uint8_t addr[6])
{
  HciVendorSpecificCmd(0xFC32, sizeof(addr), addr);
  return true;
}


