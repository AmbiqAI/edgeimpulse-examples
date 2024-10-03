//*****************************************************************************
//
//! @file hello_world_uart.c
//!
//! @brief A uart example that demonstrates the async driver.
//!
//! @addtogroup peripheral_examples Peripheral Examples
//
//! @defgroup uart_async UART Asynchronous Example
//! @ingroup peripheral_examples
//! @{
//!
//! Purpose: This example demonstrates the usage of the asynchronous UART driver.<br>
//! This driver allows the application to append UART Tx data to the
//! Tx queue with interrupt code managing queue transmission.<br><br>
//!
//! Similarly, the interrupt code will move received data into the Rx queue
//! and the application periodically reads from the Rx queue.<br><br>
//!
//! The Rx timeout interrupt has been enabled in this example.<br>
//! If insufficient Rx data triggers the FIFO full interrupt,
//! the Rx timeout interrupt activates after a fixed delay.<br><br>
//!
//! The associated ISR handler am_hal_uart_interrupt_queue_service() will return
//! status in a bitfield, suitable for use as a callback or polling.<br><br>
//!
//! Default Configuration:<br>
//! By default, this example uses UART0. The I/O pins used are defined in the BSP
//! file as AM_BSP_GPIO_UART0_TX and AM_BSP_GPIO_UART0_RX<br><br>
//!
//! Configuration and Operation:<br>
//! - This example requires enabling Tx and Rx fifos and queues.<br>
//! - It operates in a non-blocking manner without using callbacks.<br>
//! - The example monitors (polls) the Rx input queue.<br>
//! - It will transmit small blocks of data every second.<br><br>
//!
//! To interact with these pins from a PC, the user should obtain a 1.8v uart/usb
//! cable (FTDI, etc.).<br>
//! Using a terminal application (CoolTerm, RealTerm, Minicomm, etc.),
//! the user will see data buffers being sent from the example
//! (a different buffer every second), and the user can send data by typing.<br>
//! The swo output will report the character count the example receives.<<br><br>
//!
//! The SWO output will send Rx/Tx status and error information.
//! SWO Printing takes place over the ITM at 1M Baud.

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
#include "timer_utils.h"
#include "uart_async_utils.h"

//
//! create buffer that will cause at least two fifo refills
//
uint8_t txBuf[74] =
{
    'a', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    //'0','1','2','3','4','5','6','7','8', '9',
    'z', '\r', '\n', ' ',
};

//
//! create a buffer that will not require a fifo refil
//! (so no tx interrupt, just tx complete)
//
uint8_t txBuf2[14] =
{
    'a', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'y', '\r', '\n', ' ',
};


//
//! define global ram used in this example
//
typedef struct
{
    //
    //! pointer to descptor associated to each uart used
    //! this descriptor is obtained when calling  init_uart()
    //
    uart_async_local_vars_t *uartIsrMap[eMAX_UARTS];

    //
    //! set when timer callback is called (1 Hz)
    //
    bool bTimerIntOccured;
}
uart_example_ramGlobals_t;

//
//! allocate global ram
//
uart_example_ramGlobals_t g_localv;


static void swo_enable(void);
static void uartAsyncTest(void);
static void timer_callback(void);

//*****************************************************************************
//
// Timer callback
//
//*****************************************************************************
static void
timer_callback(void)
{
    g_localv.bTimerIntOccured = true;

}
//*****************************************************************************
//
// enable swo
//
//*****************************************************************************
static void swo_enable(void)
{
    //
    // Initialize the printf interface for ITM output
    //
    am_bsp_itm_printf_enable();

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_stdio_printf("Uart Async SWO\n\n");

#ifdef UART_PRINT_ENABLE
    //
    // Set the main print interface to use the UART print function we defined.
    //
    am_util_stdio_printf_init(uart_print);
#endif
}

//*****************************************************************************
//
// sends and receives uart data using the asynchronous driver interface
//
//*****************************************************************************
static void uartAsyncTest(void)
{
    uint8_t rxBuf[32];

    timer_init(TMR_TICKS_PER_SECOND, timer_callback);

    am_util_stdio_printf("serial test started\n");

    // enable tx and tx interrupts
    for (uint32_t i = 0; i < NUM_UARTS_USED; i++)
    {
        uart_async_local_vars_t *puartAsync = g_localv.uartIsrMap[i];
        am_hal_uart_interrupt_enable(puartAsync->pvUART,
                                     (AM_HAL_UART_INT_RX | AM_HAL_UART_INT_TX | AM_HAL_UART_INT_RX_TMOUT));
    }

    uint32_t txCount = 0;

    while (1)
    {
        //
        // loop through all the uart descriptors
        //
        for (uint32_t i = 0; i < NUM_UARTS_USED; i++)
        {
            uart_async_local_vars_t *puartAsync = g_localv.uartIsrMap[i];
            // check through the uart status
            // read data
            // look at input status
            // could mask interrupts or disable ISR interrupts
            uart_id_e eUartId = puartAsync->uartId;

            am_hal_uart_status_t isrStatus;
            AM_CRITICAL_BEGIN
                //
                // clear off the accumulated status for this module
                //
                isrStatus = puartAsync->e32Status;
                puartAsync->e32Status &= ~(AM_HAL_UART_STATUS_INTRNL_MSK | AM_HAL_UART_STATUS_RX_DATA_AVAIL |
                                           AM_HAL_UART_STATUS_TX_COMPLETE);
            AM_CRITICAL_END

            //
            // print isr activity/status messages
            //
            if (isrStatus & AM_HAL_UART_STATUS_TX_COMPLETE)
            {
                am_util_stdio_printf("txComplete on uart %d\r\n", eUartId);
            }
            if (isrStatus & AM_HAL_UART_STATUS_INTRNL_MSK)
            {
                am_util_stdio_printf("uart error on uart %d : 0x%x\r\n", eUartId,
                                     isrStatus & AM_HAL_UART_STATUS_INTRNL_MSK);

            }
            if (isrStatus & AM_HAL_UART_STATUS_RX_DATA_AVAIL)
            {
                //
                // read uart after interrupt
                // here the uart will accumulate data until the fifo is full or a rx timeout occurs
                // Note: the rx-timeout interrupt must be enabled for the rx timeout to occur
                //
                int32_t i32numRx = am_hal_uart_get_rx_data(puartAsync->pvUART, rxBuf, sizeof(rxBuf));

                if (i32numRx < 0)
                {
                    am_util_stdio_printf("rxError on uart %d : %d\r\n", eUartId, -i32numRx);
                }
                else if (i32numRx > 0)
                {
                    am_util_stdio_printf("num bytes rxed on uart %d : %d\r\n", eUartId, i32numRx);
                }
            }
#ifdef POLL_UART
            else
            {
                // this iwll poll the uart and will dig data out of the fifo, depending on polling and data reate,
                // possibly before it causes an rx interrupt. This is typically used if the RX-TIMEOUT interrups is
                // NOT enabled.

                int32_t i32numRx = am_hal_uart_get_rx_data(pUartDesc->pvUART, rxBuf, sizeof(rxBuf));

                if (i32numRx < 0)
                {
                    am_util_stdio_printf("noRxIsr flag: rxError2 on uart %d : %d\r\n", eUartId, -i32numRx);
                }
                else if (i32numRx > 0)
                {
                    am_util_stdio_printf("noRxIsr flag: num bytes rxed on uart %d : %d %c\r\n", eUartId, i32numRx, rxBuf[0] );
                }

            }
#endif
        }

        if (g_localv.bTimerIntOccured)
        {
            //
            // timer interrupt, this is used to send data periodically
            //
            g_localv.bTimerIntOccured = false;

            txCount++;
            uint8_t *p;
            uint32_t txSize;
            if (txCount & 0x01)
            {
                p = txBuf;
                txSize = 73;
            }
            else
            {
                p = txBuf2;
                txSize = 13;
            }

            //
            // change first character every loop
            //
            uint32_t ax = (txCount % 26) + 'a';
            for (uint32_t i = 0; i < NUM_UARTS_USED; i++)
            {
                uart_async_local_vars_t *puartAsync = g_localv.uartIsrMap[i];
                p[0] = ax;
                uint32_t ui32TxStat = am_hal_uart_append_tx(puartAsync->pvUART, (uint8_t *) p, txSize);
                ui32TxStat += am_hal_uart_append_tx(puartAsync->pvUART, (uint8_t *) p, txSize);
                if (ui32TxStat)
                {
                    am_util_stdio_printf("serial tx, tx_start_stat: %d\n", ui32TxStat);
                }
            }
        } // g_localv.bTimerIntOccured
    } // while(1)

} // uartAsyncTest(void)

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
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
    //! use swo to send status messages
    //
    swo_enable();

    uart_async_local_vars_t *pUartAsync = (uart_async_local_vars_t *) 0;
    for ( uart_id_e uartNum = (uart_id_e) 0; uartNum < eMAX_UARTS; uartNum++)
    {
        uint32_t status = init_uart(uartNum, &pUartAsync);
        if (status != AM_HAL_STATUS_SUCCESS)
        {
            am_util_stdio_printf("uart init error %d %d\n program will hang", uartNum, status);
            while (true);
        }
        g_localv.uartIsrMap[uartNum] = pUartAsync;
    }

    am_hal_interrupt_master_enable();

    //
    // run the uart test, this will continue forever
    //
    uartAsyncTest();

    //
    // Loop forever while sleeping.
    //
    while (1)
    {
        //
        // Go to Deep Sleep.
        //
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
} // main
