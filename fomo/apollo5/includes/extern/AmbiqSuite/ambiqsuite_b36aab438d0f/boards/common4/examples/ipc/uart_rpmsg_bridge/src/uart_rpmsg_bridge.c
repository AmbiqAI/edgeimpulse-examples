//*****************************************************************************
//
//! @file uart_rpmsg_bridge.c
//!
//! @brief UART driver for uart hci bridge
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

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "rpc_common.h"

//*****************************************************************************
//
// Configuration options
//
//*****************************************************************************
//
// Define the UART module (0 or 1) to be used.
// Also define the max packet size
//
#define UART_RPMSG_BRIDGE               AM_BSP_UART_PRINT_INST
#define MAX_UART_PACKET_SIZE            2048
#define UART_RX_TIMEOUT_MS              1

#define HCI_CMD_TYPE                    1
#define HCI_ACL_TYPE                    2
#define HCI_SCO_TYPE                    3
#define HCI_ISO_TYPE                    5
#define HCI_CMD_HEADER_LEN              3
#define HCI_ACL_HEADER_LEN              4
#define HCI_SCO_HEADER_LEN              3
#define HCI_ISO_HEADER_LEN              4
#define MAX_READ_BYTES                  23
//compatible for WVT case requiring send more than 256 data(eg. case hci_cfc_tc_01 may send 265bytes data)
#define HCI_MAX_RX_PACKET               512

typedef enum
{
    HCI_RX_STATE_IDLE,
    HCI_RX_STATE_HEADER,
    HCI_RX_STATE_DATA,
    HCI_RX_STATE_COMPLETE
} hciRxState_t;

//*****************************************************************************
//
// Global Variables
//
//*****************************************************************************
uint8_t g_pui8UARTTXBuffer[MAX_UART_PACKET_SIZE];
uint8_t g_pui8UARTRXBuffer[MAX_UART_PACKET_SIZE];
uint8_t test_buff[MAX_UART_PACKET_SIZE];
volatile uint32_t g_ui32UARTRxIndex = 0;
volatile bool g_bRxTimeoutFlag = false;
void *g_pvUART;

//*****************************************************************************
//
// Process "special" UART commands.  Format is:
//      'A'     Header
//      'M'
//      'Y'     Command (ASCII '0' - '2')
//       X      Value   (0 - 255)
//       X
//
//*****************************************************************************

/*************************************************************************************************/
/*!
 *  \fn     hciUartRxIncoming
 *
 *  \brief  Receive function.  Gets called by external code when bytes are received.
 *
 *  \param  pBuf   Pointer to buffer of incoming bytes.
 *  \param  len    Number of bytes in incoming buffer.
 *
 *  \return The valid hci data number of bytes consumed.
 */
/*************************************************************************************************/
uint16_t hciUartRxIncoming(uint8_t *pBuf, uint16_t len)
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
            else if (pktIndRx == HCI_SCO_TYPE)
            {
                hdrLen = HCI_SCO_HEADER_LEN;
            }
            else if (pktIndRx == HCI_ISO_TYPE)
            {
                hdrLen = HCI_ISO_HEADER_LEN;
            }
            else
            {
                /* invalid packet type */
                stateRx = HCI_RX_STATE_IDLE;
                /* discard this packet */
                return 0;
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
                if ((pktIndRx == HCI_CMD_TYPE) || (pktIndRx == HCI_SCO_TYPE))
                {
                    dataLen = hdrRx[2];
                }
                else if ((pktIndRx == HCI_ACL_TYPE) || (pktIndRx == HCI_ISO_TYPE))
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
                g_bRxTimeoutFlag = true;
            }
            /* reset state machine */
            stateRx = HCI_RX_STATE_IDLE;
        }
    }
    return consumed_bytes;
}

//*****************************************************************************
//
// Read UART data.
//
//*****************************************************************************
void serial_data_read(uint8_t* pui8Data, uint32_t* ui32Length)
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
void serial_data_write(uint8_t* pui8Data, uint32_t ui32Length)
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
void serial_irq_enable(void)
{
    NVIC_SetPriority((IRQn_Type)(UART0_IRQn + UART_RPMSG_BRIDGE), AM_IRQ_PRIORITY_DEFAULT);
    NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + UART_RPMSG_BRIDGE));
}

//*****************************************************************************
//
// Disable UART IRQ.
//
//*****************************************************************************
void serial_irq_disable(void)
{
    NVIC_DisableIRQ((IRQn_Type)(UART0_IRQn + UART_RPMSG_BRIDGE));
}

//*****************************************************************************
//
// Interrupt handler for the UART.
//
//*****************************************************************************
#if UART_RPMSG_BRIDGE == 0
void am_uart_isr(void)
#elif UART_RPMSG_BRIDGE == 1
void am_uart1_isr(void)
#elif UART_RPMSG_BRIDGE == 2
void am_uart2_isr(void)
#elif UART_RPMSG_BRIDGE == 3
void am_uart3_isr(void)
#endif
{
    uint32_t ui32Status;
    uint8_t * pData = &(g_pui8UARTRXBuffer[g_ui32UARTRxIndex]);

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
        serial_data_read(pData, &ui32BytesRead);
        g_ui32UARTRxIndex += (hciUartRxIncoming(pData, ui32BytesRead));
        if ( g_bRxTimeoutFlag )
        {
            serial_irq_disable();
            ipc_set_event(IPC_HCI_TX_BIT);
        }
    }
}

//*****************************************************************************
//
// Initialize the UART.
//
//*****************************************************************************
void serial_interface_init(void)
{
    //
    // Start the UART.
    //
    am_hal_uart_config_t sUartConfig =
    {
        //
        // Standard UART settings: 921600-8-N-1
        //
        .ui32BaudRate    = 921600,
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

    am_hal_uart_initialize(UART_RPMSG_BRIDGE, &g_pvUART);
    am_hal_uart_power_control(g_pvUART, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_uart_configure(g_pvUART, &sUartConfig);
    am_hal_uart_buffer_configure(g_pvUART, g_pui8UARTTXBuffer, sizeof(g_pui8UARTTXBuffer), NULL, 0);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX);
    //
    // Make sure to enable the interrupts for RX, since the HAL doesn't already
    // know we intend to use them.
    //
    serial_irq_enable();
    //NVIC_EnableIRQ((IRQn_Type)(UART0_IRQn + UART_RPMSG_BRIDGE));
    am_hal_uart_interrupt_enable(g_pvUART, (AM_HAL_UART_INT_RX |
                                           AM_HAL_UART_INT_RX_TMOUT |
                                           AM_HAL_UART_INT_OVER_RUN |
                                           AM_HAL_UART_INT_TX |
                                           AM_HAL_UART_INT_TXCMP));
}
