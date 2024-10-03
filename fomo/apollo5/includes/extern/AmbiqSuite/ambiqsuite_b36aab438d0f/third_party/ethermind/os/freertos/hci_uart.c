
/**
 *  \file hci_uart.c
 *
 *  This File contains the routines to Transmit and Receive Data from
 *  the UART Transport Layer (Windows Version).
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* ----------------------------------------- Header File Inclusion */
#include "hci_uart_internal.h"

#ifdef FREERTOS
void hci_uart_init (void)
{
    /* Stub to keep compiler happy */
    return;
}

void hci_uart_bt_init (void)
{
    /* Stub to keep compiler happy */
    return;
}

void hci_uart_bt_shutdown (void)
{
    /* Stub to keep compiler happy */
    return;
}

/* HCI UART Send Data */
API_RESULT hci_uart_send_data
           (
               UCHAR type,
               UCHAR * buf,
               UINT16 length,
               UCHAR flag
           )
{
    /* Stub to keep compiler happy */
    return API_SUCCESS;
}

void hci_uart_set_serial_settings (CHAR *device, UINT32 speed)
{
    /* Stub to keep compiler happy */
    return;
}

#else /* FREERTOS */
#include "conf_example.h"

/* Atmel library includes. */
#include "freertos_usart_serial.h"

#ifdef BT_UART

/* #define HAVE_HCI_TX_RX_BYTE_COUNT */
/* ----------------------------------------- Function Declaraions */
/*
 *  This is a hack to support different Serial Port & Baudrate in UART Init.
 *  The following API is also defined in BT_api.h, but since UART does not
 *  include BT_api.h, it is redeclared here.
 */
/* This call should be made before the Stack in Initialized */
void hci_uart_set_serial_settings (CHAR *device, UINT32 speed);

DECL_STATIC BT_THREAD_RETURN_TYPE hci_uart_read_task (BT_THREAD_ARGS args);
DECL_STATIC void hci_uart_write_data (UCHAR *, UINT16);


/* ----------------------------------------- External Global Variables */
void hci_transport_enqueue (UCHAR * data, UINT16 datalen);


/* ----------------------------------------- Exported Global Variables */


/* ----------------------------------------- Static Global Variables */

/* UART Read Task State */
DECL_STATIC UCHAR hci_uart_state;


/* The size of the buffer used to receive characters from the USART driver. */
#define TRANSPORT_BUFFER_SIZE          HCI_TRANSPORT_MAX_PACKET_SIZE

/* The baud rate to use. */
#define USART_BAUD_RATE         (115200)

/* The buffer provided to the USART driver to store incoming data */
static uint8_t usart_drv_rx_buffer[TRANSPORT_BUFFER_SIZE] = {0};

static freertos_usart_if hci_usart_fd;


#ifdef HCI_UART_COLLECT_AND_WR_COMPLETE_PKT
/* Buffer for writing HCI Commands & ACL Data packets */
DECL_STATIC UCHAR hci_uart_wr_buf [HCI_UART_WR_BUF_SIZE];
#endif /* HCI_UART_COLLECT_AND_WR_COMPLETE_PKT */

/* UART Read Task Synchronization */
BT_DEFINE_MUTEX (hci_uart_mutex)
BT_DEFINE_COND (hci_uart_cond)

/* UART OverLap Synchronization */
BT_DEFINE_MUTEX (hci_uart_read)
BT_DEFINE_MUTEX (hci_uart_write)

#ifdef HAVE_HCI_TX_RX_BYTE_COUNT
/* Number of bytes Transmitted */
DECL_STATIC UINT32 hci_uart_tx_byte_count;

/* Number of bytes Received */
DECL_STATIC UINT32 hci_uart_rx_byte_count;
#endif /* HAVE_HCI_TX_RX_BYTE_COUNT */

#ifdef HAVE_HCI_TX_RX_BYTE_COUNT
/* Macro to increament Tx and Rx byte count */
#define HCI_UART_INC_TX_BYTES(nbytes) hci_uart_tx_byte_count += (nbytes)
#define HCI_UART_INC_RX_BYTES(nbytes) hci_uart_rx_byte_count += (nbytes)
#else
#define HCI_UART_INC_TX_BYTES(nbytes)
#define HCI_UART_INC_RX_BYTES(nbytes)
#endif /* HAVE_HCI_TX_RX_BYTE_COUNT */

DECL_STATIC HT_PARSE ht;

/* ----------------------------------------- Functions */

/** HCI-UART Initialization */
void hci_uart_init (void)
{
    BT_thread_type tid;

    BT_MUTEX_INIT_VOID (hci_uart_mutex, TRANSPORT);
    BT_COND_INIT_VOID(hci_uart_cond, TRANSPORT);

    /* Create Manual Events for Overlap Mode */
    BT_COND_INIT_VOID(hci_uart_read, TRANSPORT);
    BT_COND_INIT_VOID(hci_uart_write, TRANSPORT);

    /* Create a thread to receive data From Serial PORT and BUFFER it */
    if (0 != BT_thread_create(&tid, NULL, hci_uart_read_task, NULL))
    {
        HCI_UART_ERR(
        "[HCI-UART] Could NOT Create UART Read Thread\n");

        return;
    }

    /* Initialize UART State */
    hci_uart_state = 0x0;

    hci_usart_fd = 0;

    HCI_UART_TRC(
    "[HCI-UART] UART Power On Initialization Complete\n");

    return;
}


/** HCI-UART Bluetooth-ON Initialization */
void hci_uart_bt_init(void)
{
#if 0
    BT_SERIAL_PARAMS serial_params;

    HCI_UART_TRC(
    "[HCI-UART] Configuring Host Controller on Device : %s\n",
    hci_uart_device);

    /* Open UART Device */
    hci_uart_fd = CreateFile
                  (
                      hci_uart_device,
                      GENERIC_READ | GENERIC_WRITE,
                      0,
                      NULL,
                      OPEN_EXISTING,
                      FILE_FLAG_OVERLAPPED, /* Overlap Enabling */
                      NULL
                  );

    if (INVALID_HANDLE_VALUE == hci_uart_fd)
    {
        HCI_UART_ERR(
        "[HCI-UART] FAILED to Open UART Device: %s\n",
        hci_uart_device);

        return;
    }

#ifdef HAVE_HCI_TX_RX_BYTE_COUNT
    /* Initialize Number of Rx and Tx bytes */
    hci_uart_tx_byte_count = 0;
    hci_uart_rx_byte_count = 0;
#endif /* HAVE_HCI_TX_RX_BYTE_COUNT */

    /* Set Serial Port Settings */
    serial_params.use_parity = 0x0;
    serial_params.use_hw_flow = 0x1;
    serial_params.baudrate = hci_uart_baudrate;

#ifdef BT_BCSP
#ifndef BCSP_UART_USE_H4_DEFAULTS
    serial_params.use_parity = 0x1;
    serial_params.parity_type = 0x0;
    serial_params.use_hw_flow = 0x0;
    serial_params.parity_char = 0xC0;
#endif  /* BCSP_UART_USE_H4_DEFAULTS */
#endif /* BT_BCSP */

    /* Set UART/Serial Parameters */
    BT_set_serial_params (hci_uart_fd, &serial_params);

    /* Increasing the Buffer size with the UART for storing more data */
    if (!SetupComm(hci_uart_fd, 8196, 8196))
    {
        HCI_UART_ERR(
        "[HCI-UART] FAILED to Setup COMM for Buffer Size: %d\n",
        GetLastError());
    }

#else
    freertos_peripheral_options_t driver_options = {
        usart_drv_rx_buffer,                            /* The buffer used internally by the USART driver to store incoming characters. */
        TRANSPORT_BUFFER_SIZE,                          /* The size of the buffer provided to the USART driver to store incoming characters. */
        configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY,   /* The priority used by the USART interrupts. */
        USART_RS232,                                    /* Configure the USART for RS232 operation. */
        #if 0
        (USE_TX_ACCESS_MUTEX | USE_RX_ACCESS_MUTEX)
        #else
        (WAIT_TX_COMPLETE | USE_TX_ACCESS_MUTEX | USE_RX_ACCESS_MUTEX)
        #endif
    };

    const sam_usart_opt_t usart_settings = {
        USART_BAUD_RATE,
        US_MR_CHRL_8_BIT,
        US_MR_PAR_NO,
        US_MR_NBSTOP_1_BIT,
        US_MR_CHMODE_NORMAL,
        US_MR_USART_MODE_HW_HANDSHAKING,
        0 /* Only used in IrDA mode. */
        }; /*_RB_ TODO This is not SAM specific, not a good thing. */

        if (0 == hci_usart_fd)
        {
            /* Initialise the USART interface. */
            hci_usart_fd = freertos_usart_serial_init(BOARD_USART,
            &usart_settings,
            &driver_options);
            configASSERT(hci_usart_fd);
        }
#endif /* 0 */

    ht_parse_packet_init (&ht);

    /* Signal UART Read Task */
    hci_uart_lock (hci_uart_mutex);
    hci_uart_state = 0x1;
    hci_uart_signal (hci_uart_cond);
    hci_uart_unlock (hci_uart_mutex);

    HCI_UART_TRC(
    "[HCI-UART] UART Bluetooth-ON Initialization Complete\n");

    return;
}


/** HCI-UART Bluetooth-OFF Shutdown */
void hci_uart_bt_shutdown (void)
{
    /* Signal UART Read Task */
    hci_uart_lock (hci_uart_mutex);
    hci_uart_state = 0x0;
    hci_uart_unlock (hci_uart_mutex);

#if 0
    /* Wait for UART Read Task to Exit */
    BT_usleep (100);

    /* Close UART Device */
    CloseHandle (hci_uart_fd);
#endif /* 0 */
}


/** HCI-UART Read Task */
DECL_STATIC BT_THREAD_RETURN_TYPE hci_uart_read_task (BT_THREAD_ARGS args)
{
    UCHAR rdata [1024];
    UINT32 nbytes, marker;

    /* Initialize HT Parse data */
    marker = 0;
    BT_mem_set (rdata, 0x00, sizeof(rdata));
    ht.packet = rdata;

#if 0
    DWORD modemstatus;
    DWORD dwerror;
    COMSTAT portstat;
    OVERLAPPED overlap;

    /* Initialize */
    overlap.hEvent = hci_uart_read;
    overlap.Offset = 0;
    overlap.OffsetHigh = 0;
    modemstatus = dwerror = 0;

    /* Masking the event of Byte transfer */
    SetCommMask (hci_uart_fd, EV_RXCHAR);
#endif /* 0 */

    BT_LOOP_FOREVER()
    {
        hci_uart_lock (hci_uart_mutex);

        if (0x0 == hci_uart_state)
        {
            HCI_UART_TRC(
            "[HCI-UART] UART Read Task: Waiting to Service ..\n");

            hci_uart_wait (hci_uart_cond, hci_uart_mutex);

            HCI_UART_TRC(
            "[HCI-UART] UART Read Task: Ready to Service ..\n");
        }

        hci_uart_unlock (hci_uart_mutex);

#if 0
        /* Blocking on UART till it recieves any data */
        WaitCommEvent (hci_uart_fd, &modemstatus, 0);
        SetCommMask (hci_uart_fd, EV_RXCHAR);

        if (modemstatus & EV_RXCHAR)
        {
            ClearCommError (hci_uart_fd, &dwerror, &portstat);

            while (portstat.cbInQue > 0)
            {
                toread = (portstat.cbInQue > 1000) ? 1000 : portstat.cbInQue;

                /* Read Data */
                if (!ReadFile(hci_uart_fd, data, toread, &nbytes, &overlap))
                {
                    while (!GetOverlappedResult(hci_uart_fd,
                                                &overlap, &nbytes, FALSE))
                    {
                        if (ERROR_IO_INCOMPLETE == GetLastError())
                        {
                            /* Could not proceed as some error Occured */
                            ClearCommError(hci_uart_fd, &dwerror, &portstat);

                            HCI_UART_ERR(
                            "[HCI-UART] ReadFile: ERROR I/O Complete: %d\n",
                            dwerror);

                            BT_usleep (20);
                            break;
                        }
                    }

                    ResetEvent (hci_uart_read);
                }

                if (nbytes > 0)
                {
                    /* Deliver Data */
                    hci_transport_enqueue (data, (UINT16)nbytes);
                    portstat.cbInQue -= nbytes;
                }
            }
        }
        else
        {
            BT_usleep (20);
            continue;
        }
#else
        /**
         *  With portMAX_DELAY based on the serial read interface assuming
         *  the function will return only after reading ht.packet_expected_len bytes.
         */
        nbytes = freertos_usart_serial_read_packet
                 (
                     hci_usart_fd,
                     (rdata + marker),
                     ht.packet_expected_len,
                     portMAX_DELAY
                 );

        if (nbytes > 0)
        {
            HCI_UART_INC_RX_BYTES(nbytes);

            /* Increment the marker */
            marker += nbytes;

            /* Get the data parsed */
            ht_parse_packet (&ht);

            if (1 == ht.packet_expected_len)
            {
                /* Deliver Data */
                hci_transport_enqueue (rdata, (UINT16)marker);

                marker = 0;
            }
        }
#endif /* 0 */
    }

    return BT_THREAD_RETURN_VAL_SUCCESS;
}


#ifdef HCI_UART_COLLECT_AND_WR_COMPLETE_PKT
/** HCI-UART Send Data */
API_RESULT hci_uart_send_data
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

        if (total_len > HCI_UART_WR_BUF_SIZE)
        {
            HCI_UART_ERR(
            "[HCI-UART] HCI Packet Size %d exceeds Configuration %d bytes\n",
            total_len, HCI_UART_WR_BUF_SIZE);

            return API_FAILURE;
        }
    }

    if (acl_data_pkt)
    {
        if (0 == cur_len)
        {
            hci_uart_wr_buf[0] = type;
            cur_len = 1;
        }

        BT_mem_copy (hci_uart_wr_buf + cur_len, buf, length);
        cur_len += length;

        if (cur_len != total_len)
        {
            return API_SUCCESS;
        }
    }
    else
    {
        hci_uart_wr_buf[0] = type;
        BT_mem_copy (hci_uart_wr_buf + 1, buf, length);
        total_len = length + 1;
    }

    /* Write HCI Packet */
    hci_uart_write_data (hci_uart_wr_buf, total_len);

    /* Re-initialize */
    cur_len = total_len = 0;
    if (BT_FALSE != acl_data_pkt)
    {
        acl_data_pkt = BT_FALSE;
    }

    return API_SUCCESS;
}
#else
/** HCI-UART Send Data */
API_RESULT hci_uart_send_data
           (UCHAR type, UCHAR * buf, UINT16 length, UCHAR flag)
{
    /* Write Packet Identifier */
    if (0x1 == flag)
    {
        hci_uart_write_data (&type, 1);
    }

    /* Write HCI Packet */
    hci_uart_write_data (buf, length);

    return API_SUCCESS;
}
#endif /* HCI_UART_COLLECT_AND_WR_COMPLETE_PKT */


/** HCI-UART Write Data */
DECL_STATIC void hci_uart_write_data (UCHAR * buf, UINT16 length)
{
#if 0
    UCHAR * tmp_ptr;
    UINT32 written, nbytes;
    DWORD dwerror;
    OVERLAPPED overlap;
    COMSTAT portstat;

    /* Initialize */
    overlap.hEvent = hci_uart_write;
    overlap.Offset = 0;
    overlap.OffsetHigh = 0;
    dwerror = 0;

    written = 0;
    tmp_ptr = buf;
    while (written < length)
    {
        ClearCommError (hci_uart_fd, &dwerror, &portstat);
        if ((CE_TXFULL == dwerror) || (CE_IOE == dwerror))
        {
            BT_usleep (20);
            continue;
        }

        /* Write Data */
        if (!WriteFile(hci_uart_fd,
                       tmp_ptr, (length-written), &nbytes, &overlap))
        {
            if (ERROR_IO_PENDING == GetLastError())
            {
                while (!GetOverlappedResult(hci_uart_fd,
                                            &overlap, &nbytes, TRUE))
                {
                    if (ERROR_IO_INCOMPLETE == GetLastError())
                    {
                        ClearCommError(hci_uart_fd, &dwerror, &portstat);

                        HCI_UART_ERR(
                        "[HCI-UART] WriteFile: ERROR I/O Complete: %d\n",
                        dwerror);

                        BT_usleep (20);
                        break;
                    }
                }

                ResetEvent(hci_uart_write);
            }
        }

        tmp_ptr += nbytes;
        written += nbytes;
    }
#else
    freertos_usart_write_packet
    (
        hci_usart_fd,
        buf,
        length,
        portMAX_DELAY
    );

    HCI_UART_INC_TX_BYTES(length);
#endif /* 0 */

#ifdef UART_TEST_MODE
    BT_debug_dump_bytes (buf, length);
#endif /* UART_TEST_MODE */
}
#endif /* FREERTOS */
#endif /* BT_UART */


