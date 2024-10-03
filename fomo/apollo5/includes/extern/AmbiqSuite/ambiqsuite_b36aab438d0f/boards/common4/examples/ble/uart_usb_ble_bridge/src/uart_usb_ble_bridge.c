//*****************************************************************************
//
//! @file uart_usb_ble_bridge.c
//!
//! @brief Converts UART and USB HCI commands to SPI.
//!
//! @addtogroup ble_examples BLE Examples
//!
//! @defgroup uart_usb_ble_bridge BLE UART USB HCI Example
//! @ingroup ble_examples
//! @{
//!
//! This exapmle can be used as a way to communicate by using UART/USB HCI between
//! the Apollo4 (host) and Cooper (controller).
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
#include "am_bsp.h"
#include "am_mcu_apollo.h"
#include "am_util.h"
#include "am_util_ble_cooper.h"
#include "am_devices_cooper.h"
#include "tusb.h"

//*****************************************************************************
//
// Macro Definition.
//
//*****************************************************************************
// Configure ECHO_BACK_USB_DATA_ONLY to 1 to echo back the USB received data to
// the sender (DTM tester or PC) only. Set it to 0 can post the data to the BLE
// controller via HCI, then send the received response data from controller to
// the DTM tester or PC via USB.
#define ECHO_BACK_USB_DATA_ONLY         (1)

#define UART_HCI_BRIDGE                 (AM_BSP_UART_PRINT_INST)
#define MAX_UART_PACKET_SIZE            (2048)
#define UART_RX_TIMEOUT_MS              (1)
#define MAX_READ_BYTES                  (23)

#define HCI_RX_DATA                     (0)
#define SERIAL_RX_DATA                  (1)

#define RUNNING_GPIO_PIN                (31)
#define DONE_GPIO_PIN                   (32)

#define HCI_CMD_TYPE                    (1)
#define HCI_ACL_TYPE                    (2)
#define HCI_CMD_HEADER_LEN              (3)
#define HCI_ACL_HEADER_LEN              (4)
//compatible for WVT case requiring send more than 256 data(eg. case hci_cfc_tc_01 may send 265bytes data)
#define HCI_MAX_RX_PACKET               (512)

#define RUNNING_MODE_NONE               (0)
#define RUNNING_MODE_UART               (1)
#define RUNNING_MODE_USB                (2)

#define IOM_INTERRUPT1(n)       AM_HAL_INTERRUPT_IOMASTER ## n
#define IOM_INTERRUPT(n)        IOM_INTERRUPT1(n)
#define COOPER_IOM_IRQn         ((IRQn_Type)(IOMSTR0_IRQn + SPI_MODULE))

//
// Take over the interrupt handler for whichever IOM we're using.
//
#define cooper_iom_isr                                                        \
    am_iom_isr1(SPI_MODULE)
#define am_iom_isr1(n)                                                        \
    am_iom_isr(n)
#define am_iom_isr(n)                                                         \
    am_iomaster ## n ## _isr

#define BYTES_TO_UINT16(n, p)     {n = ((uint16_t)(p)[0] + ((uint16_t)(p)[1] << 8)); }
//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
void *g_pvUART;
uint8_t g_pui8UARTTXBuffer[MAX_UART_PACKET_SIZE];

uint32_t DMATCBBuf[2048];
am_devices_cooper_buffer(1024) g_psUartWriteData;
am_devices_cooper_buffer(1024) g_psUartReadData;
am_devices_cooper_buffer(1024) g_psUsbWriteData;
am_devices_cooper_buffer(1024) g_psUsbReadData;
uint8_t g_RunningMode = RUNNING_MODE_NONE;

uint8_t nvds_cmd[] =
{
    NVDS_PARAMETER_MEM_WRITE_ENABLE,
    NVDS_PARAMETER_SLEEP_DISABLE,
    NVDS_PARAMETER_EXT_32K_CLK_SOURCE,
    NVDS_PARAMETER_BD_ADDRESS
};

uint8_t hci_nop_opcode_event[1 + 1 + 1 + 1 + 2 + 1] = {0x04, 0x0e, 0x04, 0x05, 0x00, 0x00, 0x00};

am_devices_cooper_config_t stCooperConfig;

// Rx Index of UART/USB
volatile uint32_t g_ui32UartRxIndex = 0;
volatile bool g_bUartRxTimeoutFlag = false;
volatile bool g_bUartCmdProcessedFlag = false;
volatile uint32_t g_ui32UsbRxIndex = 0;
volatile bool g_bUsbRxTimeoutFlag = false;
volatile bool g_bUsbCmdProcessedFlag = false;

void *g_IomDevHdl;
void *g_pvHciSpiHandle;

typedef enum
{
    HCI_RX_STATE_IDLE,
    HCI_RX_STATE_HEADER,
    HCI_RX_STATE_DATA,
    HCI_RX_STATE_COMPLETE
} hciRxState_t;

//*****************************************************************************
//
// Internal function declaration.
//
//*****************************************************************************
static void hci_rx_process(uint8_t mode, uint32_t* pui32Data, uint32_t ui32Length);
static void serial_rx_process(uint32_t* pui32Data, uint32_t ui32Length);
static void running_done_gpio_init(void);
static void running_done_gpio_set(bool is_running, bool is_done);
static uint32_t hci_data_write(uint32_t* pui32Data, uint32_t ui32Length);
static uint32_t hci_data_read(uint32_t* pui32Data, uint32_t* ui32Length);

//*****************************************************************************
//
// External variables declaration.
//
//*****************************************************************************

//*****************************************************************************
//
// External function declaration.
//
//*****************************************************************************

//*****************************************************************************
//
// Initialize the UART.
//
//*****************************************************************************
void uart_init(void)
{
    am_util_stdio_printf("Apollo4 UART/USB to SPI Bridge\n");

    //
    // Start the UART.
    //
    am_hal_uart_config_t sUartConfig =
    {
        //
        // Standard UART settings: 115200-8-N-1
        //
        .ui32BaudRate    = 115200,
        //
        // Set TX and RX FIFOs to interrupt at three-quarters full.
        //
        .eDataBits    = AM_HAL_UART_DATA_BITS_8,
        .eParity      = AM_HAL_UART_PARITY_NONE,
        .eStopBits    = AM_HAL_UART_ONE_STOP_BIT,
        .eFlowControl = AM_HAL_UART_FLOW_CTRL_NONE,
        .eTXFifoLevel = AM_HAL_UART_FIFO_LEVEL_28,
        .eRXFifoLevel = AM_HAL_UART_FIFO_LEVEL_28,
    };

    am_hal_uart_initialize(UART_HCI_BRIDGE, &g_pvUART);
    am_hal_uart_power_control(g_pvUART, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_uart_configure(g_pvUART, &sUartConfig);
    am_hal_uart_buffer_configure(g_pvUART, g_pui8UARTTXBuffer, sizeof(g_pui8UARTTXBuffer), NULL, 0);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX);
    //
    // Make sure to enable the interrupts for RX, since the HAL doesn't already
    // know we intend to use them.
    //
    NVIC_SetPriority((IRQn_Type)(UART0_IRQn + UART_HCI_BRIDGE), AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + UART_HCI_BRIDGE));
    am_hal_uart_interrupt_enable(g_pvUART,  (AM_HAL_UART_INT_RX |
                                           AM_HAL_UART_INT_RX_TMOUT |
                                           AM_HAL_UART_INT_OVER_RUN |
                                           AM_HAL_UART_INT_TX |
                                           AM_HAL_UART_INT_TXCMP));

}

//*****************************************************************************
//
// Deinitialize the UART.
//
//*****************************************************************************
void uart_deinit(void)
{
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, am_hal_gpio_pincfg_disabled);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, am_hal_gpio_pincfg_disabled);

    am_hal_uart_deinitialize(g_pvUART);
}

//*****************************************************************************
//
// Read UART data.
//
//*****************************************************************************
void uart_data_read(uint8_t* pui8Data, uint32_t* ui32Length)
{
    am_hal_uart_transfer_t sRead =
    {
        .eType = AM_HAL_UART_BLOCKING_READ,
        .ui32TimeoutMs = UART_RX_TIMEOUT_MS,
        .pui8Data = pui8Data,
        .ui32NumBytes = MAX_READ_BYTES,
        .pui32BytesTransferred = ui32Length,
    };

    am_hal_uart_transfer(g_pvUART, &sRead);
}

//*****************************************************************************
//
// Write UART data.
//
//*****************************************************************************
void uart_data_write(uint8_t* pui8Data, uint32_t ui32Length)
{
    uint32_t ui32BytesTransferred = 0;
    am_hal_uart_transfer_t sWrite =
    {
        .eType = AM_HAL_UART_BLOCKING_WRITE,
        .pui8Data = pui8Data,
        .ui32NumBytes = ui32Length,
        .ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER,
        .pui32BytesTransferred = &ui32BytesTransferred,
    };

    am_hal_uart_transfer(g_pvUART, &sWrite);
}

//*****************************************************************************
//
// Enable UART IRQ.
//
//*****************************************************************************
void uart_irq_enable(void)
{
    NVIC_SetPriority((IRQn_Type)(UART0_IRQn + UART_HCI_BRIDGE), AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + UART_HCI_BRIDGE));
}

//*****************************************************************************
//
// Disable UART IRQ.
//
//*****************************************************************************
void uart_irq_disable(void)
{
    NVIC_DisableIRQ((IRQn_Type)(UART0_IRQn + UART_HCI_BRIDGE));
}

//*****************************************************************************
//
// IOM ISRs.
//
//*****************************************************************************
//
//! Take over default ISR. (Queue mode service)
//
void cooper_iom_isr(void)
{
    uint32_t ui32Status;

    if (!am_hal_iom_interrupt_status_get(g_pvHciSpiHandle, true, &ui32Status))
    {
        if ( ui32Status )
        {
            am_hal_iom_interrupt_clear(g_pvHciSpiHandle, ui32Status);
            am_hal_iom_interrupt_service(g_pvHciSpiHandle, ui32Status);
        }
    }
}

//*****************************************************************************
//
// GPIO interrupt handler.
//
//*****************************************************************************
void am_cooper_irq_isr(void)
{
    uint32_t    ui32IntStatus;

    am_hal_gpio_interrupt_irq_status_get(AM_COOPER_IRQn, false, &ui32IntStatus);
    am_hal_gpio_interrupt_irq_clear(AM_COOPER_IRQn, ui32IntStatus);
    am_hal_gpio_interrupt_service(AM_COOPER_IRQn, ui32IntStatus);
}

/*************************************************************************************************/
/*!
 *  \fn     uart_rx_hci_state_machine
 *
 *  \brief  Receive function. Gets called by external code when UART bytes are received.
 *
 *  \param  pBuf   Pointer to buffer of incoming bytes.
 *  \param  len    Number of bytes in incoming buffer.
 *
 *  \return The number of bytes consumed.
 */
/*************************************************************************************************/
uint16_t uart_rx_hci_state_machine(uint8_t *pBuf, uint16_t len)
{
    static uint8_t    stateRx = HCI_RX_STATE_IDLE;
    static uint8_t    pktIndRx;
    static uint16_t   iRx;
    static uint8_t    hdrRx[HCI_ACL_HEADER_LEN];
    static uint8_t    pPktRx[HCI_MAX_RX_PACKET];
    static uint8_t    *pDataRx;
    uint8_t   dataByte;
    uint16_t  consumed_bytes;

    consumed_bytes = 0;
    /* loop until all bytes of incoming buffer are handled */
    while (len)
    {
        /* read single byte from incoming buffer and advance to next byte */
        dataByte = *pBuf;

        /* --- Idle State --- */
        if (stateRx == HCI_RX_STATE_IDLE)
        {
            /* save the packet type */
            pktIndRx = dataByte;
            iRx      = 0;
            stateRx  = HCI_RX_STATE_HEADER;
            pBuf++;
            consumed_bytes++;
            len--;
        }
        /* --- Header State --- */
        else if (stateRx == HCI_RX_STATE_HEADER)
        {
            uint8_t  hdrLen = 0;
            uint16_t dataLen = 0;

            /* determine header length based on packet type */
            if (pktIndRx == HCI_CMD_TYPE)
            {
                hdrLen = HCI_CMD_HEADER_LEN;
            }
            else if (pktIndRx == HCI_ACL_TYPE)
            {
                hdrLen = HCI_ACL_HEADER_LEN;
            }
            else
            {
                /* invalid packet type */
                stateRx = HCI_RX_STATE_IDLE;
                return consumed_bytes;
            }

            if (iRx != hdrLen)
            {
                /* copy current byte into the temp header buffer */
                hdrRx[iRx++] = dataByte;
                pBuf++;
                consumed_bytes++;
                len--;
            }

            /* see if entire header has been read */
            if (iRx == hdrLen)
            {
                uint8_t  i = 0;
                /* extract data length from header */
                if (pktIndRx == HCI_CMD_TYPE)
                {
                    dataLen = hdrRx[2];
                }
                else if (pktIndRx == HCI_ACL_TYPE)
                {
                    BYTES_TO_UINT16(dataLen, &hdrRx[2]);
                }

                pDataRx = pPktRx;

                /* copy header into data packet (note: memcpy is not so portable) */
                for (i = 0; i < hdrLen; i++)
                {
                    *pDataRx++ = hdrRx[i];
                }

                /* save number of bytes left to read */
                iRx = dataLen;
                if (iRx == 0)
                {
                    stateRx = HCI_RX_STATE_COMPLETE;
                }
                else
                {
                    stateRx = HCI_RX_STATE_DATA;
                }
            }
        }
        /* --- Data State --- */
        else if (stateRx == HCI_RX_STATE_DATA)
        {
            /* write incoming byte to allocated buffer */
            *pDataRx++ = dataByte;

            /* determine if entire packet has been read */
            iRx--;
            if (iRx == 0)
            {
                stateRx = HCI_RX_STATE_COMPLETE;
            }
            pBuf++;
            consumed_bytes++;
            len--;
        }

        /* --- Complete State --- */
        /* ( Note Well!  There is no else-if construct by design. ) */
        if (stateRx == HCI_RX_STATE_COMPLETE)
        {
            /* deliver data */
            if (pPktRx != NULL)
            {
                uart_irq_disable();
                g_bUartRxTimeoutFlag = true;
            }

            /* reset state machine */
            stateRx = HCI_RX_STATE_IDLE;
        }
    }
    return consumed_bytes;
}

/*************************************************************************************************/
/*!
 *  \fn     usb_rx_hci_state_machine
 *
 *  \brief  Receive function. Gets called by external code when USB bytes are received.
 *
 *  \param  pBuf   Pointer to buffer of incoming bytes.
 *  \param  len    Number of bytes in incoming buffer.
 *
 *  \return The number of bytes consumed.
 */
/*************************************************************************************************/
uint16_t usb_rx_hci_state_machine(uint8_t *pBuf, uint16_t len)
{
    static uint8_t    stateRx = HCI_RX_STATE_IDLE;
    static uint8_t    pktIndRx;
    static uint16_t   iRx;
    static uint8_t    hdrRx[HCI_ACL_HEADER_LEN];
    static uint8_t    pPktRx[HCI_MAX_RX_PACKET];
    static uint8_t    *pDataRx;
    uint8_t   dataByte;
    uint16_t  consumed_bytes;

    consumed_bytes = 0;
    /* loop until all bytes of incoming buffer are handled */
    while (len)
    {
        /* read single byte from incoming buffer and advance to next byte */
        dataByte = *pBuf;

        /* --- Idle State --- */
        if (stateRx == HCI_RX_STATE_IDLE)
        {
            /* save the packet type */
            pktIndRx = dataByte;
            iRx      = 0;
            stateRx  = HCI_RX_STATE_HEADER;
            pBuf++;
            consumed_bytes++;
            len--;
        }
        /* --- Header State --- */
        else if (stateRx == HCI_RX_STATE_HEADER)
        {
            uint8_t  hdrLen = 0;
            uint16_t dataLen = 0;

            /* determine header length based on packet type */
            if (pktIndRx == HCI_CMD_TYPE)
            {
                hdrLen = HCI_CMD_HEADER_LEN;
            }
            else if (pktIndRx == HCI_ACL_TYPE)
            {
                hdrLen = HCI_ACL_HEADER_LEN;
            }
            else
            {
                /* invalid packet type */
                stateRx = HCI_RX_STATE_IDLE;
                return consumed_bytes;
            }

            if (iRx != hdrLen)
            {
                /* copy current byte into the temp header buffer */
                hdrRx[iRx++] = dataByte;
                pBuf++;
                consumed_bytes++;
                len--;
            }

            /* see if entire header has been read */
            if (iRx == hdrLen)
            {
                uint8_t  i = 0;
                /* extract data length from header */
                if (pktIndRx == HCI_CMD_TYPE)
                {
                    dataLen = hdrRx[2];
                }
                else if (pktIndRx == HCI_ACL_TYPE)
                {
                    BYTES_TO_UINT16(dataLen, &hdrRx[2]);
                }

                pDataRx = pPktRx;

                /* copy header into data packet (note: memcpy is not so portable) */
                for (i = 0; i < hdrLen; i++)
                {
                    *pDataRx++ = hdrRx[i];
                }

                /* save number of bytes left to read */
                iRx = dataLen;
                if (iRx == 0)
                {
                    stateRx = HCI_RX_STATE_COMPLETE;
                }
                else
                {
                    stateRx = HCI_RX_STATE_DATA;
                }
            }
        }
        /* --- Data State --- */
        else if (stateRx == HCI_RX_STATE_DATA)
        {
            /* write incoming byte to allocated buffer */
            *pDataRx++ = dataByte;

            /* determine if entire packet has been read */
            iRx--;
            if (iRx == 0)
            {
                stateRx = HCI_RX_STATE_COMPLETE;
            }
            pBuf++;
            consumed_bytes++;
            len--;
        }

        /* --- Complete State --- */
        /* ( Note Well!  There is no else-if construct by design. ) */
        if (stateRx == HCI_RX_STATE_COMPLETE)
        {
            /* deliver data */
            if (pPktRx != NULL)
            {
                g_bUsbRxTimeoutFlag = true;
            }

            /* reset state machine */
            stateRx = HCI_RX_STATE_IDLE;
        }
    }
    return consumed_bytes;
}

//*****************************************************************************
//
// Write USB data.
//
//*****************************************************************************
void usb_data_write(uint8_t* pui8Data, uint32_t ui32Length)
{
    tud_cdc_write(pui8Data, ui32Length);
    tud_cdc_write_flush();
}

//*****************************************************************************
//
// RX callback of USB.
//
//*****************************************************************************
void tud_cdc_rx_cb(uint8_t itf)
{
#if ECHO_BACK_USB_DATA_ONLY
    // Read available data in USB FIFO
    uint32_t size = tud_cdc_n_available(itf);
    uint8_t buf[64];
    tud_cdc_n_read(itf, buf, size);

    // Echo back the read data to the sender
    usb_data_write(buf, size);
#else
    if (g_RunningMode == RUNNING_MODE_UART)
    {
        return;
    }
    else if (g_RunningMode == RUNNING_MODE_NONE)
    {
        g_RunningMode = RUNNING_MODE_USB;
    }

    uint32_t size = tud_cdc_n_available(itf);
    uint8_t * pData = (uint8_t *) &(g_psUsbWriteData.bytes[g_ui32UsbRxIndex]);

    tud_cdc_n_read(itf, pData, size);
    (void)usb_rx_hci_state_machine(pData, size);
    g_ui32UsbRxIndex += size;
#endif
}

//*****************************************************************************
//
// Interrupt handler for the UART.
//
//*****************************************************************************
#if UART_HCI_BRIDGE == 0
void am_uart_isr(void)
#elif UART_HCI_BRIDGE == 1
void am_uart1_isr(void)
#elif UART_HCI_BRIDGE == 2
void am_uart2_isr(void)
#elif UART_HCI_BRIDGE == 3
void am_uart3_isr(void)
#endif
{
    if (g_RunningMode == RUNNING_MODE_USB)
    {
        return;
    }
    else if (g_RunningMode == RUNNING_MODE_NONE)
    {
        g_RunningMode = RUNNING_MODE_UART;
    }

    uint32_t ui32Status;
    uint8_t * pData = (uint8_t *) &(g_psUartWriteData.bytes[g_ui32UartRxIndex]);

    //
    // Read the masked interrupt status from the UART.
    //
    am_hal_uart_interrupt_status_get(g_pvUART, &ui32Status, true);
    am_hal_uart_interrupt_clear(g_pvUART, ui32Status);
    am_hal_uart_interrupt_service(g_pvUART, ui32Status);
    //
    // If there's an RX interrupt, handle it in a way that preserves the
    // timeout interrupt on gaps between packets.
    //
    if (ui32Status & (AM_HAL_UART_INT_RX_TMOUT | AM_HAL_UART_INT_RX))
    {
        uint32_t ui32BytesRead;
        uart_data_read(pData, &ui32BytesRead);
        (void)uart_rx_hci_state_machine(pData, ui32BytesRead);
        g_ui32UartRxIndex += ui32BytesRead;
    }
}

//*****************************************************************************
//
// Write HCI data to controller.
//
//*****************************************************************************
static uint32_t hci_data_write(uint32_t* pui32Data, uint32_t ui32Length)
{
    return (am_devices_cooper_blocking_write(g_IomDevHdl, AM_DEVICES_COOPER_RAW, pui32Data, ui32Length, true));
}

//*****************************************************************************
//
// Read HCI data from controller.
//
//*****************************************************************************
static uint32_t hci_data_read(uint32_t* pui32Data, uint32_t* ui32Length)
{
    return (am_devices_cooper_blocking_read(g_IomDevHdl, pui32Data, ui32Length));
}

//*****************************************************************************
//
// Process the HCI received data from controller.
//
//*****************************************************************************
static void hci_rx_process(uint8_t mode, uint32_t* pui32Data, uint32_t ui32Length)
{
    if ( ( pui32Data == NULL ) || (ui32Length == 0) )
    {
        am_util_stdio_printf("invalid parameter\n");
        return;
    }

    am_devices_cooper_buffer(1024) psReadData;

    memcpy(psReadData.words, pui32Data, ui32Length);

    // Do not post the received hci_nop_opcode_event to serial port,
    // or it may cause the DTM tester to fail to recognize the received HCI data.
    if (memcmp(psReadData.bytes, &hci_nop_opcode_event[0], sizeof(hci_nop_opcode_event)))
    {
        if (mode == RUNNING_MODE_UART)
        {
            uart_data_write(psReadData.bytes, ui32Length);
        }
        else if (mode == RUNNING_MODE_USB)
        {
            usb_data_write(psReadData.bytes, ui32Length);
        }
    }
    else if ((ui32Length - sizeof(hci_nop_opcode_event)) > 0)
    {
        if (mode == RUNNING_MODE_UART)
        {
            uart_data_write(&psReadData.bytes[sizeof(hci_nop_opcode_event)], (ui32Length - sizeof(hci_nop_opcode_event)));
        }
        else if (mode == RUNNING_MODE_USB)
        {
            usb_data_write(&psReadData.bytes[sizeof(hci_nop_opcode_event)], (ui32Length - sizeof(hci_nop_opcode_event)));
        }
    }
    am_util_delay_ms(1);
}

//*****************************************************************************
//
// Process the received data from UART/USB.
//
//*****************************************************************************
static void serial_rx_process(uint32_t* pui32Data, uint32_t ui32Length)
{
    if ( ( pui32Data == NULL ) || (ui32Length == 0) )
    {
        am_util_stdio_printf("invalid parameter\n");
        return;
    }

    am_devices_cooper_buffer(1024) psWriteData;

    memcpy(psWriteData.words, pui32Data, ui32Length);

    // go to Cooper over HCI interface.
    hci_data_write(psWriteData.words, ui32Length);

}

//*****************************************************************************
//
// Callback to process the received data from HCI/Serial interface.
//
//*****************************************************************************
static void dtm_data_callback(uint8_t dataType, uint8_t mode, uint32_t* pui32Data, uint32_t length)
{
    switch ( dataType )
    {
        case HCI_RX_DATA:
            hci_rx_process(mode, pui32Data, length);
            break;
        case SERIAL_RX_DATA:
            serial_rx_process(pui32Data, length);
            break;
        default:
            break;
    }
}

//*****************************************************************************
//
// running_done_gpio_init.
//
//*****************************************************************************
static void running_done_gpio_init(void)
{
    // Running GPIO init
    am_hal_gpio_pinconfig(RUNNING_GPIO_PIN, am_hal_gpio_pincfg_output);
    am_hal_gpio_state_write(RUNNING_GPIO_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);

    // Done GPIO init
    am_hal_gpio_pinconfig(DONE_GPIO_PIN, am_hal_gpio_pincfg_output);
    am_hal_gpio_state_write(DONE_GPIO_PIN, AM_HAL_GPIO_OUTPUT_CLEAR);
}

//*****************************************************************************
//
// running_done_gpio_set.
//
//*****************************************************************************
static void running_done_gpio_set(bool is_running, bool is_done)
{
    am_hal_gpio_state_write(RUNNING_GPIO_PIN, (am_hal_gpio_write_type_e)is_running);
    am_hal_gpio_state_write(DONE_GPIO_PIN, (am_hal_gpio_write_type_e)is_done);
}

//*****************************************************************************
//
// Initialize the SPI communicating with cooper/controller.
//
//*****************************************************************************
void cooper_spi_init(void)
{
    uint32_t ui32Status;

    running_done_gpio_init();

    running_done_gpio_set(1, 0);

    //
    // Initialize the SPI module.
    //
    stCooperConfig.pNBTxnBuf = DMATCBBuf;
    stCooperConfig.ui32NBTxnBufLength = sizeof(DMATCBBuf) / 4;

    ui32Status = am_devices_cooper_init(SPI_MODULE, &stCooperConfig, &g_IomDevHdl, &g_pvHciSpiHandle);
    if ( ui32Status )
    {
        running_done_gpio_set(0, 0);
        return;
    }

    // disable sleep
    am_util_ble_nvds_set(g_IomDevHdl, nvds_cmd, sizeof(nvds_cmd));

    running_done_gpio_set(0, 1);
}

/*****************************************************************************/
/*!
 *  \fn     dtm_init
 *
 *  \brief  DTM initialization.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*****************************************************************************/
void dtm_init(void)
{
    //
    // Default setup.
    //
    am_bsp_low_power_init();

    //
    // Enable the ITM
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();

    //
    // Initialize serial interface.
    //
    uart_init();

    tusb_init();

    //
    // Enable interrupt service routines.
    //
    am_hal_interrupt_master_enable();

    //
    // Initialize SPI interface connecting with Cooper.
    //
    cooper_spi_init();
}

/*****************************************************************************/
/*!
 *  \fn     dtm_process
 *
 *  \brief  DTM processing, infinite loop.
 *
 *  \param  None.
 *
 *  \return None.
 */
/*****************************************************************************/
void dtm_process(void)
{
    uint32_t ui32NumChars;
    uint32_t ui32Status;
    uint32_t ui32IntStatus;
    //
    // Loop forever.
    //
    while ( 1 )
    {
        tud_task();

        //
        // Check for incoming traffic from either the UART/USB or the BLE interface.
        //
        ui32IntStatus = am_devices_cooper_irq_read();

        if ( ui32IntStatus > 0 )
        {
            if (g_RunningMode == RUNNING_MODE_UART)
            {
                //
                // If we have incoming BLE traffic, read it into a buffer.
                //
                ui32Status = hci_data_read(g_psUartReadData.words, &ui32NumChars);

                //
                // If the read was successful, echo it back out over the USB.
                //
                if ( ui32Status == AM_DEVICES_COOPER_STATUS_SUCCESS )
                {
                    dtm_data_callback(HCI_RX_DATA, RUNNING_MODE_UART, g_psUartReadData.words, ui32NumChars);
                    g_RunningMode = RUNNING_MODE_NONE;
                }
                else
                {
                    //
                    // Handle the error here.
                    //
                    am_util_stdio_printf("Read from BLE Controller failed\n");
                    while (1);
                }
            }
            else if (g_RunningMode == RUNNING_MODE_USB)
            {
                //
                // If we have incoming BLE traffic, read it into a buffer.
                //
                ui32Status = hci_data_read(g_psUsbReadData.words, &ui32NumChars);

                //
                // If the read was successful, echo it back out over the USB.
                //
                if ( ui32Status == AM_DEVICES_COOPER_STATUS_SUCCESS )
                {
                    dtm_data_callback(HCI_RX_DATA, RUNNING_MODE_USB, g_psUsbReadData.words, ui32NumChars);
                    g_RunningMode = RUNNING_MODE_NONE;
                }
                else
                {
                    //
                    // Handle the error here.
                    //
                    am_util_stdio_printf("Read from BLE Controller failed\n");
                    while (1);
                }
            }
        }
        else if ( g_bUartRxTimeoutFlag )
        {
            //
            // If we have incoming UART/USB traffic, the interrupt handler will
            // read it out for us, but we will need to echo it back out to the
            // radio manually.
            //
            if ( false == g_bUartCmdProcessedFlag )
            {
                dtm_data_callback(SERIAL_RX_DATA, g_RunningMode, g_psUartWriteData.words, g_ui32UartRxIndex);
            }

            g_ui32UartRxIndex = 0;
            g_bUartRxTimeoutFlag = false;
            uart_irq_enable();
        }
        else if ( g_bUsbRxTimeoutFlag )
        {
            //
            // If we have incoming UART/USB traffic, the interrupt handler will
            // read it out for us, but we will need to echo it back out to the
            // radio manually.
            //
            if ( false == g_bUsbCmdProcessedFlag )
            {
                dtm_data_callback(SERIAL_RX_DATA, g_RunningMode, g_psUsbWriteData.words, g_ui32UsbRxIndex);
            }

            g_ui32UsbRxIndex = 0;
            g_bUsbRxTimeoutFlag = false;
        }
    }
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
