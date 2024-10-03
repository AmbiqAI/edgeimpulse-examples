//*****************************************************************************
//
//! @file mindtree_pxm.c
//!
//! @brief Test application to see if Mindtree will compile.
//!
//! Detailed description.
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************
#include "stdio.h"
#include "stdarg.h"
#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"
#include "FreeRTOS.h"
#include "task.h"

#define UART_DEBUG

extern int appl_init(void);

#ifdef UART_DEBUG
char        rbuf[128];
void *UART;
//*****************************************************************************
//
// UART configuration settings.
//
//*****************************************************************************
void
setup_serial(int32_t i32Module)
{
    //
    // Enable a UART to use for the menu.
    //
    const am_hal_uart_config_t sUartConfig =
    {
        //
        // Standard UART settings: 115200-8-N-1
        //
        .ui32BaudRate = 115200,
        .ui32DataBits = AM_HAL_UART_DATA_BITS_8,
        .ui32Parity = AM_HAL_UART_PARITY_NONE,
        .ui32StopBits = AM_HAL_UART_ONE_STOP_BIT,
        .ui32FlowControl = AM_HAL_UART_FLOW_CTRL_NONE,

        //
        // Set TX and RX FIFOs to interrupt at half-full.
        //
        .ui32FifoLevels = (AM_HAL_UART_TX_FIFO_1_2 |
                           AM_HAL_UART_RX_FIFO_1_2),

        //
        // Buffers
        //
        .pui8TxBuffer = 0,
        .ui32TxBufferSize = 0,
        .pui8RxBuffer = 0,
        .ui32RxBufferSize = 0,
    };

    am_hal_uart_initialize(0, &UART);
    am_hal_uart_power_control(UART, AM_HAL_SYSCTRL_WAKE, false);
    am_hal_uart_configure(UART, &sUartConfig);

    //
    // Make sure the UART interrupt priority is set low enough to allow
    // FreeRTOS API calls.
    //
    NVIC_SetPriority(UART0_IRQn, NVIC_configMAX_SYSCALL_INTERRUPT_PRIORITY);


    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_TX, g_AM_BSP_GPIO_COM_UART_TX);
    am_hal_gpio_pinconfig(AM_BSP_GPIO_COM_UART_RX, g_AM_BSP_GPIO_COM_UART_RX);

    //
    // Enable UART RX interrupts manually.
    //
    am_hal_uart_interrupt_clear(UART, AM_HAL_UART_INT_RX | AM_HAL_UART_INT_RX_TMOUT);
    am_hal_uart_interrupt_enable(UART, AM_HAL_UART_INT_RX | AM_HAL_UART_INT_RX_TMOUT);
    NVIC_EnableIRQ(UART0_IRQn);
}


// buffer for printf
static char g_prfbuf[AM_PRINTF_BUFSIZE];

uint32_t
am_uart_printf(const char *pcFmt, ...)
{
    uint32_t ui32NumChars;

    //
    // Convert to the desired string.
    //
    va_list pArgs;
    va_start(pArgs, pcFmt);
    ui32NumChars = am_util_stdio_vsprintf(g_prfbuf, pcFmt, pArgs);
    va_end(pArgs);

    //
    // This is where we print the buffer to the configured interface.
    //
    am_hal_uart_transfer_t sSend =
    {
        .ui32Direction = AM_HAL_UART_WRITE,
        .pui8Data = (uint8_t *)g_prfbuf,
        .ui32NumBytes = ui32NumChars,
        .ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER,
        .pui32BytesTransferred = 0,
    };

    am_hal_uart_transfer(UART, &sSend);

    //
    // return the number of characters printed.
    //
    return ui32NumChars;
}

uint32_t am_uart_scanf(const char *pcFmt, ...)
{

  char r_byte;
  int  count;
  unsigned int c_count = 0;
  va_list pArgs;
  rbuf[c_count] = 0;
  while(1)
  {
    const am_hal_uart_transfer_t sGetChar =
    {
        .ui32Direction = AM_HAL_UART_READ,
        .pui8Data = (uint8_t *) &r_byte,
        .ui32NumBytes = 1,
        .ui32TimeoutMs = AM_HAL_UART_WAIT_FOREVER,
        .pui32BytesTransferred = 0,
    };

    am_hal_uart_transfer(UART, &sGetChar);

    //am_hal_uart_char_transmit_polled(i32Module, r_byte);

    if ( r_byte == 0x0d || r_byte == 0x0a )
    {

      rbuf[c_count] = 0; //EOF
      break;
    }
    rbuf[c_count++] = r_byte;

  }
  va_start(pArgs, pcFmt);
  count = vsscanf(rbuf, pcFmt, pArgs);
  va_end(pArgs);
  return (count);
}
#endif

/* Task to be created. */
void vPXMTask( void * pvParameters )
{
    /* The parameter value is expected to be 1 as 1 is passed in the
    pvParameters value in the call to xTaskCreate() below. */
    configASSERT( ( ( uint32_t ) pvParameters ) == 1 );
    appl_init();
}

/* Function that creates a task. */
void vCreatePXMTask( void )
{
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

    /* Create the task, storing the handle. */
    xReturned = xTaskCreate(
                    vPXMTask,           /* Function that implements the task. */
                    "PXMTask",          /* Text name for the task. */
                    512,                /* Stack size in words, not bytes. */
                    ( void * ) 1,       /* Parameter passed into the task. */
                    2,   /* Priority at which the task is created. */
                    &xHandle );         /* Used to pass out the created task's handle. */

    if ( xReturned == pdPASS )
    {
        /* The task was created.  Use the task's handle to delete the task. */
        //vTaskDelete( xHandle );
    }
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
    // Set the clock frequency
    //
    am_hal_clkgen_control(AM_HAL_CLKGEN_CONTROL_SYSCLK_MAX, 0);

    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

#ifndef NOFPU
    //
    // Enable the floating point module, and configure the core for lazy
    // stacking.
    //
    am_hal_sysctrl_fpu_enable();
    am_hal_sysctrl_fpu_stacking_enable(true);
#else
    am_hal_sysctrl_fpu_disable();
#endif

    //
    // Configure the board for low power.
    //
    am_bsp_low_power_init();

    // Turn off unused Flash & SRAM

#ifdef AM_PART_APOLLO
    //
    // SRAM bank power setting.
    // Need to match up with actual SRAM usage for the program
    // Current usage is between 32K and 40K - so disabling upper 3 banks
    //
    am_hal_mcuctrl_sram_power_set(AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7,
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_5 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_6 |
                                  AM_HAL_MCUCTRL_SRAM_POWER_DOWN_7);

#if 0 // Not turning off the Flash as it may be needed to download the image
    //
    // Flash bank power set.
    //
    am_hal_mcuctrl_flash_power_set(AM_HAL_MCUCTRL_FLASH_POWER_DOWN_1);
#endif
#endif // AM_PART_APOLLO

#ifdef AM_PART_APOLLO2
#if 0 // Not turning off the Flash as it may be needed to download the image
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_FLASH512K);
#endif
    am_hal_pwrctrl_memory_enable(AM_HAL_PWRCTRL_MEMEN_SRAM64K);
#endif // AM_PART_APOLLO2

    //
    // Enable printing to the console.
    //
#ifdef AM_DEBUG_PRINTF
    am_bsp_itm_printf_enable();
#endif

    //
    // Initialize plotting interface.
    //
    am_util_debug_printf("Mindtree PXM Example\n");
    setup_serial(0);

    vCreatePXMTask();

    vTaskStartScheduler();

    //
    // We shouldn't ever get here.
    //
    while (1)
    {
    }

}

