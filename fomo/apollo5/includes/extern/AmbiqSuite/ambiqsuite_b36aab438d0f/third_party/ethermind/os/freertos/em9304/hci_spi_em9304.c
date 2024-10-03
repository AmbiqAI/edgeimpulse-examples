//*****************************************************************************
//
//! @file hci_spi_em9304.c
//!
//! @brief This File contains the routines to Transmit and Receive Data from
//! to the EM9304 via the SPI Transport Layer for the BlueLitE/Ethermind BLE
//! host stack.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

/* ----------------------------------------- Header File Inclusion */
#include "hci_spi_internal.h"

#ifdef BT_SPI

#include "am_util.h"
#include "am_devices_em9304.h"
#include "hci_apollo_config.h"
#include "hci_drv_em9304.h"
#include "em9304_patches.h"
#include "em9304_init.h"

#ifndef HCI_APOLLO_MAC
#define HCI_APOLLO_MAC                  {0x01, 0x00, 0x00, 0xEE, 0xF3, 0x0C}
#endif

// HCI_APOLLO_USE_CUSTOMER_OWN_MAC should be defined to true if customer
// supply their own BD address.

#ifndef HCI_APOLLO_USE_CUSTOMER_OWN_MAC
#define HCI_APOLLO_USE_CUSTOMER_OWN_MAC   false
#endif

DECL_STATIC BT_THREAD_RETURN_TYPE hci_spi_read_task (BT_THREAD_ARGS args);


/* ----------------------------------------- External Global Variables */
void hci_transport_enqueue (UCHAR * data, UINT16 datalen);

/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */

//
// Mac address for the EM.
//
static uint8_t g_pui8BLEMacAddress[6] = HCI_APOLLO_MAC;

//
// Static record of the EM9304 vendor specific events
//
g_EMVSEvent_t g_EMVendorSpecificEvents = {0,0,0,0,0,0,0};

/* SPI Read Task State */
DECL_STATIC UCHAR hci_spi_state;

/* The size of the buffer used to receive characters from the SPI driver. */
#define TRANSPORT_BUFFER_SIZE          HCI_TRANSPORT_MAX_PACKET_SIZE

/* The buffer provided to the SPI driver to store incoming data */
static uint8_t spi_drv_rx_buffer[TRANSPORT_BUFFER_SIZE] = {0};

#ifdef HCI_SPI_COLLECT_AND_WR_COMPLETE_PKT
/* Buffer for writing HCI Commands & ACL Data packets */
DECL_STATIC UCHAR hci_spi_wr_buf [HCI_SPI_WR_BUF_SIZE];
#endif /* HCI_SPI_COLLECT_AND_WR_COMPLETE_PKT */

/* SPI Read Task Synchronization */
BT_DEFINE_MUTEX (hci_spi_mutex)
BT_DEFINE_COND (hci_spi_cond)

/* SPI OverLap Synchronization */
BT_DEFINE_MUTEX (hci_spi_read)
BT_DEFINE_MUTEX (hci_spi_write)

#ifdef HAVE_HCI_TX_RX_BYTE_COUNT
/* Number of bytes Transmitted */
DECL_STATIC UINT32 hci_spi_tx_byte_count;

/* Number of bytes Received */
DECL_STATIC UINT32 hci_spi_rx_byte_count;
#endif /* HAVE_HCI_TX_RX_BYTE_COUNT */

#ifdef HAVE_HCI_TX_RX_BYTE_COUNT
/* Macro to increament Tx and Rx byte count */
#define HCI_SPI_INC_TX_BYTES(nbytes) hci_spi_tx_byte_count += (nbytes)
#define HCI_SPI_INC_RX_BYTES(nbytes) hci_spi_rx_byte_count += (nbytes)
#else
#define HCI_SPI_INC_TX_BYTES(nbytes)
#define HCI_SPI_INC_RX_BYTES(nbytes)
#endif /* HAVE_HCI_TX_RX_BYTE_COUNT */

/* ----------------------------------------- Functions */

/** HCI-SPI Initialization */
void hci_spi_init (void)
{
    BT_thread_type tid;

    BT_MUTEX_INIT_VOID (hci_spi_mutex, TRANSPORT);
    BT_COND_INIT_VOID(hci_spi_cond, TRANSPORT);

    /* Create Manual Events for Overlap Mode */
    BT_COND_INIT_VOID(hci_spi_read, TRANSPORT);
    BT_COND_INIT_VOID(hci_spi_write, TRANSPORT);

    /* Create a thread to receive data From Serial PORT and BUFFER it */
    if (0 != BT_thread_create(&tid, NULL, hci_spi_read_task, NULL))
    {
        HCI_SPI_ERR(
        "[HCI-SPI] Could NOT Create SPI Read Thread\n");

        return;
    }

    /* Initialize SPI State */
    hci_spi_state = 0x0;

    HCI_SPI_TRC(
    "[HCI-SPI] SPI Power On Initialization Complete\n");

    return;
}

//
// EM9304 GPIO Interrupt Handler.
//
void hci_spi_gpio_int_hander(void)
{
  BaseType_t xHigherPriorityTaskWoken;

  xHigherPriorityTaskWoken = pdFALSE;

	// Send an event to the HCI Read Task.
	//
	// hci_spi_signal(hci_spi_cond);
	xSemaphoreGiveFromISR(hci_spi_cond,&xHigherPriorityTaskWoken);

  //
  // If the GPS task is higher-priority than the context we're currently
  // running from, we should yield now and run the radio task.
  //
  if (xHigherPriorityTaskWoken != pdFAIL)
  {
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
  }

}


/** HCI-SPI Bluetooth-ON Initialization */
void hci_spi_bt_init(void)
{
    uint32_t patch_dest_memory = DEST_MEMORY_IRAM;

    uint32_t  ui32PN;

#ifdef HAVE_HCI_TX_RX_BYTE_COUNT
    /* Initialize Number of Rx and Tx bytes */
    hci_spi_tx_byte_count = 0;
    hci_spi_rx_byte_count = 0;
#endif /* HAVE_HCI_TX_RX_BYTE_COUNT */

    /* Signal SPI Read Task */
    hci_spi_lock (hci_spi_mutex);
    hci_spi_state = 0x1;
    hci_spi_signal (hci_spi_cond);
    hci_spi_unlock (hci_spi_mutex);

    // disable interrupt during EM9304 initialization.
    am_devices_em9304_disable_interrupt();

    //
    // Enable the radio pins.
    //
#ifdef HCI_APOLLO_POWER_PIN
    am_hal_gpio_pin_config(HCI_APOLLO_POWER_PIN, HCI_APOLLO_POWER_CFG);
    am_hal_gpio_out_bit_clear(HCI_APOLLO_POWER_PIN);
    am_util_delay_ms(100);
    am_hal_gpio_out_bit_set(HCI_APOLLO_POWER_PIN);
    am_util_delay_ms(100);
#endif


    //
    // Device identification
    //
    ui32PN = AM_REG(MCUCTRL, CHIP_INFO)  &
             AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_PN_M;

    // Currently only enable this for Apollo2-Blue
    if (ui32PN == AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLOBL)
    {
      am_hal_gpio_pin_config(30, AM_HAL_GPIO_OUTPUT);
      am_hal_gpio_out_bit_clear(30);
      am_hal_gpio_pin_config(35, AM_HAL_GPIO_OUTPUT);
      am_hal_gpio_out_bit_clear(35);
      am_hal_gpio_pin_config(36, AM_HAL_GPIO_OUTPUT);
      am_hal_gpio_out_bit_clear(36);

    }

    //
    // Assert RESET to the EM9304 device.
    //
    am_hal_gpio_pin_config(HCI_APOLLO_RESET_PIN, AM_HAL_GPIO_OUTPUT);
    am_hal_gpio_out_bit_clear(HCI_APOLLO_RESET_PIN);

    //
    // Setup SPI interface for EM9304
    //
    am_devices_em9304_config_pins();
    am_devices_em9304_spi_init(g_sEm9304.ui32IOMModule, &g_sEm9304IOMConfigSPI);

    //
    // Need to set the interrupt priority to allow SYSCALL
    //
    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_GPIO, configMAX_SYSCALL_INTERRUPT_PRIORITY);

    //
    // Register the GPIO interrupt handler.
    //
    am_hal_gpio_int_register(AM_BSP_GPIO_EM9304_INT,hci_spi_gpio_int_hander);

    //
    // Enable BLE data ready interrupt
    //
    am_hal_interrupt_enable(AM_HAL_INTERRUPT_GPIO);

    //
    // Enable the IOM and GPIO interrupt handlers.
    //
    am_hal_gpio_out_bit_set(HCI_APOLLO_RESET_PIN);

    //
    // Delay for 20ms to make sure the em device gets ready for commands.
    //
    am_util_delay_ms(20);

    //
    // Initialize the EM9304.
    //
    patch_dest_memory = initEM9304();

    // delay here to make sure EM9304 is ready for operation after
    // patch is loaded, otherwise first HCI command will be irgnored.
    am_util_delay_ms(20);


    if (patch_dest_memory == DEST_MEMORY_OTP)
    {

      //
      // Completed the patching process by cycling RESET to the EM9304 device.
      //
      am_hal_gpio_pin_config(HCI_APOLLO_RESET_PIN, AM_HAL_GPIO_OUTPUT);
      am_hal_gpio_out_bit_clear(HCI_APOLLO_RESET_PIN);
      am_util_delay_ms(20);
      am_hal_gpio_out_bit_set(HCI_APOLLO_RESET_PIN);

      // delay here to make sure EM9304 is ready for operation after
      // patch is loaded.
      am_util_delay_ms(20);

    }

    //
    // Set the MAC address if customer provides their own BD address
    //
    if (HCI_APOLLO_USE_CUSTOMER_OWN_MAC)
    {
      //
      // currently just use the default one, customer either update g_pui8BLEMacAddress
      // directly or call HciDrvAssignBDAddress() with a pointer of assigned BD address.
      HciDrvAssignBDAddress(g_pui8BLEMacAddress);
    }

    // enable interrupt after EM9304 initialization is done.
    am_devices_em9304_enable_interrupt();

    HCI_SPI_TRC(
    "[HCI-SPI] SPI Bluetooth-ON Initialization Complete\n");

    return;
}


/** HCI-SPI Bluetooth-OFF Shutdown */
void hci_spi_bt_shutdown (void)
{
    uint32_t  ui32PN;

    /* Signal SPI Read Task */
    hci_spi_lock (hci_spi_mutex);
    hci_spi_state = 0x0;
    hci_spi_unlock (hci_spi_mutex);

    am_devices_em9304_disable_interrupt();
    AM_HAL_GPIO_MASKCREATE(GpioIntMask);
    am_hal_gpio_int_clear(AM_HAL_GPIO_MASKBIT(pGpioIntMask, AM_BSP_GPIO_EM9304_INT));

    am_hal_gpio_pin_config(AM_BSP_GPIO_EM9304_CS, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(AM_BSP_GPIO_EM9304_CS);

    am_hal_clkgen_clkout_disable();

    //
    // Device identification
    //
    ui32PN = AM_REG(MCUCTRL, CHIP_INFO)  &
             AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_PN_M;

    if (ui32PN == AM_UTIL_MCUCTRL_CHIP_INFO_PARTNUM_APOLLOBL)
    {
        // Currently clear pin 24 only for Apollo2-Blue
        am_hal_gpio_pin_config(24, AM_HAL_PIN_24_CLKOUT);
        am_hal_gpio_out_bit_clear(24);
    }

    am_hal_gpio_pin_config(HCI_APOLLO_RESET_PIN, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(HCI_APOLLO_RESET_PIN);


    am_hal_gpio_pin_config(AM_BSP_GPIO_EM9304_INT, AM_HAL_PIN_OUTPUT);
    am_hal_gpio_out_bit_clear(AM_BSP_GPIO_EM9304_INT);

}

// Suppress the lack of return statement.  Function loops forever.
#if defined(__GNUC_STDC_INLINE__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif
/** HCI-SPI Read Task */
DECL_STATIC BT_THREAD_RETURN_TYPE hci_spi_read_task (BT_THREAD_ARGS args)
{
    UINT32      ui32HCIPacketSize = 0;

    BT_LOOP_FOREVER()
    {
        hci_spi_lock (hci_spi_mutex);

        if ((0x0 == hci_spi_state) || (0x1 == hci_spi_state))
        {
            HCI_SPI_TRC(
            "[HCI-SPI] SPI Read Task: Waiting to Service ..\n");

            hci_spi_wait (hci_spi_cond, hci_spi_mutex);

            HCI_SPI_TRC(
            "[HCI-SPI] SPI Read Task: Ready to Service ..\n");

            //
            // Turn on the IOM for this operation.
            //
            am_devices_em9304_spi_awake(g_sEm9304.ui32IOMModule);

            ui32HCIPacketSize = am_devices_em9304_block_read(&g_sEm9304, (uint32_t *)spi_drv_rx_buffer, 0);

            //
            // Disable IOM SPI pins and turn off the IOM after operation
            //
            am_devices_em9304_spi_sleep(g_sEm9304.ui32IOMModule);

            // Check for EM9304 Vendor Specific events and record them.
            if ( (ui32HCIPacketSize > 3) && (0x04 == spi_drv_rx_buffer[0]) &&
            (0xFF == spi_drv_rx_buffer[1]) && (0x01 == spi_drv_rx_buffer[2]))
            {
                switch(spi_drv_rx_buffer[3])
                {
                    case 0x01:
                        g_EMVendorSpecificEvents.EM_ActiveStateEntered++;
                        am_util_debug_printf("Received EM_ActiveStateEntered Event\n");
                        break;
                    case 0x03:
                        g_EMVendorSpecificEvents.EM_TestModeEntered++;
                        am_util_debug_printf("Received EM_TestModeEntered Event\n");
                        break;
                    case 0x04:
                        g_EMVendorSpecificEvents.EM_HalNotification++;
                        am_util_debug_printf("Received EM_HalNotification Event\n");
                        break;
                    default:
                        am_util_debug_printf("Received Unknown Vendor Specific Event from EM9304\n");
                        break;
                }

                //
                // Reset the packet size to 0 so that this packet will not be processed by the host stack.
                //
                ui32HCIPacketSize = 0;
            }
        }

        hci_spi_unlock (hci_spi_mutex);

		//
		// Parse the data segment and send it to the core stack when packet is complete.
		//
        if (ui32HCIPacketSize > 0)
        {
            /* Deliver Data */
            hci_transport_enqueue ((uint8_t *)spi_drv_rx_buffer, (UINT16)ui32HCIPacketSize);
            // see if there's more packet in BLE controller or not.
            hci_spi_signal(hci_spi_cond);
        }
    }

}
// Restore default warnings.
#if defined(__GNUC_STDC_INLINE__)
#pragma GCC diagnostic pop
#endif


/** HCI-SPI Send Data */
API_RESULT hci_spi_send_data
           (UCHAR type, UCHAR * buf, UINT16 length, UCHAR flag)
{
    static INT32 total_len = 0;
    static INT32 cur_len = 0;
    static UCHAR acl_data_pkt = BT_FALSE;

    if (flag)
    {
        if (HCI_ACL_DATA_PACKET == type)
        {
            acl_data_pkt = BT_TRUE;
            total_len = ((buf[3] << 8) | buf[2]) + 5;
        }
        else
        {
            total_len = length + 1;
        }

        if (total_len > HCI_SPI_WR_BUF_SIZE)
        {
            HCI_SPI_ERR(
            "[HCI-SPI] HCI Packet Size %d exceeds Configuration %d bytes\n",
            total_len, HCI_SPI_WR_BUF_SIZE);

            return API_FAILURE;
        }
    }

    if (acl_data_pkt)
    {
        if (0 == cur_len)
        {
            hci_spi_wr_buf[0] = type;
            cur_len = 1;
        }

        BT_mem_copy (hci_spi_wr_buf + cur_len, buf, length);
        cur_len += length;

        if (cur_len != total_len)
        {
            return API_SUCCESS;
        }
    }
    else
    {
        hci_spi_wr_buf[0] = type;
        BT_mem_copy (hci_spi_wr_buf + 1, buf, length);
        total_len = length + 1;
    }

    // add mutex around access of em9304 since Mindtree
    // stack has read and write taskes in parallel access
    // to EM9304.

    hci_spi_lock (hci_spi_mutex);

    //
    // Turn on the IOM for this operation.
    //
    am_devices_em9304_spi_awake(g_sEm9304.ui32IOMModule);

	//
	// Write the HCI packet.
	//
    am_devices_em9304_block_write(&g_sEm9304, hci_spi_wr_buf[0], &hci_spi_wr_buf[1], total_len - 1);

    //
    // Disable IOM SPI pins and turn off the IOM after operation
    //
    am_devices_em9304_spi_sleep(g_sEm9304.ui32IOMModule);

    hci_spi_unlock (hci_spi_mutex);

    /* Re-initialize */
    cur_len = total_len = 0;
    if (BT_FALSE != acl_data_pkt)
    {
        acl_data_pkt = BT_FALSE;
    }

    return API_SUCCESS;
}

/*************************************************************************************************/
/*!
 *  \fn     HciVsSetRfPowerLevelEx
 *
 *  \brief  Vendor-specific command for settting Radio transmit power level
 *          for EM9304.
 *
 *  \param  txPowerlevel    valid range from 0 to 17 in decimal.
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
uint32_t HciVsEM_SetRfPowerLevelEx(txPowerLevel_t txPowerlevel)
{
  // make sure it's 8 bit
  uint8_t tx_power_level = (uint8_t)txPowerlevel;

  if(tx_power_level < TX_POWER_LEVEL_INVALID) {
    BT_hci_vendor_specific_command(0xFC26, &tx_power_level, sizeof(tx_power_level));
    return true;
  }
  else {
    return false;
  }

}


/*************************************************************************************************/
/*!
 *  \fn     HciVsEM_TransmitterTest
 *
 *  \brief  Vendor-specific command for start transmitter testing
 *
 *  \param  test_mode       refer to em9304 datasheet
 *  \param  channel_number  refer to em9304 datasheet
 *  \param  packet_len      refer to em9304 datasheet
 *  \param  packet_payload_type    refer to em9304 datasheet
 *
 *  \return None
 */
/*************************************************************************************************/
void HciVsEM_TransmitterTest(uint8_t test_mode, uint8_t channel_number, uint8_t packet_len, uint8_t packet_payload_type)
{
    uint8_t params[4] = {
      test_mode,
      channel_number,
      packet_len,
      packet_payload_type
    };

    BT_hci_vendor_specific_command(0xFC11, &params[0], sizeof(params));

}


/*************************************************************************************************/
/*!
 *  \fn     HciVsEM_TransmitterTestEnd
 *
 *  \brief  Vendor-specific command for ending Radio transmitter testing.
 *
 *  \param  None
 *
 *  \return None
 */
/*************************************************************************************************/
void HciVsEM_TransmitterTestEnd(void)
{
    BT_hci_vendor_specific_command(0xFC12, NULL, 0);
}

void HciDrvAssignBDAddress(uint8_t * customer_unique_bd_address)
{
  if (customer_unique_bd_address)
  {
      memcpy(g_pui8BLEMacAddress, customer_unique_bd_address, 6);
  }
}

void HciVsEM_SetBDAddress(void)
{
  if (HCI_APOLLO_USE_CUSTOMER_OWN_MAC)
  {
    BT_hci_vendor_specific_command(0xFC02, g_pui8BLEMacAddress, 6);
  }
}

#endif /* BT_SPI */


