//*****************************************************************************
//
//! @file hci_drv_apollo3.c
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
#include <string.h>
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "hci_drv_apollo3.h"
#include "hci_drv_internal.h"
#include "FreeRTOSConfig.h"

//*****************************************************************************
//
// Enable the heartbeat command?
//
// Setting this to 1 will cause the MCU to send occasional HCI packets to the
// BLE core if there hasn't been any activity for a while. This can help catch
// communication issues that might otherwise go unnoticed.
//
//*****************************************************************************
#define ENABLE_BLE_HEARTBEAT            1

//*****************************************************************************
//
// Configurable buffer sizes.
//
//*****************************************************************************
#define HCI_DRV_MAX_TX_PACKET           256
#define HCI_DRV_MAX_RX_PACKET           256

//*****************************************************************************
//
// Configurable error-detection thresholds.
//
//*****************************************************************************
#define HEARTBEAT_TIMEOUT_MS            (10000)   //milli-seconds
#define HCI_DRV_MAX_IRQ_TIMEOUT          2000
#define HCI_DRV_MAX_XTAL_RETRIES         10
#define HCI_DRV_MAX_HCI_TRANSACTIONS     10000
#define HCI_DRV_MAX_READ_PACKET          4   // max read in a row at a time


DECL_STATIC uint8_t g_pui8WriteBuffer[HCI_DRV_MAX_TX_PACKET];
DECL_STATIC UINT16  g_pui8WriteBuffer_len;

//*****************************************************************************
//
// Heartbeat implementation functions.
//
//*****************************************************************************
#if ENABLE_BLE_HEARTBEAT

#define BLE_HEARTBEAT_START()                                                 \
    do { BT_start_timer( &g_HeartBeatTimer, (HEARTBEAT_TIMEOUT_MS+1000)/1000, heartbeat_timer_handler, NULL, 0);} while (0)

#define BLE_HEARTBEAT_STOP()                                                  \
    do { BT_stop_timer(g_HeartBeatTimer); } while (0)

#define BLE_HEARTBEAT_RESTART()                                               \
    do                                                                        \
    {                                                                         \
        BT_stop_timer( g_HeartBeatTimer);                                      \
        BT_start_timer( &g_HeartBeatTimer, (HEARTBEAT_TIMEOUT_MS+1000)/1000, heartbeat_timer_handler, NULL, 0);   \
    } while (0)

#else

#define BLE_HEARTBEAT_START()
#define BLE_HEARTBEAT_STOP()
#define BLE_HEARTBEAT_RESTART()

#endif

//*****************************************************************************
//
// Global variables.
//
//*****************************************************************************

// BLE module handle
void *BLE;

//fixme: set the BLE MAC address to a special value
uint8_t g_BLEMacAddress[6] = {0x00};

// Global handle used to send BLE events about the Hci driver layer.
BT_timer_handle g_HeartBeatTimer;

// Buffers for HCI read data.
uint32_t g_pui32ReadBuffer[HCI_DRV_MAX_RX_PACKET / 4];
uint8_t *g_pui8ReadBuffer = (uint8_t *) g_pui32ReadBuffer;

// Counters for tracking read data.
volatile uint32_t g_ui32InterruptsSeen = 0;

void HciDrvEmptyWriteQueue(void);


//*****************************************************************************
//
// Events for the HCI driver interface.
//
//*****************************************************************************
#define BLE_TRANSFER_NEEDED_EVENT                   0x01
#define BLE_HEARTBEAT_EVENT                         0x02
#define BLE_SET_WAKEUP                              0x03

//*****************************************************************************
//
// Error-handling wrapper macro.
//
//*****************************************************************************
#define ERROR_CHECK_VOID(status)                                              \
    {                                                                         \
        uint32_t ui32ErrChkStatus;                                            \
        if (0 != (ui32ErrChkStatus = (status)))                               \
        {                                                                     \
            am_util_debug_printf("ERROR_CHECK_VOID "#status "\n");            \
            error_check(ui32ErrChkStatus);                                    \
            return;                                                           \
        }                                                                     \
    }

#define ERROR_RETURN(status, retval)                                          \
    if ((status))                                                             \
    {                                                                         \
        error_check(status);                                                  \
        return (retval);                                                      \
    }

#define ERROR_RECOVER(status)                                                 \
    if ((status))                                                             \
    {                                                                         \
        am_hal_debug_gpio_toggle(BLE_DEBUG_TRACE_10);                         \
        error_check(status);                                                  \
        HciDrvRadioShutdown();                                                \
        HciDrvRadioBoot(0);                                                   \
        HciDrvEmptyWriteQueue();                                              \
        DmDevReset();                                                         \
        return;                                                               \
    }

//*****************************************************************************
//
// Debug section.
//
//*****************************************************************************
#if 0
#define CRITICAL_PRINT(...)                                                   \
    do                                                                        \
    {                                                                         \
        AM_CRITICAL_BEGIN;                                                    \
        am_util_debug_printf(__VA_ARGS__);                                    \
        AM_CRITICAL_END;                                                      \
    } while (0)
#else
#define CRITICAL_PRINT(...)
#endif

#define ENABLE_IRQ_PIN 0

#define TASK_LEVEL_DELAYS 0

// #### INTERNAL BEGIN ####
#define ENABLE_HISTOGRAMS           0
#if ENABLE_HISTOGRAMS

#define HISTOGRAM_RESOLUTION        10
#define HISTOGRAM_SIZE              1024

uint32_t g_pui32StatusDelays[HISTOGRAM_SIZE];
uint32_t g_pui32IRQDelays[HISTOGRAM_SIZE];
#endif
// #### INTERNAL END ####

#ifndef NO_ERRORS

volatile uint32_t g_ui32FailingStatus = 0;

#define ERROR_CHECK(expr)                                                     \
    g_ui32FailingStatus = (expr);                                             \
    if (g_ui32FailingStatus != AM_HAL_STATUS_SUCCESS)                         \
    {                                                                         \
        while (1);                                                            \
    }

#else

#define ERROR_CHECK(expr)

#endif


//*****************************************************************************
//
// By default, errors will be printed. If there is an error handler defined,
// they will be sent there intead.
//
//*****************************************************************************
static void
error_check(uint32_t ui32Status)
{
    //
    // Don't do anything unless there's an error.
    //
    if (ui32Status)
    {
        //
        // Set the global error status. If there's an error handler function,
        // call it. Otherwise, just print the error status and wait.
        //
        g_ui32FailingStatus = ui32Status;

        CRITICAL_PRINT("Error detected: 0x%08x\n", g_ui32FailingStatus);
        CRITICAL_PRINT("BSTATUS: 0x%08x\n", BLEIF->BSTATUS);
    }
}


#define BLE_IRQ_CHECK()             (BLEIF->BSTATUS_b.BLEIRQ)



//*****************************************************************************
//
// Forward declarations.
//
//*****************************************************************************

void hci_spi_bt_shutdown (void);

#ifdef BT_SPI

#include "am_util.h"
#include "hci_apollo_config.h"

#ifndef HCI_APOLLO_MAC
#define HCI_APOLLO_MAC                  {0x01, 0x00, 0x00, 0xEE, 0xF3, 0x0C}
#endif

#ifndef HCI_APOLLO_USE_CHIPID_FOR_MAC
#define HCI_APOLLO_USE_CHIPID_FOR_MAC   true
#endif

DECL_STATIC BT_THREAD_RETURN_TYPE hci_spi_read_task (BT_THREAD_ARGS args);


/* ----------------------------------------- External Global Variables */
void hci_transport_enqueue (UCHAR * data, UINT16 datalen);

/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */

/* SPI Read Task State */
DECL_STATIC UCHAR hci_spi_state;
/* SPI Read Task Synchronization */
BT_DEFINE_MUTEX (hci_spi_mutex)
BT_DEFINE_COND (hci_spi_cond)

/* ----------------------------------------- Functions */

/** HCI-SPI Initialization */
void hci_spi_init (void)
{
    BT_thread_type tid;

    BT_MUTEX_INIT_VOID (hci_spi_mutex, TRANSPORT);
    BT_COND_INIT_VOID(hci_spi_cond, TRANSPORT);

    /* Initialize SPI State */
    hci_spi_state = 0x0;

    /* Create a thread to receive data From Serial PORT and BUFFER it */
    if (0 != BT_thread_create(&tid, NULL, hci_spi_read_task, NULL))
    {
        HCI_SPI_ERR(
        "[HCI-SPI] Could NOT Create SPI Read Thread\n");

        return;
    }


    HCI_SPI_TRC(
    "[HCI-SPI] SPI Power On Initialization Complete\n");

    return;
}


void heartbeat_timer_handler (void *data, UINT16 datalen)
{
    BT_hci_read_local_version_information();
    BLE_HEARTBEAT_START();
}

//*****************************************************************************
//
// Other useful macros.
//
//*****************************************************************************

#define BLE_IRQ_CHECK()             (BLEIF->BSTATUS_b.BLEIRQ)

// Ellisys HCI SPI tapping support

// #define ELLISYS_HCI_LOG_SUPPORT 1

//*****************************************************************************
//
// Boot the radio.
//
//*****************************************************************************

/** HCI-SPI Bluetooth-ON Initialization */
void hci_spi_bt_init(bool bColdBoot)
{
    uint32_t ui32NumXtalRetries = 0;
	
	// #### INTERNAL BEGIN ####
#if ENABLE_HISTOGRAMS
    am_util_debug_printf("g_pui32StatusDelays: 0x%08X\n", g_pui32StatusDelays);
    am_util_debug_printf("g_pui32IRQDelays: 0x%08X\n", g_pui32IRQDelays);
    am_util_debug_printf("HISTOGRAM_SIZE (bytes): %d\n", HISTOGRAM_SIZE * 4);
    am_util_delay_ms(500);
#endif
// #### INTERNAL END ####

    hci_spi_lock (hci_spi_mutex);
    hci_spi_state = 0x1;
    hci_spi_signal (hci_spi_cond);
    hci_spi_unlock (hci_spi_mutex);
	#if !defined(AM_DEBUG_BLE_TIMING) && defined(ELLISYS_HCI_LOG_SUPPORT)
    am_hal_gpio_pincfg_t pincfg = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    pincfg.uFuncSel = 6;
    am_hal_gpio_pinconfig(30, pincfg);
    am_hal_gpio_pinconfig(31, pincfg);
    am_hal_gpio_pinconfig(32, pincfg);
    pincfg.uFuncSel = 4;
    am_hal_gpio_pinconfig(33, pincfg);
    pincfg.uFuncSel = 7;
    am_hal_gpio_pinconfig(35, pincfg);
#endif

#ifdef AM_DEBUG_BLE_TIMING
    //
    // Enable debug pins.
    //
    // 30.6 - SCLK
    // 31.6 - MISO
    // 32.6 - MOSI
    // 33.4 - CSN
    // 35.7 - SPI_STATUS
    //
    am_hal_gpio_pincfg_t pincfg = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    pincfg.uFuncSel = 6;
    am_hal_gpio_pinconfig(30, pincfg);
    am_hal_gpio_pinconfig(31, pincfg);
    am_hal_gpio_pinconfig(32, pincfg);
    pincfg.uFuncSel = 4;
    am_hal_gpio_pinconfig(33, pincfg);
    pincfg.uFuncSel = 7;
    am_hal_gpio_pinconfig(35, pincfg);
    pincfg.uFuncSel = 1;
#if ENABLE_IRQ_PIN
    am_hal_gpio_pinconfig(41, pincfg);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_08);
#endif

    am_hal_gpio_pinconfig(11, g_AM_HAL_GPIO_OUTPUT);

#endif // AM_DEBUG_BLE_TIMING

    //
    // This pin is also used to generate BLE interrupts in the current
    // implementation.
    //
    // 41.1 - BLE IRQ
    //
    //am_hal_gpio_pin_config(41, AM_HAL_GPIO_FUNC(1));

    // #### INTERNAL BEGIN ####
    // Use for debugging!
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_01);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_02);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_03);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_04);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_05);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_06);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_07);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_08);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_09);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_10);
    am_hal_debug_gpio_pinconfig(BLE_DEBUG_TRACE_11);

    // #### INTERNAL END ####

    NVIC_SetPriority(BLE_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);

    //
    // Configure and enable the BLE interface.
    //
    uint32_t ui32Status = AM_HAL_STATUS_FAIL;
    while (ui32Status != AM_HAL_STATUS_SUCCESS)
    {
        ERROR_CHECK_VOID(am_hal_ble_initialize(0, &BLE));
        ERROR_CHECK_VOID(am_hal_ble_power_control(BLE, AM_HAL_BLE_POWER_ACTIVE));

        am_hal_ble_config_t sBleConfig =
        {
            // Configure the HCI interface clock for 6 MHz
            .ui32SpiClkCfg = AM_HAL_BLE_HCI_CLK_DIV8,

            // Set HCI read and write thresholds to 32 bytes each.
            .ui32ReadThreshold = 32,
            .ui32WriteThreshold = 32,

            // The MCU will supply the clock to the BLE core.
            .ui32BleClockConfig = AM_HAL_BLE_CORE_MCU_CLK,

            // Default settings for expected BLE clock drift (measured in PPM).
            .ui32ClockDrift = 0,
            .ui32SleepClockDrift = 50,

            // Default setting - AGC Enabled
            .bAgcEnabled = true,

            // Default setting - Sleep Algo enabled
            .bSleepEnabled = true,

            // Apply the default patches when am_hal_ble_boot() is called.
            .bUseDefaultPatches = true,
        };

        ERROR_CHECK_VOID(am_hal_ble_config(BLE, &sBleConfig));
        //
        // Delay 1s for 32768Hz clock stability. This isn't required unless this is
        // our first run immediately after a power-up.
        //
        if ( bColdBoot )
        {
            am_util_delay_ms(1000);
        }
        //
        // Attempt to boot the radio.
        //
        ui32Status = am_hal_ble_boot(BLE);

        //
        // Check our status.
        //
        if (ui32Status == AM_HAL_STATUS_SUCCESS)
        {
            //
            // If the radio is running, we can exit this loop.
            //
            break;
        }
        else if (ui32Status == AM_HAL_BLE_32K_CLOCK_UNSTABLE)
        {
            //
            // If the radio is running, but the clock looks bad, we can try to
            // restart.
            //
            ERROR_CHECK_VOID(am_hal_ble_power_control(BLE, AM_HAL_BLE_POWER_OFF));
            ERROR_CHECK_VOID(am_hal_ble_deinitialize(BLE));

            //
            // We won't restart forever. After we hit the maximum number of
            // retries, we'll just return with failure.
            //
            if (ui32NumXtalRetries++ < HCI_DRV_MAX_XTAL_RETRIES)
            {
                am_util_delay_ms(1000);
            }
            else
            {
                return;
            }
        }
        else
        {
            ERROR_CHECK_VOID(am_hal_ble_power_control(BLE, AM_HAL_BLE_POWER_OFF));
            ERROR_CHECK_VOID(am_hal_ble_deinitialize(BLE));
            //
            // If the radio failed for some reason other than 32K Clock
            // instability, we should just report the failure and return.
            //
            error_check(ui32Status);
            return;
        }
    }

    //
    // Set the BLE TX Output power to 0dBm.
    //
    am_hal_ble_tx_power_set(BLE, 0x8);

    //
    // Enable interrupts for the BLE module.
    //

    am_hal_ble_int_clear(BLE, (AM_HAL_BLE_INT_CMDCMP |
                               AM_HAL_BLE_INT_DCMP |
                               AM_HAL_BLE_INT_BLECIRQ));

    am_hal_ble_int_enable(BLE, (AM_HAL_BLE_INT_CMDCMP |
                                AM_HAL_BLE_INT_DCMP |
                                AM_HAL_BLE_INT_BLECIRQ));

    CRITICAL_PRINT("INTEN:  %d\n", BLEIF->INTEN_b.BLECSSTAT);
    CRITICAL_PRINT("INTENREG:  %d\n", BLEIF->INTEN);

    NVIC_EnableIRQ(BLE_IRQn);

    //
    // Reset the RX interrupt counter.
    //
    g_ui32InterruptsSeen = 0;


    // When it's bColdBoot, it will use Apollo's Device ID to form Bluetooth address.
    if (bColdBoot)
    {
        am_hal_mcuctrl_device_t sDevice;
        am_hal_mcuctrl_info_get(AM_HAL_MCUCTRL_INFO_DEVICEID, &sDevice);

        // Bluetooth address formed by ChipID1 (32 bits) and ChipID0 (8-23 bits).
        memcpy(g_BLEMacAddress, &sDevice.ui32ChipID1, sizeof(sDevice.ui32ChipID1));
        // ui32ChipID0 bit 8-31 is test time during chip manufacturing
        g_BLEMacAddress[4] = (sDevice.ui32ChipID0 >> 8) & 0xFF;
        g_BLEMacAddress[5] = (sDevice.ui32ChipID0 >> 16) & 0xFF;
    }

    return;
}


/** HCI-SPI Bluetooth-OFF Shutdown */
void hci_spi_bt_shutdown (void)
{
    BLE_HEARTBEAT_STOP();

    NVIC_DisableIRQ(BLE_IRQn);

    ERROR_CHECK_VOID(am_hal_ble_power_control(BLE, AM_HAL_BLE_POWER_OFF));

    while ( PWRCTRL->DEVPWREN_b.PWRBLEL );

    ERROR_CHECK_VOID(am_hal_ble_deinitialize(BLE));
   /* Signal SPI Read Task */
    hci_spi_lock (hci_spi_mutex);
    hci_spi_state = 0x0;
    hci_spi_unlock (hci_spi_mutex);
}


//*****************************************************************************
//
//
// This handler can perform HCI reads or writes, and keeps the actions in the
// correct order.
//
//*****************************************************************************

void
HciDrvReadWriteHandler(uint8_t hci_type, uint8_t * hci_sendbuf, uint32_t hci_sendbuf_len)
{
    uint32_t ui32ErrorStatus;

    //
    // Loop indefinitely, checking to see if there are still tranfsers we need
    // to complete.
    //
    while (1)
    {
        //
        // Figure out what kind of transfer the BLE core will accept.
        //
        if ( BLE_IRQ_CHECK() )
        {
            //
            // Is the BLE core asking for a read? If so, do that now.
            //
            uint32_t ui32NumBytes = 0;
            ui32ErrorStatus = am_hal_ble_blocking_hci_read(BLE, (uint32_t*)g_pui32ReadBuffer, &ui32NumBytes);

            if (ui32NumBytes > HCI_DRV_MAX_RX_PACKET)
            {
                am_util_debug_printf("ERROR: Trying to receive an HCI packet larger than the hci driver buffer size (needs %d bytes of space).",
                                     ui32NumBytes);
                while (1);
            }


            if ( ui32ErrorStatus == AM_HAL_STATUS_SUCCESS)
            {
                //
                // If the read succeeded, we need to wait for the IRQ signal to
                // go back down. If we don't we might inadvertently try to read
                // the same packet twice.
                //
                while ( BLE_IRQ_CHECK() );

                //
                // Pass the data along to the stack. The stack should be able
                // to read as much data as we send it.  If it can't, we need to
                // know that.
                //

                hci_transport_enqueue ((uint8_t *)g_pui32ReadBuffer, (UINT16)ui32NumBytes);

            }
            else
            {
                //
                // If the read didn't succeed for some physical reason, we need
                // to know. We shouldn't get failures here. We checked the IRQ
                // signal before calling the read function, and this driver
                // only contains a single call to the blocking read function,
                // so there shouldn't be any physical reason for the read to
                // fail.
                //
                am_util_debug_printf("HCI READ failed with status %d. Try recording with a logic analyzer to catch the error.\n",
                                     ui32ErrorStatus);
                while (1);
            }

        }
        else
        {
            //
            // If we don't have anything to read, we can start checking to see
            // if we have things to write.
            //
            if (hci_sendbuf == NULL)
            {
                //
                // If not, we're done!
                //
                break;
            }
            else
            {
                //
                // If we do have something to write, just pop a single item
                // from the queue and send it.
                //
                ui32ErrorStatus = am_hal_ble_blocking_hci_write(BLE,
                                                                hci_type,
                                                                (uint32_t *)hci_sendbuf,
                                                                hci_sendbuf_len);


                if (ui32ErrorStatus == AM_HAL_STATUS_SUCCESS)
                {
                    // mark it so that we can exit in next loop.
                    hci_sendbuf = NULL;
                }
            }
        }

    }
}

/** HCI-SPI Read Task */
DECL_STATIC BT_THREAD_RETURN_TYPE hci_spi_read_task (BT_THREAD_ARGS args)
{
    BT_LOOP_FOREVER()
    {
        hci_spi_lock (hci_spi_mutex);

        if ( 0x1 == hci_spi_state)
        {
            HCI_SPI_TRC(
            "[HCI-SPI] SPI Read Task: Waiting to Service ..\n");

            hci_spi_wait (hci_spi_cond, hci_spi_mutex);

            HCI_SPI_TRC(
            "[HCI-SPI] SPI Read Task: Ready to Service ..\n");

            HciDrvReadWriteHandler(0, NULL, 0);
        }

        hci_spi_unlock (hci_spi_mutex);
    }

    return API_SUCCESS;
}

/** HCI-SPI Send Data */
API_RESULT hci_spi_send_data
           (UCHAR type, UCHAR * buf, UINT16 length, UCHAR flag)
{
    // HCI command is sent as a complete packet from Mindtree stack
    if(type == HCI_COMMAND_PACKET) {
        uint16_t opcode;

        hci_spi_lock (hci_spi_mutex);

        HciDrvReadWriteHandler(type, buf, length);
        hci_unpack_2_byte_param(&opcode, buf);
        
        if (HCI_RESET_OPCODE == opcode) {
            HciVscSetBDAddr(g_BLEMacAddress);
        }
        hci_spi_unlock (hci_spi_mutex);
    }
    else {
        // HCI ACL packet is sent in at least three fragments
        if(flag == 1) {
            // beginning of HCI packet header
            BT_mem_copy(g_pui8WriteBuffer, buf, length);
            g_pui8WriteBuffer_len = length;
        }
        else {
            // L2CAP header and payload are sent seperately
            BT_mem_copy(g_pui8WriteBuffer + g_pui8WriteBuffer_len, buf, length);
            g_pui8WriteBuffer_len += length;
        }

        // Check if a complete HCI ACL packet is received.
        if ((g_pui8WriteBuffer_len - 4) < (g_pui8WriteBuffer[2] + (g_pui8WriteBuffer[3] << 8))) {
            return API_SUCCESS;
        }

        hci_spi_lock (hci_spi_mutex);

        HciDrvReadWriteHandler(type, g_pui8WriteBuffer, g_pui8WriteBuffer_len);

        hci_spi_unlock (hci_spi_mutex);

    }

    return API_SUCCESS;
}

//*****************************************************************************
//
// Interrupt handler for BLE
//
//*****************************************************************************
void
am_ble_isr(void)
{

    BaseType_t xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken = pdFALSE;

    uint32_t ui32Status = am_hal_ble_int_status(BLE, true);

    am_hal_ble_int_clear(BLE, ui32Status);

    // Send an event to the HCI Read Task.
    //
    xSemaphoreGiveFromISR(hci_spi_cond,&xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken != pdFAIL)
    {
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }

}

/*************************************************************************************************/
/*!
 *  \fn     HciVscSetRfPowerLevelEx
 *
 *  \brief  Vendor-specific command for settting Radio transmit power level
 *          for Nationz.
 *
 *  \param  txPowerlevel    valid range from 0 to 15 in decimal.
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
bool HciVscSetRfPowerLevelEx(txPowerLevel_t txPowerlevel)
{
    switch(txPowerlevel){

        case TX_POWER_LEVEL_MINUS_10P0_dBm:
            am_hal_ble_tx_power_set(BLE,0x04);
            return true;
        break;
        case TX_POWER_LEVEL_0P0_dBm:
            am_hal_ble_tx_power_set(BLE,0x08);
            return true;
        break;
        case TX_POWER_LEVEL_PLUS_3P0_dBm:
            am_hal_ble_tx_power_set(BLE,0x0F);
            return true;
        break;
        default:
            return false;
        break;
    }
}

/*************************************************************************************************/
/*!
 *  \fn     HciVscSetBDAddr
 *
 *  \brief  Vendor-specific command for setting Bluetooth device address.
 *
 *  \param  pointer to supplied BD address.
 *
 *  \return true when success, otherwise false
 */
/*************************************************************************************************/
void HciVscSetBDAddr(uint8_t *bd_addr)
{
    BT_hci_vendor_specific_command(0xFC32, &bd_addr[0], 6);
}

#endif /* BT_SPI */
