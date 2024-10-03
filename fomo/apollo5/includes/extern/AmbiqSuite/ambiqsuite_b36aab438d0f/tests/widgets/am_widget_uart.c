//*****************************************************************************
//
//! @file am_widget_uart.c
//!
//! @brief This widget allows test cases to exercise the Apollo2 UART HAL
//! using loopback between UART #0 and #1.
//!
//!
//! @verbatim
//! The Pin configuration is as follows:
//!     UART0           UART1
//!     -----           -----
//!     GPIO[1]-TX      GPIO[9]-RX
//!     GPIO[2]-RX      GPIO[8]-TX
//!     GPIO[3]-RTS     GPIO[11]-CTS
//!     GPIO[4]-CTS     GPIO[10]-RTS
//!
//! @endverbatim
//
//*****************************************************************************

//*****************************************************************************
//
// ${copyright}
//
// This is part of revision ${version} of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include <stdlib.h>
#include "am_mcu_apollo.h"
#include "am_widget_uart.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Macro definitions
//
//*****************************************************************************
#define FIFO_SIZE               (32)
#define TEST_BUFFER_SIZE        (1024)
#define ARRAY_BUFFER_SIZE       (1024)

//*****************************************************************************
//
// Memory allocation.
//
//*****************************************************************************
//
// UART test configuration (defines the widget state).
//
am_widget_uart_config_t          g_UARTTestConfig;

uint8_t g_ui8UART0TxArray[ARRAY_BUFFER_SIZE];
uint8_t g_ui8UART0RxArray[ARRAY_BUFFER_SIZE];
uint8_t g_ui8UART1TxArray[ARRAY_BUFFER_SIZE];
uint8_t g_ui8UART1RxArray[ARRAY_BUFFER_SIZE];

uint8_t g_UART0TXBuffer[TEST_BUFFER_SIZE];
uint8_t g_UART0RXBuffer[TEST_BUFFER_SIZE];
uint8_t g_UART1TXBuffer[TEST_BUFFER_SIZE];
uint8_t g_UART1RXBuffer[TEST_BUFFER_SIZE];

uint8_t g_UART0TXFifoBuf[FIFO_SIZE+1];
uint8_t g_UART0RXFifoBuf[FIFO_SIZE+1];
uint8_t g_UART1TXFifoBuf[FIFO_SIZE+1];
uint8_t g_UART1RXFifoBuf[FIFO_SIZE+1];

volatile bool    g_UART0TXCMP = false;
volatile bool    g_UART1TXCMP = false;
volatile bool    g_UART0RXTMOUT = false;
volatile bool    g_UART1RXTMOUT = false;
volatile bool    g_UART0RX = false;
volatile bool    g_UART1RX = false;
volatile bool    g_UART0TX = false;
volatile bool    g_UART1TX = false;

am_hal_uart_pwrsave_t g_testSaveRegs;


//*****************************************************************************
//
// Interrupt handlers for the UARTs
//
//*****************************************************************************
void
am_uart_isr(void)
{
    uint32_t ui32Status;

    //
    // Read the masked interrupt status from the UART.
    //
    ui32Status = am_hal_uart_int_status_get(0, true);

    //
    // Clear the UART interrupts.
    //
    am_hal_uart_int_clear(0, ui32Status);

    if (ui32Status & AM_HAL_UART_INT_TXCMP)
    {
      am_hal_gpio_out_bit_set(28);
      g_UART0TXCMP = true;
      am_hal_gpio_out_bit_clear(28);
    }

    if (ui32Status & AM_HAL_UART_INT_RX_TMOUT)
    {
      am_hal_gpio_out_bit_set(29);
      g_UART0RXTMOUT = true;
      am_hal_gpio_out_bit_clear(29);
    }

    if (ui32Status & AM_HAL_UART_INT_RX)
    {
      am_hal_gpio_out_bit_set(30);
      g_UART0RX = true;
      am_hal_gpio_out_bit_clear(30);
    }

    if (ui32Status & AM_HAL_UART_INT_TX)
    {
      am_hal_gpio_out_bit_set(31);
      g_UART0TX = true;
      am_hal_gpio_out_bit_clear(31);
    }


    //
    // Service the buffered UART.
    //
    am_hal_uart_service_buffered(0, ui32Status);

}

void
am_uart1_isr(void)
{
    uint32_t ui32Status;

    //
    // Read the masked interrupt status from the UART.
    //
    ui32Status = am_hal_uart_int_status_get(1, true);

    //
    // Clear the UART interrupts.
    //
    am_hal_uart_int_clear(1, ui32Status);

    if (ui32Status & AM_HAL_UART_INT_TXCMP)
    {
      am_hal_gpio_out_bit_set(28);
      g_UART1TXCMP = true;
      am_hal_gpio_out_bit_clear(28);
    }

    if (ui32Status & AM_HAL_UART_INT_RX_TMOUT)
    {
      am_hal_gpio_out_bit_set(29);
      g_UART1RXTMOUT = true;
      am_hal_gpio_out_bit_clear(29);
    }

    if (ui32Status & AM_HAL_UART_INT_RX)
    {
      am_hal_gpio_out_bit_set(30);
      g_UART1RX = true;
      am_hal_gpio_out_bit_clear(30);
    }

    if (ui32Status & AM_HAL_UART_INT_TX)
    {
      am_hal_gpio_out_bit_set(31);
      g_UART1TX = true;
      am_hal_gpio_out_bit_clear(31);
    }

    //
    // Service the buffered UART.
    //
    am_hal_uart_service_buffered_timeout_save(1, ui32Status);

}

//*****************************************************************************
//
// Initialize the UART buffering.
//
//*****************************************************************************
void uart_enable_buffering(uint32_t ui32Module)
{
  //
  // Initialize the buffered UART.
  //
  switch(ui32Module)
  {
  case 0:
    am_hal_uart_init_buffered(0, g_ui8UART0RxArray, ARRAY_BUFFER_SIZE, g_ui8UART0TxArray, ARRAY_BUFFER_SIZE);
    break;
  case 1:
    am_hal_uart_init_buffered(1, g_ui8UART1RxArray, ARRAY_BUFFER_SIZE, g_ui8UART1TxArray, ARRAY_BUFFER_SIZE);
    break;
  }

  //
  // Enable the UART interrupts.
  //
  am_hal_uart_int_enable(ui32Module, AM_HAL_UART_INT_RX | AM_HAL_UART_INT_TXCMP | AM_HAL_UART_INT_RX_TMOUT );

  //
  // Enable the UART interrupt handler.
  //
#if AM_CMSIS_REGS
  NVIC_EnableIRQ(UART0_IRQn + ui32Module);
#else // AM_CMSIS_REGS
  am_hal_interrupt_enable(AM_HAL_INTERRUPT_UART + ui32Module);
#endif // AM_CMSIS_REGS

}

//*****************************************************************************
//
// Initialize the UART.
//
//*****************************************************************************
void init_uart(uint32_t ui32Module, am_hal_uart_config_t *pUARTConfig, bool useFifo, bool useBuffers)
{
  switch(ui32Module)
  {
  case 0:
    //
    // Set up the UART #0 pins.
    //
    am_hal_gpio_pin_config(1, AM_HAL_PIN_1_UART0TX);
    am_hal_gpio_pin_config(2, AM_HAL_PIN_2_UART0RX);
    am_hal_gpio_pin_config(3, AM_HAL_PIN_3_UART0RTS);
    am_hal_gpio_pin_config(4, AM_HAL_PIN_4_UART0CTS);
    break;
  case 1:
    //
    // Set up the UART #1 pins.
    //
    am_hal_gpio_pin_config(8, AM_HAL_PIN_8_UART1TX);
    am_hal_gpio_pin_config(9, AM_HAL_PIN_9_UART1RX);
    am_hal_gpio_pin_config(10, AM_HAL_PIN_10_UART1RTS);
    am_hal_gpio_pin_config(11, AM_HAL_PIN_11_UART1CTS);
    break;
  default:
    break;
  }

  //
  // Power on the selected UART
  //
  am_hal_uart_pwrctrl_enable(ui32Module);

  //
  // Enable the clock generation to the UART.
  //
  am_hal_uart_clock_enable(ui32Module);

  //
  // Disable the UART before configuring it.
  //
  am_hal_uart_disable(ui32Module);

  //
  // Configure the UART.
  //
  am_hal_uart_config(ui32Module, pUARTConfig);

  //
  // Enable the UART FIFO if requested.
  //
  if (useFifo)
  {
    am_hal_uart_fifo_config(ui32Module, AM_HAL_UART_TX_FIFO_1_2 | AM_HAL_UART_RX_FIFO_1_2);
  }

  //
  // Enable the UART Buffered service if requested.
  //
  if (useBuffers)
  {
    uart_enable_buffering(ui32Module);
  }

  //
  // Enable the UART.
  //
  am_hal_uart_enable(ui32Module);


}

//*****************************************************************************
//
// Disable the UART.
//
//*****************************************************************************
void
disable_uart(uint32_t ui32Module)
{
  //
  // Wait for the uart to stop transmitting.
  //
  while (AM_BFRn(UART, ui32Module, FR, BUSY));

  //
  // Clear all interrupts before sleeping as having a pending UART interrupt
  // burns power.
  //
  am_hal_uart_int_clear(ui32Module, 0xFFFFFFFF);

  //
  // Disable the UART interrupt handler.
  //
#if AM_CMSIS_REGS
  NVIC_DisableIRQ(UART0_IRQn + ui32Module);
#else // AM_CMSIS_REGS
  am_hal_interrupt_disable(AM_HAL_INTERRUPT_UART + ui32Module);
#endif // AM_CMSIS_REGS

  //
  // Disable the UART.
  //
  am_hal_uart_disable(ui32Module);

  //
  // Disable the clock generation to the UART.
  //
  am_hal_uart_clock_disable(ui32Module);

  //
  // Power off the UART.
  //
  am_hal_uart_pwrctrl_disable(ui32Module);

}

//*****************************************************************************
//
// Generate the Test Data
//
//*****************************************************************************
void
generate_test_data(uint8_t *pBuffer, uint32_t size, am_widget_uart_data_t type)
{
  for (uint32_t i = 0; i<size; i++)
  {
    switch(type)
    {
    case AM_WIDGET_UART_DATA_ALL_ZEROS:
      pBuffer[i] = 0x00;
      break;
    case AM_WIDGET_UART_DATA_ALL_ONES:
      pBuffer[i] = 0xFF;
      break;
    case AM_WIDGET_UART_DATA_COUNTING:
      if (('\0' == (i & 0xFF)) ||
          ('\r' == (i & 0xFF)) ||
          ('\n' == (i & 0xFF)))
      {
        pBuffer[i] = 0xFF;      // Need to avoid /0 as this is NULL termination for string tests.
      }
      else
      {
        pBuffer[i] = i & 0xFF;
      }
      break;
    case AM_WIDGET_UART_DATA_RANDOM:
      pBuffer[i] = rand() & 0xFF;
      break;
    }
  }
}

//*****************************************************************************
//
// Flush the FIFOs.
//
//*****************************************************************************
void flush_uart_fifos(void)
{
  // Flush the receive FIFOs.
  while (!AM_BFRn(UART, 0, FR, RXFE))
  {
    AM_REGn(UART, 0, DR);
  }
  while (!AM_BFRn(UART, 1, FR, RXFE))
  {
    AM_REGn(UART, 1, DR);
  }
}

//*****************************************************************************
//
// Reset the Test Buffers
//
//*****************************************************************************
void reset_buffers(void)
{
  for (uint32_t i = 0; i < TEST_BUFFER_SIZE; i++)
  {
    g_UART0TXBuffer[i] = 0;
    g_UART0RXBuffer[i] = 0;
    g_UART1TXBuffer[i] = 0;
    g_UART1RXBuffer[i] = 0;
  }

  for (uint32_t i = 0; i < (FIFO_SIZE+1); i++)
  {
    g_UART0TXFifoBuf[i] = 0;
    g_UART0RXFifoBuf[i] = 0;
    g_UART1TXFifoBuf[i] = 0;
    g_UART1RXFifoBuf[i] = 0;
  }
}

//*****************************************************************************
//
// Widget Setup Function.
//
//*****************************************************************************
uint32_t am_widget_uart_setup(am_widget_uart_config_t *pUARTTestConfig, void **ppWidget, char *pErrStr)
{
  uint32_t      ui32Status = 0;
  am_hal_uart_config_t  UARTConfig;

  UARTConfig.ui32BaudRate = pUARTTestConfig->ui32BaudRate;
  UARTConfig.ui32DataBits = AM_HAL_UART_DATA_BITS_8;
  UARTConfig.bTwoStopBits = false;
  UARTConfig.ui32Parity = AM_HAL_UART_PARITY_NONE;
  UARTConfig.ui32FlowCtrl = pUARTTestConfig->ui32FlowCtrl;

  g_UARTTestConfig = *pUARTTestConfig;

  //
  // Initialize the widget state configuration.
  //
  *ppWidget = &g_UARTTestConfig;

  //
  // Initialize the UARTs with the test configuration.
  //
  init_uart(0,&UARTConfig,pUARTTestConfig->useFifo,pUARTTestConfig->useBuffers);
  init_uart(1,&UARTConfig,pUARTTestConfig->useFifo,pUARTTestConfig->useBuffers);

  if (pUARTTestConfig->useBuffers)
  {
    am_hal_interrupt_master_enable();
  }
  //
  // Flush the FIFOs.
  //
  flush_uart_fifos();

  //
  // Set up GPIOs for timing debug.
  //
  am_hal_gpio_pin_config(28, AM_HAL_GPIO_OUTPUT);
  am_hal_gpio_pin_config(29, AM_HAL_GPIO_OUTPUT);
  am_hal_gpio_pin_config(30, AM_HAL_GPIO_OUTPUT);
  am_hal_gpio_pin_config(31, AM_HAL_GPIO_OUTPUT);


  return ui32Status;
}

//*****************************************************************************
//
// Widget Cleanup Function.
//
//*****************************************************************************
uint32_t am_widget_uart_cleanup(void *pWidget, char *pErrStr)
{
  uint32_t      ui32Status = 0;

  //
  // Disable the UARTs with the test configuration.
  //
  disable_uart(0);
  disable_uart(1);

  return ui32Status;
}

//*****************************************************************************
//
// Widget Test Execution Function.
//
//*****************************************************************************
bool am_widget_uart_test_polling_char(void *pWidget, char *pErrStr)
{
  //
  // Create a pointer to the UART widget state configuration.
  //
  am_widget_uart_config_t        *pUARTConfig = (am_widget_uart_config_t *)pWidget;

  //
  // Reset the test buffers.
  //
  reset_buffers();

  //
  // Generate the TX Buffer data.
  //
  generate_test_data(g_UART0TXBuffer, pUARTConfig->Size, pUARTConfig->Data);

  //
  // Transmit the data via UART0 to UART1 and loopback to UART0.
  //
  for (uint32_t i = 0; i < pUARTConfig->Size; i++)
  {
    am_hal_uart_char_transmit_polled(0,g_UART0TXBuffer[i]);
    am_hal_uart_char_receive_polled(1,(char *)&g_UART1RXBuffer[i]);
    am_hal_uart_char_transmit_polled(1,g_UART1RXBuffer[i]);             // Loopback
    am_hal_uart_char_receive_polled(0,(char *)&g_UART0RXBuffer[i]);
  }

  //
  // Compare the data in UART0 TX & RX buffers.
  //
  for (uint32_t i = 0; i < pUARTConfig->Size; i++)
  {
    if (g_UART0TXBuffer[i] != g_UART0RXBuffer[i])
    {
      return false;
    }
  }

  return true;

}

//*****************************************************************************
//
// Widget Test Execution Function.
//
//*****************************************************************************
bool am_widget_uart_test_polling_string(void *pWidget, char *pErrStr)
{

  //
  // Create a pointer to the UART widget state configuration.
  //
  am_widget_uart_config_t        *pUARTTestConfig = (am_widget_uart_config_t *)pWidget;

  //
  // Reset the test buffers.
  //
  reset_buffers();

  //
  // Generate the TX Buffer data.
  //
  generate_test_data(g_UART0TXBuffer, pUARTTestConfig->Size, pUARTTestConfig->Data);

  //
  // Flush the FIFOs.
  //
  flush_uart_fifos();

  //
  // Loop FIFO_SIZE packets from UART0 to UART1 and back.
  //
  for (uint32_t i = 0; i < pUARTTestConfig->Size; i += FIFO_SIZE)
  {

    // Create a NULL terminated string to send.
    for (uint32_t j = 0; j < FIFO_SIZE; j++)
    {
      g_UART0TXFifoBuf[j] = g_UART0TXBuffer[i+j];
    }
    g_UART0TXFifoBuf[FIFO_SIZE] = 0;
    am_hal_uart_string_transmit_polled(0,(char *)g_UART0TXFifoBuf);
    for (uint32_t i = 0; i < FIFO_SIZE; i++)
    {
      am_hal_uart_char_receive_polled(1,(char *)&g_UART1RXFifoBuf[i]);
      am_hal_uart_char_transmit_polled(1,g_UART1RXFifoBuf[i]);             // Loopback
    }
    am_hal_uart_line_receive_polled(0,FIFO_SIZE+1,(char *)&g_UART0RXBuffer[i]);
  }

  //
  // Compare the data in UART0 TX & RX buffers.
  //
  for (uint32_t i = 0; i < pUARTTestConfig->Size; i++)
  {
    if (g_UART0TXBuffer[i] != g_UART0RXBuffer[i])
    {
      return false;
    }
  }

  return true;
}

//*****************************************************************************
//
// Widget Test Execution Function.
//
//*****************************************************************************
bool am_widget_uart_test_buffered_char(void *pWidget, char *pErrStr)
{
  //
  // Create a pointer to the UART widget state configuration.
  //
  am_widget_uart_config_t        *pUARTConfig = (am_widget_uart_config_t *)pWidget;

  //
  // Reset the test buffers.
  //
  reset_buffers();

  //
  // Generate the TX Buffer data.
  //
  generate_test_data(g_UART0TXBuffer, pUARTConfig->Size, pUARTConfig->Data);

  //
  // Transmit the data via UART0 to UART1 and loopback to UART0.
  //
  for (uint32_t i = 0; i < pUARTConfig->Size; i++)
  {
    am_hal_uart_char_transmit_buffered(0,g_UART0TXBuffer[i]);
    while (0 == am_hal_uart_char_receive_buffered(1,(char *)&g_UART1RXBuffer[i],1));
    am_hal_uart_char_transmit_buffered(1,g_UART1RXBuffer[i]);             // Loopback
    while (0 == am_hal_uart_char_receive_buffered(0,(char *)&g_UART0RXBuffer[i],1));
  }

  //
  // Compare the data in UART0 TX & RX buffers.
  //
  for (uint32_t i = 0; i < pUARTConfig->Size; i++)
  {
    if (g_UART0TXBuffer[i] != g_UART0RXBuffer[i])
    {
      return false;
    }
  }

  return true;

}

//*****************************************************************************
//
// Widget Test Execution Function.
//
//*****************************************************************************
bool am_widget_uart_test_buffered_string(void *pWidget, char *pErrStr)
{
  uint32_t      rxSize, txSize;

  //
  // Create a pointer to the UART widget state configuration.
  //
  am_widget_uart_config_t        *pUARTTestConfig = (am_widget_uart_config_t *)pWidget;

  //
  // Reset the test buffers.
  //
  reset_buffers();

  g_UART0TXCMP = false;
  g_UART1TXCMP = false;
  g_UART0RXTMOUT = false;
  g_UART1RXTMOUT = false;
  g_UART0RX = false;
  g_UART1RX = false;
  g_UART0TX = false;
  g_UART1TX = false;

  //
  // Generate the TX Buffer data.
  //
  generate_test_data(g_UART0TXBuffer, pUARTTestConfig->Size, pUARTTestConfig->Data);

  //
  // Flush the FIFOs.
  //
  flush_uart_fifos();

  //
  // Generate the TX Buffer data.
  //
  generate_test_data(g_UART0TXBuffer, pUARTTestConfig->Size, pUARTTestConfig->Data);
  g_UART0TXBuffer[pUARTTestConfig->Size] = 0;                                    // Null terminate the string.

  am_hal_uart_string_transmit_buffered(0,(char *)g_UART0TXBuffer);
  while(!g_UART0TXCMP);

  am_hal_gpio_out_bit_set(31);
  do
  {
    am_hal_uart_get_status_buffered(1,&rxSize,&txSize);
  }
  while (rxSize < pUARTTestConfig->Size);
  am_hal_gpio_out_bit_clear(31);

  am_hal_uart_char_receive_buffered(1,(char *)g_UART1RXBuffer,pUARTTestConfig->Size);

  am_hal_uart_string_transmit_buffered(1,(char *)g_UART1RXBuffer);
  while(!g_UART1TXCMP);

  do
  {
    am_hal_uart_get_status_buffered(0,&rxSize,&txSize);
  }
  while (rxSize < pUARTTestConfig->Size);


  am_hal_uart_char_receive_buffered(0,(char *)g_UART0RXBuffer,pUARTTestConfig->Size);

  //
  // Compare the data in UART0 TX & RX buffers.
  //
  for (uint32_t i = 0; i < pUARTTestConfig->Size; i++)
  {
    if (g_UART0TXBuffer[i] != g_UART0RXBuffer[i])
    {
      return false;
    }
  }

  return true;
}

//*****************************************************************************
//
// Widget Test Execution Function.
//
//*****************************************************************************
bool am_widget_uart_test_power(void)
{
  am_hal_uart_config_t  UARTConfig;
  bool                  testStatus = true;

  UARTConfig.ui32BaudRate = 115200;
  UARTConfig.ui32DataBits = AM_HAL_UART_DATA_BITS_8;
  UARTConfig.bTwoStopBits = false;
  UARTConfig.ui32Parity = AM_HAL_UART_PARITY_NONE;
  UARTConfig.ui32FlowCtrl = false;

  //
  // Initialize the UARTs with the test configuration.
  //
  init_uart(0,&UARTConfig,false,false);

  //
  // Save the UART 0 configuration state.
  //
  g_testSaveRegs.ILPR = AM_REGn(UART, 0, ILPR);
  g_testSaveRegs.IBRD = AM_REGn(UART, 0, IBRD);
  g_testSaveRegs.FBRD = AM_REGn(UART, 0, FBRD);
  g_testSaveRegs.LCRH = AM_REGn(UART, 0, LCRH);
  g_testSaveRegs.CR = AM_REGn(UART, 0, CR);
  g_testSaveRegs.IFLS = AM_REGn(UART, 0, IFLS);
  g_testSaveRegs.IER = AM_REGn(UART, 0, IER);

  //
  // Execute a power-off save.
  //
  am_hal_uart_power_off_save(0);

  //
  // Power down the UART.
  //
  am_hal_uart_pwrctrl_disable(0);

  //
  // Wait to make sure we are powered down.
  //
  am_util_delay_ms(100);

  //
  // Power on the UART.
  //
  am_hal_uart_pwrctrl_enable(0);

  //
  // Execute the power-on restore.
  //
  am_hal_uart_power_on_restore(0);

  if ((g_testSaveRegs.ILPR == AM_REGn(UART, 0, ILPR)) &&
      (g_testSaveRegs.IBRD == AM_REGn(UART, 0, IBRD)) &&
      (g_testSaveRegs.FBRD == AM_REGn(UART, 0, FBRD)) &&
      (g_testSaveRegs.LCRH == AM_REGn(UART, 0, LCRH)) &&
      (g_testSaveRegs.CR == AM_REGn(UART, 0, CR)) &&
      (g_testSaveRegs.IFLS == AM_REGn(UART, 0, IFLS)) &&
      (g_testSaveRegs.IER == AM_REGn(UART, 0, IER)))
  {
    testStatus = true;
  }
  else
  {
    testStatus = false;
  }

  //
  // Shutdown the UART.
  //
  disable_uart(1);

  return testStatus;
}

uint32_t ui32UARTInterrupts[] =
{
  AM_HAL_UART_INT_OVER_RUN,
  AM_HAL_UART_INT_BREAK_ERR,
  AM_HAL_UART_INT_PARITY_ERR,
  AM_HAL_UART_INT_FRAME_ERR,
  AM_HAL_UART_INT_RX_TMOUT,
  AM_HAL_UART_INT_TX,
  AM_HAL_UART_INT_RX,
  AM_HAL_UART_INT_DSRM,
  AM_HAL_UART_INT_DCDM,
  AM_HAL_UART_INT_CTSM,
  AM_HAL_UART_INT_TXCMP
};

//*****************************************************************************
//
// Widget Test Execution Function.
//
//*****************************************************************************
bool am_widget_uart_test_status(void)
{
  am_hal_uart_config_t  UARTConfig;
  bool                  testStatus = true;

  UARTConfig.ui32BaudRate = 115200;
  UARTConfig.ui32DataBits = AM_HAL_UART_DATA_BITS_8;
  UARTConfig.bTwoStopBits = false;
  UARTConfig.ui32Parity = AM_HAL_UART_PARITY_NONE;
  UARTConfig.ui32FlowCtrl = false;

  //
  // Initialize the UARTs with the test configuration.
  //
  init_uart(1,&UARTConfig,false,false);

  //
  // Disable all the interrupts
  //
  am_hal_uart_int_disable(1,0xFFFFFFFF);

  //
  // Cycle through each interrupt value.
  //
  for (uint32_t i = 0; i < (sizeof(ui32UARTInterrupts)/sizeof(uint32_t)); i++)
  {
    //
    // Set the interrupt.
    //
    am_hal_uart_int_enable(1, ui32UARTInterrupts[i]);

    //
    // Check that it was set correctly.
    //
    if (ui32UARTInterrupts[i] != am_hal_uart_int_enable_get(1))
    {
      testStatus = false;
      break;
    }

    //
    // Clear the interrupt.
    //
    am_hal_uart_int_disable(1, ui32UARTInterrupts[i]);

    //
    // Check that it was cleared correctly.
    //
    if (0 != am_hal_uart_int_enable_get(1))
    {
      testStatus = false;
      break;
    }
  }

  //
  // Check that there are no errors on the RSR register status.
  //
  if (0 != am_hal_uart_status_get(1))
  {
    testStatus = false;
  }

  //
  // Check for the correct status on the FR register.
  //
  if ((am_hal_uart_flags_get(1) & 0xFF) != (       // Filed as SHELBY-1633 defect in JIRA.  TXBUSY is always set.  Should be Ring Indicator.
                                   AM_HAL_UART_FR_TX_EMPTY |
                                    AM_HAL_UART_FR_RX_EMPTY |
                                      AM_HAL_UART_FR_DCD_DETECTED |
                                        AM_HAL_UART_FR_DSR_READY ))
  {
    testStatus = false;
  }

  //
  // Shutdown the UART.
  //
  disable_uart(1);

  return testStatus;
}


