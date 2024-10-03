//*****************************************************************************
//
//! @file freertos_sampler.c
//!
//! @brief FreeRTOS example that showcases many freeRTOS features.
//!
//! This FreeRTOS example provides samples of many uses of freeRTOS features.
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

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

#include "FreeRTOS.h"
#include "task.h"
#include "portmacro.h"
#include "portable.h"
#include "semphr.h"
#include "event_groups.h"

#include "freertos_sampler.h"
#include "rtos.h"

#include "itm_task.h"

#include "app_task.h"
#include "serial_task.h"
#include "ser_queue.h"
//
TaskHandle_t xTaskDelayTask;
TaskHandle_t xTaskNotifyTest;

//
//*****************************************************************************
//
// UART configuration settings.
//
//*****************************************************************************
am_hal_uart_config_t g_sUartConfig =
{
    .ui32BaudRate = 115200,
    .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
    .bTwoStopBits = false,
    .ui32Parity   = AM_HAL_UART_PARITY_NONE,
    .ui32FlowCtrl = AM_HAL_UART_FLOW_CTRL_NONE,
};

//
//
#define MAX_BUFFER 128
volatile uint32_t ui32_serial_status;
volatile unsigned char cRecChar, RX_FIFO_FULL, RX_FIFO_EMPTY;
volatile unsigned char uart_RX_buffer[MAX_BUFFER];
volatile unsigned char uart_RX_cnt, uart_RX_head, uart_RX_tail;
// ************************************
unsigned char cTxChar, TX_FIFO_FULL, TX_FIFO_EMPTY;
unsigned char uart_TX_buffer[MAX_BUFFER];
unsigned char uart_TX_cnt, uart_TX_head, uart_TX_tail;
// ************************************



void
am_uart1_isr( void )
{
    uint32_t ui32Status;
    SERQueueElement_t msg;

    //
    // Check to see what caused this interrupt, then clear the bit from the
    // interrupt register.
    //
                                                                                                                                                                                   ui32Status = am_hal_uart_int_status_get(AM_BSP_UART_BOOTLOADER_INST, false);
     am_hal_uart_int_clear(AM_BSP_UART_BOOTLOADER_INST, ui32Status);
    // ui32_serial_status = ui32Status;
    //
    // Service the uart FIFO.
    //

        uart_RX_buffer[uart_RX_head++] = AM_REGn(UART, AM_BSP_UART_BOOTLOADER_INST, DR);
        uart_RX_head = uart_RX_head & (MAX_BUFFER - 1);

//         am_devices_led_toggle(am_bsp_psLEDs,2);     // dv*** 10242016


        msg.u.RTOS_event = 2;

        if ( msg.u.RTOS_event != 0 )
        {
            //    msg.u.RTOS_event = 0;
            SERQueue_sendFromISR(&msg);
        }


}


void
ser_out( unsigned char data )
{
    AM_REGn(UART, AM_BSP_UART_BOOTLOADER_INST, DR) = data;
    while ( AM_BFRn(UART, AM_BSP_UART_BOOTLOADER_INST, FR, TXFF) );
}


//
//*****************************************************************************
//
void
hex_out( unsigned char hex_data )
{
    unsigned char temp;
    temp = hex_data >> 4;
    if ( temp > 10 )
    {
        ser_out( temp + 0x37);
    }
    else
    {
        ser_out( temp + 0x30);
    }

    temp = hex_data & 0x0f;
    if ( (hex_data & 0x0f ) > 10 )
    {
        ser_out( temp + 0x37);
    }
    else
    {
        ser_out( temp + 0x30);
    }
}

void
hex_out_int( int ihex_data)
{
    hex_out( ihex_data >> 8 );
    hex_out( ihex_data & 0x00ff);
}


//*****************************************************************************
//
// The User TaskNotifyTest Code.
//
//*****************************************************************************
void
TaskNotifyTestSetUp(void)
{
    am_util_debug_printf("TaskNotifyTestSetUp: SetUp\n\r");

    xTaskCreate(TaskNotifyTest, "TaskNotifyTest", 256 << 1, 0, configMAX_PRIORITIES - 2, &xTaskNotifyTest);

}


//*****************************************************************************
//
// The User TaskNotifyTest Code.
//
//*****************************************************************************
void
TaskNotifyTest(void *pvParameters)
{
    static uint8_t TaskNotifyCount = 0;

    am_util_debug_printf("TaskNotifyTest: starting\n\r");

    while (1)
    {
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY);

        am_devices_led_toggle(am_bsp_psLEDs, 2);

        TaskNotifyCount++;
        am_util_debug_printf("TaskNotify Count: %d\n\r", TaskNotifyCount);
    }

}


//*****************************************************************************
//
// The User TaskDelay Code.
//
//*****************************************************************************
void
TaskDelayTask(void *pvParameters)
{
    uint32_t xLastWakeTime;
    static uint8_t SwitchTime = 0;

    am_util_debug_printf("TaskDelayTask: starting\n\r");

    xLastWakeTime = xTaskGetTickCount();
    while (1)
    {

        am_devices_led_toggle(am_bsp_psLEDs, 1);
        //        ser_out('a');
        //        am_util_debug_printf("TaskDelayTask: Last Wake = %d\n\r", xLastWakeTime);

        //        am_devices_led_toggle(am_bsp_psLEDs,0);
        //        am_util_debug_printf("TaskDelayTask: %d\n\r",SwitchTime);
        SwitchTime++;
        if ( (SwitchTime & 0x03) == 0 )
        {
            vTaskDelayUntil(&xLastWakeTime, 31);
        }
        else
            if ( (SwitchTime & 0x03) == 1 )
            {
                vTaskDelayUntil(&xLastWakeTime, 8);
            }
            else
                if ( (SwitchTime & 0x03) == 2 )
                {
                    vTaskDelayUntil(&xLastWakeTime, 55);
                }
                else
                    if ( (SwitchTime & 0x03) == 3 )
                    {
                        vTaskDelayUntil(&xLastWakeTime, 24000);
                    }
    }

}

// *****************************************************************************
//
// Enable ITM printing.
//
// *****************************************************************************

void enable_itm_print(char *msg)
{
    //
    // Setup ITM pin for plotting
    //
    am_bsp_pin_enable(ITM_SWO);

    //
    // Initialize the printf interface for ITM/SWO output.
    //
    am_util_stdio_printf_init((am_util_stdio_print_char_t) am_bsp_itm_string_print);

    //
    // Initialize the SWO GPIO pin
    //
    am_bsp_pin_enable(ITM_SWO);

    //
    // Enable the ITM.
    //
    am_hal_itm_enable();

    //
    // Enable debug printf messages using ITM on SWO pin
    //
    am_bsp_debug_printf_enable();

    //
    // Clear the terminal, and print a message to show that we're up and
    // running.
    //
    am_util_stdio_terminal_clear();
    if ( msg != NULL )
    {
        am_util_stdio_printf(msg);
    }
}

//
// ************************************

uint32_t command;
unsigned char packet_cnt, pkt_size;

void
setup_serial( int32_t i32Module)
{

    command = 0;
    uart_RX_head = uart_RX_tail = uart_RX_cnt = 0;
    RX_FIFO_FULL = 0;  // check serial FIFO operation
    RX_FIFO_EMPTY = 1;
    packet_cnt = 0xff;


    //
    // Make sure the UART RX and TX pins are enabled.
    //
    am_bsp_pin_enable(COM_UART_TX);
    am_bsp_pin_enable(COM_UART_RX);

    //
    // Power on the selected UART
    //
    am_hal_uart_pwrctrl_enable(i32Module);

    //
    // Start the UART interface, apply the desired configuration settings, and
    // enable the FIFOs.
    //
    am_hal_uart_clock_enable(i32Module);

    //
    // Disable the UART before configuring it.
    //
    am_hal_uart_disable(i32Module);

    //
    // Configure the UART.
    //
    am_hal_uart_config(i32Module, &g_sUartConfig);

    //
    // Enable the UART FIFO.
    //
 //   am_hal_uart_fifo_config(i32Module, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);

    //
    // Enable the UART.
    //
    am_hal_uart_enable(i32Module);

    am_hal_uart_int_enable(i32Module, AM_HAL_UART_INT_RX);
}
//*****************************************************************************
//
// Enable the UART
//
//*****************************************************************************
void
uart_enable(int32_t i32Module)
{
    //
    // Enable the UART clock.
    //
    am_hal_uart_clock_enable(i32Module);

    //
    // Enable the UART.
    //
    am_hal_uart_enable(i32Module);
    am_hal_uart_int_enable(i32Module, AM_HAL_UART_INT_RX);
    //
    // Enable the UART pins.
    //
    am_bsp_pin_enable(COM_UART_TX);
    am_bsp_pin_enable(COM_UART_RX);
}

void Uart_mode_init( void )
{
    //
    // Initialize and Enable the UART.
    //

    setup_serial(AM_BSP_UART_BOOTLOADER_INST);
    am_hal_uart_int_clear(AM_BSP_UART_BOOTLOADER_INST, AM_REG_UART_IER_RXIM_M);
    am_hal_uart_int_enable(AM_BSP_UART_BOOTLOADER_INST, AM_REG_UART_IER_RXIM_M);

    am_hal_interrupt_priority_set(AM_HAL_INTERRUPT_UART1, configKERNEL_INTERRUPT_PRIORITY);

#if AM_BSP_UART_BOOTLOADER_INST == 0
        am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART);
#else
        am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART1);
#endif

//    uart_enable(1);

    am_hal_interrupt_master_enable();


}

//*****************************************************************************
//
// Main Function
//
//*****************************************************************************
int
main(void)
{
    //
    // Set the clock frequency.
    //
    am_hal_clkgen_sysclk_select(AM_HAL_CLKGEN_SYSCLK_MAX);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_enable(&am_hal_cachectrl_defaults);

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Run the RTC off the XTAL.
    //
    am_hal_rtc_osc_select(AM_HAL_RTC_OSC_XT);

    //
    // Restart the XTAL oscillator which was stopped in am_bsp_low_power_init.
    //
    am_hal_clkgen_osc_start(AM_HAL_CLKGEN_OSC_XT);

    //
    // Disable the RTC.
    //
    am_hal_rtc_osc_enable();

    enable_itm_print("itm print enabled\r\n");

    //
    // Print the banner.
    //
    am_util_stdio_printf("STDIO Printf Example\n");

    //
    // Enable the ITM and debug printf messages using ITM on SWO pin.
    //
    // Command line -DAM_DEBUG_PRINTF

    #ifdef AM_DEBUG_PRINTF          // Defined in the Command Line
    am_bsp_debug_printf_enable();
    am_hal_itm_enable();
    am_bsp_pin_enable(ITM_SWO);

    //
    // Print the banner.
    //
    am_util_stdio_terminal_clear();
    am_util_debug_printf("freeRTOS Sampler\n");
    #endif

    am_devices_led_array_init(am_bsp_psLEDs, AM_BSP_NUM_LEDS);
    am_devices_led_off(am_bsp_psLEDs, 0);
    am_devices_led_off(am_bsp_psLEDs, 1);
    am_devices_led_off(am_bsp_psLEDs, 2);
    am_devices_led_off(am_bsp_psLEDs, 3);

    Uart_mode_init();
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// NOW we can run the FreeRTOS sampler.
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

    //
    // Start the RTOS scheduler. This never returns.
    //
    run_tasks();

    //
    // Loop forever.
    //
    while (1) // main function should never get here.
    {
    }

}
